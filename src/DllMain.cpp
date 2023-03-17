#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>

#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <ini.h>

#include "DLSSTweaks.hpp"
#include "Proxy.hpp"

const wchar_t* NgxFileName = L"_nvngx.dll";
const wchar_t* DlssFileName = L"nvngx_dlss.dll";

const wchar_t* LogFileName = L"dlsstweaks.log";
const wchar_t* IniFileName = L"dlsstweaks.ini";

std::filesystem::path ExePath;
std::filesystem::path DllPath;
std::filesystem::path LogPath;
std::filesystem::path IniPath;

UserSettings settings;

std::unordered_map<NVSDK_NGX_PerfQuality_Value, float> qualityLevelRatios =
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
	auto filenameStr = libPath.filename().wstring();

	if (!_wcsicmp(filenameStr.c_str(), DlssFileName) && !settings.overrideDlssDll.empty())
	{
		std::call_once(dlssOverrideMessage, []() { 
			spdlog::info("Game is loading DLSS, overriding with DLL path: {}", settings.overrideDlssDll.string());
		});

		libPath = settings.overrideDlssDll;
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
		if (!settings.overrideDlssDll.empty())
			dlssName = settings.overrideDlssDll.filename().wstring();

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

unsigned int DLSS_ReadPresetFromIni(inih::INIReader& ini, const std::string& section, const std::string& key)
{
	std::string val = ini.Get<std::string>(section, key, "Default");

	if (!stricmp(val.c_str(), "Default"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	if (!stricmp(val.c_str(), "A"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_A;
	if (!stricmp(val.c_str(), "B"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_B;
	if (!stricmp(val.c_str(), "C"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_C;
	if (!stricmp(val.c_str(), "D"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_D;
	if (!stricmp(val.c_str(), "E"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_E;
	if (!stricmp(val.c_str(), "F"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_F;

	return NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
}

std::string DLSS_PresetEnumToName(unsigned int val)
{
	if (val <= NVSDK_NGX_DLSS_Hint_Render_Preset_Default || val > NVSDK_NGX_DLSS_Hint_Render_Preset_F)
		return "Default";
	int charVal = int(val) - 1 + 'A';
	std::string ret(1, charVal);
	return ret;
}

void UserSettings::print_to_log()
{
	spdlog::info("Settings:");
	spdlog::info(" - ForceDLAA: {}{}", forceDLAA ? "true" : "false", overrideQualityLevels ? " (overridden by DLSSQualityLevels section)" : "");
	spdlog::info(" - OverrideAutoExposure: {}", overrideAutoExposure == 0 ? "default" : (overrideAutoExposure > 0 ? "enable" : "disable"));
	spdlog::info(" - OverrideAppId: {}", overrideAppId ? "true" : "false");
	spdlog::info(" - OverrideDlssHud: {}", overrideDlssHud == 0 ? "default" : (overrideDlssHud > 0 ? "enable" : "disable"));
	spdlog::info(" - DisableDevWatermark: {}", disableDevWatermark ? "true" : "false");
	spdlog::info(" - OverrideDlssDll: {}", overrideDlssDll.empty() ? "N/A" : overrideDlssDll.string());
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
	overrideDlssDll = ini.Get<std::filesystem::path>("DLSS", "OverrideDlssDll", "");
	watchIniUpdates = ini.Get<bool>("DLSS", "WatchIniUpdates", std::move(watchIniUpdates));

	// [DLSSQualityLevels]
	overrideQualityLevels = ini.Get<bool>("DLSSQualityLevels", "Enable", std::move(overrideQualityLevels));
	if (overrideQualityLevels)
	{
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxPerf] = ini.Get<float>("DLSSQualityLevels", "Performance", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxPerf]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_Balanced] = ini.Get<float>("DLSSQualityLevels", "Balanced", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_Balanced]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxQuality] = ini.Get<float>("DLSSQualityLevels", "Quality", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxQuality]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraPerformance] = ini.Get<float>("DLSSQualityLevels", "UltraPerformance", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraPerformance]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraQuality] = ini.Get<float>("DLSSQualityLevels", "UltraQuality", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraQuality]));
	}

	// [DLSSPresets]
	overrideAppId = ini.Get<bool>("DLSSPresets", "OverrideAppId", std::move(overrideAppId));
	presetDLAA = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "DLAA");
	presetQuality = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "Quality");
	presetBalanced = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "Balanced");
	presetPerformance = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "Performance");
	presetUltraPerformance = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "UltraPerformance");

	// [Compatibility]
	resolutionOffset = ini.Get<int>("Compatibility", "ResolutionOffset", std::move(resolutionOffset));

	if (!overrideDlssDll.empty() && !std::filesystem::exists(overrideDlssDll))
	{
		spdlog::warn("Disabling OverrideDlssDll as override DLL wasn't found (path: {})", overrideDlssDll.string());
		overrideDlssDll.clear();
	}

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

	// If an INI exists next to game EXE, use that
	// else we'll try reading INI from next to this DLL
	IniPath = ExePath.parent_path() / IniFileName;
	if (!std::filesystem::exists(IniPath))
		IniPath = DllPath.parent_path() / IniFileName;

	spdlog::info("DLSSTweaks v0.200.4, by emoose: {} wrapper loaded", DllPath.filename().string());
	spdlog::info("Game path: {}", ExePath.string());
	spdlog::info("DLL path: {}", DllPath.string());
	spdlog::info("Config path: {}", IniPath.string());

	spdlog::info("---");

	settings.read(IniPath);

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
	if (!settings.overrideDlssDll.empty())
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
			auto builder = SafetyHookFactory::acquire();
			LoadLibraryExW_Orig = builder.create_inline(LoadLibraryExW_addr, LoadLibraryExW_Hook);
			LoadLibraryExA_Orig = builder.create_inline(LoadLibraryExA_addr, LoadLibraryExA_Hook);
			LoadLibraryW_Orig = builder.create_inline(LoadLibraryW_addr, LoadLibraryW_Hook);
			LoadLibraryA_Orig = builder.create_inline(LoadLibraryA_addr, LoadLibraryA_Hook);
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

	spdlog::info("Watching for INI updates...");

	std::wstring iniFolder = IniPath.parent_path().wstring();
	HANDLE file = CreateFileW(iniFolder.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		NULL);

	if (!file)
		return 0;

	OVERLAPPED overlapped;
	overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
	if (!overlapped.hEvent)
	{
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
								break;
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

BOOL APIENTRY DllMain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		ourModule = hModule;
		attachResult = proxy::on_attach(ourModule);
		if (proxy::is_wrapping_nvngx)
			nvngx::init_from_proxy();

		_beginthreadex(NULL, 0, InitThread, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		proxy::on_detach();
	}

	return TRUE;
}
