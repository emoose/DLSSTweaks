#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>
#include <tchar.h>

#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <ini.h>

#include "DLSSTweaks.hpp"
#include "Proxy.hpp"

const wchar_t* NgxFileName = L"_nvngx.dll";
const wchar_t* DlssFileName = L"nvngx_dlss.dll";
const char* DlssFileNameA = "nvngx_dlss.dll";

const wchar_t* LogFileName = L"dlsstweaks.log";
const wchar_t* IniFileName = L"dlsstweaks.ini";
const wchar_t* ErrorFileName = L"dlsstweaks_error.log";

std::filesystem::path ExePath;
std::filesystem::path DllPath;
std::filesystem::path LogPath;
std::filesystem::path IniPath;

UserSettings settings;

std::unordered_map<int, float> qualityLevelRatios =
{
	{NVSDK_NGX_PerfQuality_Value_UltraPerformance, 0.33333334f},
	{NVSDK_NGX_PerfQuality_Value_MaxPerf, 0.5f},
	{NVSDK_NGX_PerfQuality_Value_Balanced, 0.58f},
	{NVSDK_NGX_PerfQuality_Value_MaxQuality, 0.66666667f},

	// note: if NVSDK_NGX_PerfQuality_Value_UltraQuality is non-zero, some games may detect that we're passing a valid resolution and show an Ultra Quality option as a result
	// very few games support this though, and right now DLSS seems to refuse to render if UltraQuality gets passed to it
	// our SetI hook in HooksNvngx can override the quality passed to DLSS if this gets used by the game, letting it think this is MaxQuality instead
	// but we'll only do that if user overrides this in the INI to a non-zero value
	{NVSDK_NGX_PerfQuality_Value_UltraQuality, 0.f},
};

std::mutex initThreadFinishedMutex;
std::condition_variable initThreadFinishedVar;
bool initThreadFinished = false;

// In nvngx.dll wrapper mode the game might call DLSS functions immediately after loading DLL (ie. right after DllMain)
// before our InitThread has actually finished reading settings etc
// This func will just check if finished & wait for it if not, seems to be safe to let the NVNGX calls wait for this
void WaitForInitThread()
{
	if (initThreadFinished)
		return;

	std::unique_lock lock(initThreadFinishedMutex);
	initThreadFinishedVar.wait(lock, [] { return initThreadFinished; });
}

// LoadLibraryXXX hook so we can redirect DLSS library load to users choice
SafetyHookInline LoadLibraryExW_Orig;
SafetyHookInline LoadLibraryExA_Orig;
SafetyHookInline LoadLibraryA_Orig;
SafetyHookInline LoadLibraryW_Orig;

std::once_flag dlssOverrideMessage;
HMODULE __stdcall LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	std::filesystem::path libPath = lpLibFileName;
	auto filenameStr = libPath.filename().string();

	// Check if filename matches any DLL user has requested to override, change to the user-specified path if so
	for (auto& pair : settings.dllPathOverrides)
	{
		if (_stricmp(filenameStr.c_str(), pair.first.c_str()) != 0)
			continue;

		if (!pair.second.empty())
		{
			spdlog::info("DLLPathOverrides: game requested to load {} (from {}), overriding with DLL path: {}", filenameStr, libPath.string(), pair.second.string());
			if (std::filesystem::exists(pair.second))
				libPath = pair.second;
			else
				spdlog::error("DLLPathOverrides: override DLL no longer exists, skipping... (path: {})", pair.second.string());
		}
		break;
	}

	std::wstring libPathStr = libPath.wstring();
	return LoadLibraryExW_Orig.stdcall<HMODULE>(libPathStr.c_str(), hFile, dwFlags);
}

HMODULE __stdcall LoadLibraryExA_Hook(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	std::filesystem::path libPath = lpLibFileName;
	auto libPathStr = libPath.wstring();

	return LoadLibraryExW_Hook(libPathStr.c_str(), hFile, dwFlags);
}

HMODULE __stdcall LoadLibraryW_Hook(LPCWSTR lpLibFileName)
{
	return LoadLibraryExW_Hook(lpLibFileName, 0, 0);
}

HMODULE __stdcall LoadLibraryA_Hook(LPCSTR lpLibFileName)
{
	return LoadLibraryExA_Hook(lpLibFileName, 0, 0);
}

// LoaderNotificationCallback is called whenever a DLL is loaded into the process
// This is used to watch for _nvngx / nvngx_dlss loads, and if found then our XXX::init function will hook the DllMain entrypoint of them
// In that entrypoint hook we can handle applying/removing our hooks from the DLL via ATTACH/DETACH events
// Seems to be the easiest method for us to detect module unload, otherwise we'd need to hook a bunch of FreeLibrary funcs etc
// (since LoaderNotificationCallback for unloads is only called after the DLL has unloaded, making resetting our InlineHooks more difficult...)
void* dll_notification_cookie = nullptr;
void __stdcall LoaderNotificationCallback(unsigned long notification_reason, const LDR_DLL_NOTIFICATION_DATA* notification_data, void* context)
{
	if (notification_reason == LDR_DLL_NOTIFICATION_REASON_LOADED)
	{
		std::wstring dlssName = DlssFileName;

		// If user has overridden the nvngx_dlss path, use the filename they specified in our comparisons below
		if (settings.dllPathOverrides.count(DlssFileNameA))
			dlssName = settings.dllPathOverrides[DlssFileNameA].filename().wstring();

		// A module was loaded in, check if NGX/DLSS and apply hooks if so
		std::wstring dllName(notification_data->Loaded.BaseDllName->Buffer, notification_data->Loaded.BaseDllName->Length / sizeof(WCHAR));
		if (!_wcsicmp(dllName.c_str(), NgxFileName))
		{
			nvngx::init((HMODULE)notification_data->Loaded.DllBase);
		}
		else if (!_wcsicmp(dllName.c_str(), dlssName.c_str()))
		{
			nvngx_dlss::init((HMODULE)notification_data->Loaded.DllBase);
		}
	}
}

void UserSettings::print_to_log()
{
	using namespace utility;

	spdlog::info("Settings:");
	spdlog::info(" - ForceDLAA: {}{}", forceDLAA ? "true" : "false", overrideQualityLevels ? " (overridden by DLSSQualityLevels section)" : "");
	spdlog::info(" - OverrideAutoExposure: {}", overrideAutoExposure == 0 ? "default" : (overrideAutoExposure > 0 ? "enable" : "disable"));
	spdlog::info(" - OverrideAppId: {}", overrideAppId ? "true" : "false");
	spdlog::info(" - OverrideDlssHud: {}", overrideDlssHud == 0 ? "default" : (overrideDlssHud > 0 ? "enable" : "disable"));
	spdlog::info(" - DisableDevWatermark: {}", disableDevWatermark ? "true" : "false");
	spdlog::info(" - WatchIniUpdates: {}", watchIniUpdates ? "true" : "false");
	spdlog::info(" - ResolutionOffset: {}", resolutionOffset);
	spdlog::info(" - DLSSQualityLevels enabled: {}", overrideQualityLevels ? "true" : "false");

	if (overrideQualityLevels)
	{
		spdlog::info("  - UltraPerformance ratio: {}", qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraPerformance]);
		spdlog::info("  - Performance ratio: {}", qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxPerf]);
		spdlog::info("  - Balanced ratio: {}", qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_Balanced]);
		spdlog::info("  - Quality ratio: {}", qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxQuality]);
		spdlog::info("  - UltraQuality ratio: {}", qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraQuality]);
	}

	// only print presets if any of them have been changed
	if (presetDLAA || presetQuality || presetBalanced || presetPerformance || presetUltraPerformance)
	{
		spdlog::info(" - DLSSPresets:");
		if (presetDLAA)
			spdlog::info("  - DLAA: {}", DLSS_PresetEnumToName(presetDLAA));
		if (presetQuality)
			spdlog::info("  - Quality: {}", DLSS_PresetEnumToName(presetQuality));
		if (presetBalanced)
			spdlog::info("  - Balanced: {}", DLSS_PresetEnumToName(presetBalanced));
		if (presetPerformance)
			spdlog::info("  - Performance: {}", DLSS_PresetEnumToName(presetPerformance));
		if (presetUltraPerformance)
			spdlog::info("  - UltraPerformance: {}", DLSS_PresetEnumToName(presetUltraPerformance));
	}
	else
	{
		spdlog::info(" - DLSSPresets: default");
	}

	if (settings.dllPathOverrides.size())
	{
		spdlog::info(" - DLLPathOverrides:");
		for (auto& pair : settings.dllPathOverrides)
			spdlog::info("  - {} -> {}", pair.first, pair.second.string());
	}
	else
	{
		spdlog::info(" - DLLPathOverrides: N/A");
	}
}

bool UserSettings::read(const std::filesystem::path& iniPath)
{
	using namespace utility;

	std::wstring iniPathStr = iniPath.wstring();

	// Read INI via FILE* since INIReader doesn't support wstring
	FILE* iniFile;
	if (_wfopen_s(&iniFile, iniPathStr.c_str(), L"r") != 0 || !iniFile)
		return false;

	inih::INIReader ini(iniFile);
	fclose(iniFile);

	// [DLSS]
	forceDLAA = ini.Get<bool>("DLSS", "ForceDLAA", std::move(forceDLAA));
	overrideAutoExposure = ini.Get<int>("DLSS", "OverrideAutoExposure", std::move(overrideAutoExposure));
	overrideDlssHud = ini.Get<int>("DLSS", "OverrideDlssHud", std::move(overrideDlssHud));
	disableDevWatermark = ini.Get<bool>("DLSS", "DisableDevWatermark", std::move(disableDevWatermark));
	watchIniUpdates = ini.Get<bool>("DLSS", "WatchIniUpdates", std::move(watchIniUpdates));

	// [DLLPathOverrides]
	auto keys = ini.Keys("DLLPathOverrides");
	for (auto& key : keys)
	{
		auto dllFileName = key;
		if (!std::filesystem::path(dllFileName).has_extension())
			dllFileName += ".dll";

		auto path = ini.Get<std::string>("DLLPathOverrides", key, "");
		if (!path.empty() && !std::filesystem::exists(path)) // empty path is allowed so that user can clear the override in local INIs
		{
			spdlog::warn("DLLPathOverrides: override for {} skipped as path {} doesn't exist", key, path);
			continue;
		}

		settings.dllPathOverrides[dllFileName] = path;
	}

	// [DLSSQualityLevels]
	overrideQualityLevels = ini.Get<bool>("DLSSQualityLevels", "Enable", std::move(overrideQualityLevels));
	if (overrideQualityLevels)
	{
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxPerf] = ini.Get<float>("DLSSQualityLevels", "Performance", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxPerf]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_Balanced] = ini.Get<float>("DLSSQualityLevels", "Balanced", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_Balanced]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxQuality] = ini.Get<float>("DLSSQualityLevels", "Quality", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxQuality]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraPerformance] = ini.Get<float>("DLSSQualityLevels", "UltraPerformance", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraPerformance]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraQuality] = ini.Get<float>("DLSSQualityLevels", "UltraQuality", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraQuality]));

		// Clamp values between 0.0 to 1.0
		for (int i = NVSDK_NGX_PerfQuality_Value_MaxPerf; i <= NVSDK_NGX_PerfQuality_Value_UltraQuality; i++)
			qualityLevelRatios[i] = std::clamp(qualityLevelRatios[i], 0.0f, 1.0f);
	}

	// [DLSSPresets]
	overrideAppId = ini.Get<bool>("DLSSPresets", "OverrideAppId", std::move(overrideAppId));
	presetDLAA = DLSS_PresetNameToEnum(ini.Get<std::string>("DLSSPresets", "DLAA", DLSS_PresetEnumToName(presetDLAA)));
	presetQuality = DLSS_PresetNameToEnum(ini.Get<std::string>("DLSSPresets", "Quality", DLSS_PresetEnumToName(presetQuality)));
	presetBalanced = DLSS_PresetNameToEnum(ini.Get<std::string>("DLSSPresets", "Balanced", DLSS_PresetEnumToName(presetBalanced)));
	presetPerformance = DLSS_PresetNameToEnum(ini.Get<std::string>("DLSSPresets", "Performance", DLSS_PresetEnumToName(presetPerformance)));
	presetUltraPerformance = DLSS_PresetNameToEnum(ini.Get<std::string>("DLSSPresets", "UltraPerformance", DLSS_PresetEnumToName(presetUltraPerformance)));

	// [Compatibility]
	resolutionOffset = ini.Get<int>("Compatibility", "ResolutionOffset", std::move(resolutionOffset));

	return true;
}

HMODULE ourModule = 0;
bool attachResult = false;

unsigned int __stdcall InitThread(void* param)
{
	WCHAR modulePath[4096];
	GetModuleFileNameW(GetModuleHandleA(0), modulePath, 4096);
	ExePath = std::filesystem::path(modulePath);
	GetModuleFileNameW(ourModule, modulePath, 4096);
	DllPath = std::filesystem::path(modulePath);

	// spdlog setup
	{
		// Log is always written next to our DLL
		LogPath = DllPath.parent_path() / LogFileName;

		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
		try
		{
			sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(LogPath.string(), true));
		}
		catch (const std::exception&)
		{
			// spdlog failed to open log file for writing (happens in some WinStore apps)
			// let's just try to continue instead of crashing
		}

		auto combined_logger = std::make_shared<spdlog::logger>("", begin(sinks), end(sinks));
		combined_logger->set_level(spdlog::level::trace);
		spdlog::set_default_logger(combined_logger);
		spdlog::flush_on(spdlog::level::info);
	}

	spdlog::info("DLSSTweaks v0.200.5, by emoose: {} wrapper loaded", DllPath.filename().string());
	spdlog::info("Game path: {}", ExePath.string());
	spdlog::info("DLL path: {}", DllPath.string());

	spdlog::info("---");

	// Read config from next to DLL first, and then from next to EXE
	// So with a global injector, you could keep a global config stored next to the DLL, and then per-game overrides kept next to the EXE
	{
		if (DllPath.parent_path() != ExePath.parent_path())
		{
			IniPath = DllPath.parent_path() / IniFileName;
			if (std::filesystem::exists(IniPath))
			{
				if (settings.read(IniPath))
					spdlog::info("Config read from {}", IniPath.string());
				else
					spdlog::error("Failed to read config from {}", IniPath.string());
			}
		}

		IniPath = ExePath.parent_path() / IniFileName;
		if (std::filesystem::exists(IniPath))
		{
			if (settings.read(IniPath))
				spdlog::info("Config read from {}", IniPath.string());
			else
				spdlog::error("Failed to read config from {}", IniPath.string());
		}

		// IniPath will point to the INI next to game EXE after this, for WatchIniUpdates to update from that INI
		// TODO: might be nice to watch both INIs in future, but unsure how much work that'd need
	}

	// print msg about wrapping to log here, as nvngx wrap stuff was setup before spdlog was inited
	if (proxy::is_wrapping_nvngx)
		spdlog::info("Wrapped nvngx.dll, using funcptrs from original dll");
	else
		spdlog::info("Wrapped system DLL, watching for DLSS module load");

	// Register notification so we can learn of DLL loads/unloads
	LdrRegisterDllNotificationFunc LdrRegisterDllNotification =
		(LdrRegisterDllNotificationFunc)GetProcAddress(GetModuleHandle("ntdll.dll"), "LdrRegisterDllNotification");
	if (LdrRegisterDllNotification &&
		LdrRegisterDllNotification(0, &LoaderNotificationCallback, nullptr, &dll_notification_cookie) == STATUS_SUCCESS)
	{
		spdlog::debug("LdrRegisterDllNotification callback set");
	}
	else
	{
		spdlog::error("Failed to locate LdrRegisterDllNotification function address?"); // shouldn't happen
	}

	// Hook LoadLibrary so we can override DLSS path if desired
	if (settings.dllPathOverrides.size())
	{
		spdlog::debug("LoadLibrary hook set");

		auto* kernel32 = GetModuleHandleA("kernel32.dll");
		auto* LoadLibraryExW_addr = kernel32 ? GetProcAddress(kernel32, "LoadLibraryExW") : nullptr;
		auto* LoadLibraryExA_addr = kernel32 ? GetProcAddress(kernel32, "LoadLibraryExA") : nullptr;
		auto* LoadLibraryW_addr = kernel32 ? GetProcAddress(kernel32, "LoadLibraryW") : nullptr;
		auto* LoadLibraryA_addr = kernel32 ? GetProcAddress(kernel32, "LoadLibraryA") : nullptr;
		if (!LoadLibraryExW_addr || !LoadLibraryExA_addr || !LoadLibraryW_addr || !LoadLibraryA_addr)
		{
			spdlog::error("Failed to find LoadLibrary address?");
		}
		else
		{
			LoadLibraryExW_Orig = safetyhook::create_inline(LoadLibraryExW_addr, LoadLibraryExW_Hook);
			LoadLibraryExA_Orig = safetyhook::create_inline(LoadLibraryExA_addr, LoadLibraryExA_Hook);
			LoadLibraryW_Orig = safetyhook::create_inline(LoadLibraryW_addr, LoadLibraryW_Hook);
			LoadLibraryA_Orig = safetyhook::create_inline(LoadLibraryA_addr, LoadLibraryA_Hook);
		}
	}

	// Init finished, allow any pending DLSS calls to continue
	{
		std::lock_guard lock(initThreadFinishedMutex);
		initThreadFinished = true;
		initThreadFinishedVar.notify_all();
	}

	if (!settings.watchIniUpdates)
		return 0;

	std::wstring iniFolder = IniPath.parent_path().wstring();
	HANDLE file = CreateFileW(iniFolder.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		NULL);

	if (!file)
	{
		DWORD err = GetLastError();
		spdlog::error("WatchIniUpdates: CreateFileW \"{}\" failed with error code {}", IniPath.parent_path().string(), err);
		return 0;
	}

	OVERLAPPED overlapped;
	overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
	if (!overlapped.hEvent)
	{
		DWORD err = GetLastError();
		spdlog::error("WatchIniUpdates: CreateEvent failed with error code {}", err);
		CloseHandle(file);
		return 0;
	}

	uint8_t change_buf[1024];
	BOOL success = ReadDirectoryChangesW(
		file, change_buf, 1024, TRUE,
		FILE_NOTIFY_CHANGE_FILE_NAME |
		FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_LAST_WRITE,
		NULL, &overlapped, NULL);

	if (!success)
	{
		DWORD err = GetLastError();
		spdlog::error("WatchIniUpdates: ReadDirectoryChangesW failed with error code {}", err);
		return 0;
	}

	spdlog::info("Watching for INI updates...");

	while (success)
	{
		DWORD result = WaitForSingleObject(overlapped.hEvent, INFINITE);

		if (result == WAIT_OBJECT_0)
		{
			DWORD bytes_transferred;
			GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

			FILE_NOTIFY_INFORMATION* evt = (FILE_NOTIFY_INFORMATION*)change_buf;

			for (;;)
			{
				if (evt->Action == FILE_ACTION_MODIFIED)
				{
					// evt->FileName isn't null-terminated, so construct wstring for it based on FileNameLength
					std::wstring name = std::wstring(evt->FileName, evt->FileNameLength / sizeof(WCHAR));
					if (!_wcsicmp(name.c_str(), IniFileName))
					{
						// INI read might fail if it's still being updated by a text editor etc
						// so try attempting a few times, Sleep(1000) between attempts should hopefully let us read it fine
						int attempts = 3;
						while (attempts--)
						{
							if (settings.read(IniPath))
							{
								spdlog::info("Config updated from {}", IniPath.string());
								break;
							}
							Sleep(1000);
						}
					}
				}

				// Any more events to handle?
				if (evt->NextEntryOffset)
					*((uint8_t**)&evt) += evt->NextEntryOffset;
				else
					break;
			}

			// Queue the next event
			success = ReadDirectoryChangesW(
				file, change_buf, 1024, TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME |
				FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_LAST_WRITE,
				NULL, &overlapped, NULL);
		}
	}

	CloseHandle(file);
	return 0;
}

HANDLE processUniqueMutex;

// Tries to create a process-unique Win32 mutex
// If mutex already exists then another instance of this module must have already been loaded in
BOOL CheckDllAlreadyLoaded()
{
	// Generate unique name for the mutex based on the current process ID
	TCHAR szMutexName[MAX_PATH];
	_sntprintf_s(szMutexName, MAX_PATH, _T("Local\\DLSSTweaksMutex_%lu"), GetCurrentProcessId());

	processUniqueMutex = CreateMutex(NULL, TRUE, szMutexName);
	if (processUniqueMutex == NULL)
		return FALSE;

	DWORD dwError = GetLastError();
	if (dwError == ERROR_ALREADY_EXISTS)
	{
		// Another instance of the DLL must already be loaded in this process
		CloseHandle(processUniqueMutex); // Not using ReleaseMutex since we want to keep the mutex alive, just closing the handle
		processUniqueMutex = NULL;
		return TRUE;
	}

	// Mutex was created successfully, no other instance of the DLL is loaded in this process
	return FALSE;
}

BOOL APIENTRY DllMain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		ourModule = hModule;
		attachResult = proxy::on_attach(ourModule);
		if (proxy::is_wrapping_nvngx)
			nvngx::init_from_proxy();

		// Check if another instance/version of this module has already loaded in
		if (CheckDllAlreadyLoaded())
		{
			// Disable tweaks to hopefully let game continue...
			settings.disableAllTweaks = true;

			// Fire our InitThreadFinished var so that any wrapped funcs can continue
			{
				std::lock_guard lock(initThreadFinishedMutex);
				initThreadFinished = true;
				initThreadFinishedVar.notify_all();
			}

			// Try warning user via error log file & Win32 messagebox
			std::string warningText =
				"Warning: multiple versions of DLSSTweaks have attempted to load into the game process.\n\nIf you recently tried to update the DLL, an older version may still be present & loaded in.\n\nCheck your game folder for files such as dxgi.dll / xinput1_3.dll / nvngx.dll and remove any extra versions.";
			
			try
			{
				std::ofstream warningFile(ErrorFileName);
				warningFile << warningText << "\r\n";
				warningFile.close();
			}
			catch (const std::exception&)
			{
			}

			MessageBox(NULL, warningText.c_str(), "DLSSTweaks", MB_ICONEXCLAMATION);

			// Skip InitThread
			return TRUE;
		}

		_beginthreadex(NULL, 0, InitThread, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		proxy::on_detach();
		if (processUniqueMutex)
			ReleaseMutex(processUniqueMutex);
	}

	return TRUE;
}
