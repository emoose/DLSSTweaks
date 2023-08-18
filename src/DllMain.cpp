#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>
#include <tchar.h>

#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <ini.h>

#include "DLSSTweaks.hpp"
#include "Proxy.hpp"

const wchar_t* NgxFileName = L"_nvngx.dll";
const wchar_t* DlssFileName = L"nvngx_dlss.dll";
const char* DlssFileNameA = "nvngx_dlss.dll";
const wchar_t* DlssgFileName = L"nvngx_dlssg.dll";
const char* DlssgFileNameA = "nvngx_dlssg.dll";

const wchar_t* LogFileName = L"dlsstweaks.log";
const wchar_t* IniFileName = L"dlsstweaks.ini";
const wchar_t* ErrorFileName = L"dlsstweaks_error.log";

std::filesystem::path ExePath;
std::filesystem::path DllPath;
std::filesystem::path LogPath;
std::filesystem::path IniPath;

UserSettings settings;

const int qualityLevelCount = 5;

const char* qualityLevelNames[qualityLevelCount] = {
	"Performance",
	"Balanced",
	"Quality",
	"UltraPerformance",
	"UltraQuality"
};

NVSDK_NGX_PerfQuality_Value qualityLevels[qualityLevelCount] = {
	NVSDK_NGX_PerfQuality_Value_UltraPerformance,
	NVSDK_NGX_PerfQuality_Value_MaxPerf,
	NVSDK_NGX_PerfQuality_Value_Balanced,
	NVSDK_NGX_PerfQuality_Value_MaxQuality,
	NVSDK_NGX_PerfQuality_Value_UltraQuality
};

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

std::unordered_map<NVSDK_NGX_PerfQuality_Value, std::pair<int, int>> qualityLevelResolutions =
{
	{NVSDK_NGX_PerfQuality_Value_UltraPerformance, {0,0}},
	{NVSDK_NGX_PerfQuality_Value_MaxPerf, {0,0}},
	{NVSDK_NGX_PerfQuality_Value_Balanced, {0,0}},
	{NVSDK_NGX_PerfQuality_Value_MaxQuality, {0,0}},

	// see note about UltraQuality in qualityLevelRatios section above
	{NVSDK_NGX_PerfQuality_Value_UltraQuality, {0,0}},
};

// The values that we told game about for each quality level, so we can easily look them up later on
// Based on either qualityLevelRatios or qualityLevelResolutions value set by the user
std::unordered_map<NVSDK_NGX_PerfQuality_Value, std::pair<int, int>> qualityLevelResolutionsCurrent =
{
	{NVSDK_NGX_PerfQuality_Value_UltraPerformance, {0,0}},
	{NVSDK_NGX_PerfQuality_Value_MaxPerf, {0,0}},
	{NVSDK_NGX_PerfQuality_Value_Balanced, {0,0}},
	{NVSDK_NGX_PerfQuality_Value_MaxQuality, {0,0}},

	// see note about UltraQuality in qualityLevelRatios section above
	{NVSDK_NGX_PerfQuality_Value_UltraQuality, {0,0}},
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
			spdlog::info("DLLPathOverrides: redirecting {} to new path {}", libPath.string(), pair.second.string());

			if (utility::exists_safe(pair.second))
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
		// If user has overridden the nvngx_dlss path, use the filename they specified in our comparisons below
		std::wstring dlssName = DlssFileName;
		std::wstring dlssgName = DlssgFileName;

		if (settings.dllPathOverrides.count(DlssFileNameA))
			dlssName = settings.dllPathOverrides[DlssFileNameA].filename().wstring();
		if (settings.dllPathOverrides.count(DlssgFileNameA))
			dlssgName = settings.dllPathOverrides[DlssgFileNameA].filename().wstring();

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
		else if (!_wcsicmp(dllName.c_str(), dlssgName.c_str()))
		{
			nvngx_dlssg::init((HMODULE)notification_data->Loaded.DllBase);
		}
	}
}

void UserSettings::print_to_log()
{
	using namespace utility;

	spdlog::info("Settings:");
	spdlog::info(" - VerboseLogging: {}", verboseLogging ? "true" : "false");
	spdlog::info(" - ForceDLAA: {}{}", forceDLAA ? "true" : "false", overrideQualityLevels ? " (overridden by DLSSQualityLevels section)" : "");
	spdlog::info(" - OverrideAutoExposure: {}", overrideAutoExposure == 0 ? "default" : (overrideAutoExposure > 0 ? "enable" : "disable"));
	if (overrideSharpeningForceDisable)
		spdlog::info(" - OverrideSharpening: disable (force disabling sharpness flag)");
	else if (overrideSharpening.has_value())
		spdlog::info(" - OverrideSharpening: {}", *overrideSharpening);
	spdlog::info(" - OverrideAppId: {}", overrideAppId ? "true" : "false");
	spdlog::info(" - OverrideDlssHud: {}", overrideDlssHud == 0 ? "default" : (overrideDlssHud > 0 ? "enable" : "disable"));
	spdlog::info(" - DisableDevWatermark: {}", disableDevWatermark ? "true" : "false");
	spdlog::info(" - ResolutionOffset: {}", resolutionOffset);
	spdlog::info(" - DynamicResolutionOverride: {}", dynamicResolutionOverride ? "true" : "false");
	spdlog::info(" - DynamicResolutionMinOffset: {}", dynamicResolutionMinOffset);
	spdlog::info(" - DisableIniMonitoring: {}", disableIniMonitoring ? "true" : "false");
	spdlog::info(" - DLSSQualityLevels enabled: {}", overrideQualityLevels ? "true" : "false");

	if (overrideQualityLevels)
	{
		for (int i = 0; i < qualityLevelCount; i++)
		{
			auto level = qualityLevels[i];
			auto& res = qualityLevelResolutions[level];
			if (utility::ValidResolution(res))
				spdlog::info("  - {} resolution: {}x{}", qualityLevelNames[level], res.first, res.second);
			else
				spdlog::info("  - {} ratio: {}", qualityLevelNames[level], qualityLevelRatios[level]);
		}
	}

	// only print presets if any of them have been changed
	if (presetDLAA || presetQuality || presetBalanced || presetPerformance || presetUltraPerformance || presetUltraQuality)
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
		if (presetUltraQuality)
			spdlog::info("  - UltraQuality: {}", DLSS_PresetEnumToName(presetUltraQuality));
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

	std::string sharpeningString = utility::ini_get_string_safe(ini, "DLSS", "OverrideSharpening", std::move(overrideSharpeningString));
	if (!sharpeningString.length() || !_stricmp(sharpeningString.c_str(), "default") ||
		!_stricmp(sharpeningString.c_str(), "ignore") || !_stricmp(sharpeningString.c_str(), "ignored"))
	{
		overrideSharpening.reset();
		overrideSharpeningString = sharpeningString;
		overrideSharpeningForceDisable = false;
	}
	else if (!_stricmp(sharpeningString.c_str(), "disable") || !_stricmp(sharpeningString.c_str(), "disabled"))
	{
		*overrideSharpening = 0;
		overrideSharpeningString = sharpeningString;
		overrideSharpeningForceDisable = true;
	}
	else
	{
		try
		{
			float sharpeningValue = utility::stof_nolocale(sharpeningString, true);
			overrideSharpening = std::clamp(sharpeningValue, -1.0f, 1.0f);
			overrideSharpeningString = sharpeningString;
			overrideSharpeningForceDisable = false;
		}
		catch (const std::exception&)
		{
			spdlog::error("OverrideSharpening: invalid value \"{}\" specified, leaving value as {}", sharpeningString, overrideSharpeningString);
		}
	}

	overrideDlssHud = ini.Get<int>("DLSS", "OverrideDlssHud", std::move(overrideDlssHud));
	disableDevWatermark = ini.Get<bool>("DLSS", "DisableDevWatermark", std::move(disableDevWatermark));
	verboseLogging = ini.Get<bool>("DLSS", "VerboseLogging", std::move(verboseLogging));

	std::set<std::string> keys;
	// [DLLPathOverrides]
	try
	{
		keys = ini.Keys("DLLPathOverrides");
	}
	catch (const std::runtime_error&)
	{
		// No [DLLPathOverrides] section, or section is empty, ignore for now
		keys.clear();
	}

	for (auto& key : keys)
	{
		auto dllFileName = key;
		if (!std::filesystem::path(dllFileName).has_extension())
			dllFileName += ".dll";

		auto path = utility::ini_get_string_safe(ini, "DLLPathOverrides", key, "");
		if (!path.empty() && !utility::exists_safe(path)) // empty path is allowed so that user can clear the override in local INIs
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
		for (int i = 0; i < qualityLevelCount; i++)
		{
			auto level = qualityLevels[i];
			auto& curLevel = qualityLevelNames[level];

			auto value = utility::ini_get_string_safe(ini, "DLSSQualityLevels", curLevel, std::move(qualityLevelStrings[level]));

			// Remove any quotes from around the value
			if (value.size() > 0)
			{
				if (value[0] == '"')
					value = value.substr(1);
				if (value[value.length() - 1] == '"')
					value = value.substr(0, value.length() - 1);
			}

			// Try parsing users string as a resolution
			auto res = utility::ParseResolution(value);
			if (utility::ValidResolution(res))
			{
				qualityLevelResolutions[level] = res;
				qualityLevelStrings[level] = value;
				continue;
			}

			// Not a resolution, try parsing as float
			try
			{
				float float_value = utility::stof_nolocale(value, true);
				qualityLevelRatios[level] = float_value;
				qualityLevelStrings[level] = value;
			}
			catch (const std::exception&)
			{
				spdlog::error("DLSSQualityLevels: level \"{}\" has invalid value \"{}\" specified, leaving value as {}", curLevel, value, qualityLevelRatios[level]);
				continue;
			}

			// Clamp value between 0.0 - 1.0
			qualityLevelRatios[level] = std::clamp(qualityLevelRatios[level], 0.0f, 1.0f);
		}
	}

	// [DLSSPresets]
	presetDLAA = DLSS_PresetNameToEnum(utility::ini_get_string_safe(ini, "DLSSPresets", "DLAA", DLSS_PresetEnumToName(presetDLAA)));
	presetQuality = DLSS_PresetNameToEnum(utility::ini_get_string_safe(ini, "DLSSPresets", "Quality", DLSS_PresetEnumToName(presetQuality)));
	presetBalanced = DLSS_PresetNameToEnum(utility::ini_get_string_safe(ini, "DLSSPresets", "Balanced", DLSS_PresetEnumToName(presetBalanced)));
	presetPerformance = DLSS_PresetNameToEnum(utility::ini_get_string_safe(ini, "DLSSPresets", "Performance", DLSS_PresetEnumToName(presetPerformance)));
	presetUltraPerformance = DLSS_PresetNameToEnum(utility::ini_get_string_safe(ini, "DLSSPresets", "UltraPerformance", DLSS_PresetEnumToName(presetUltraPerformance)));
	presetUltraQuality = DLSS_PresetNameToEnum(utility::ini_get_string_safe(ini, "DLSSPresets", "UltraQuality", DLSS_PresetEnumToName(presetUltraQuality)));

	// [Compatibility]
	resolutionOffset = ini.Get<int>("Compatibility", "ResolutionOffset", std::move(resolutionOffset));
	dynamicResolutionOverride = ini.Get<bool>("Compatibility", "DynamicResolutionOverride", std::move(dynamicResolutionOverride));
	dynamicResolutionMinOffset = ini.Get<int>("Compatibility", "DynamicResolutionMinOffset", std::move(dynamicResolutionMinOffset));
	disableIniMonitoring = ini.Get<bool>("Compatibility", "DisableIniMonitoring", std::move(disableIniMonitoring));
	overrideAppId = ini.Get<bool>("Compatibility", "OverrideAppId", std::move(overrideAppId));

	auto log_level = verboseLogging ? spdlog::level::debug : spdlog::level::info;
#ifdef _DEBUG
	log_level = spdlog::level::debug;
#endif
	if (spdlog::default_logger())
		spdlog::default_logger()->set_level(log_level);
	spdlog::set_level(log_level);

	// Let our module hooks/patches know about new settings if needed
	nvngx_dlssg::settings_changed();

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

	if (settings.disableAllTweaks)
	{
		// Try warning user via error log file
		// (this used to use a Win32 MessageBox too, but some fullscreen games had issues when MessageBox showed, even in seperate thread, so that was cut)
		std::string warningText =
			"Warning: multiple versions of DLSSTweaks have attempted to load into the game process.\n\nIf you recently tried to update the DLL, an older version may still be present & loaded in.\n\nCheck your game folder for files such as dxgi.dll / xinput1_3.dll / nvngx.dll and remove any extra versions.";

		try
		{
			std::ofstream warningFile(ExePath.parent_path() / ErrorFileName);
			warningFile << warningText << "\r\n";
			warningFile.close();

			std::ofstream warningFileDll(DllPath.parent_path() / ErrorFileName);
			warningFileDll << warningText << "\r\n";
			warningFileDll.close();
		}
		catch (const std::exception&)
		{
		}

		// Skip InitThread if disableAllTweaks was set...
		{
			std::lock_guard lock(initThreadFinishedMutex);
			initThreadFinished = true;
			initThreadFinishedVar.notify_all();
		}

		return 0;
	}

	// spdlog setup
	{
		// Log is always written next to our DLL
		LogPath = DllPath.parent_path() / LogFileName;

		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>(true));
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
		combined_logger->set_level(spdlog::level::info);
		spdlog::set_default_logger(combined_logger);
		spdlog::flush_on(spdlog::level::debug);
	}

	std::string version = "0.200.5";
	try
	{
		std::string fileVersion = utility::ModuleVersion(DllPath);
		version = std::move(fileVersion);
	}
	catch (const std::exception&)
	{
	}

	spdlog::info("DLSSTweaks v{}, by emoose: {} wrapper loaded", version, DllPath.filename().string());
	spdlog::info("Game path: {}", ExePath.string());
	spdlog::info("DLL path: {}", DllPath.string());

	spdlog::info("---");

	// Read config from next to DLL first, and then from next to EXE
	// So with a global injector, you could keep a global config stored next to the DLL, and then per-game overrides kept next to the EXE
	{
		if (DllPath.parent_path() != ExePath.parent_path())
		{
			IniPath = DllPath.parent_path() / IniFileName;
			if (utility::exists_safe(IniPath))
			{
				if (settings.read(IniPath))
					spdlog::info("Config read from {}", IniPath.string());
				else
					spdlog::error("Failed to read config from {}", IniPath.string());
			}
		}

		IniPath = ExePath.parent_path() / IniFileName;
		if (utility::exists_safe(IniPath))
		{
			if (settings.read(IniPath))
				spdlog::info("Config read from {}", IniPath.string());
			else
				spdlog::error("Failed to read config from {}", IniPath.string());
		}

		// IniPath will point to the INI next to game EXE after this, so that we can monitor any updates to that INI
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

	if (settings.disableIniMonitoring)
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
		spdlog::error("INI monitoring: CreateFileW \"{}\" failed with error code {}", IniPath.parent_path().string(), err);
		return 0;
	}

	OVERLAPPED overlapped;
	overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
	if (!overlapped.hEvent)
	{
		DWORD err = GetLastError();
		spdlog::error("INI monitoring: CreateEvent failed with error code {}", err);
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
		spdlog::error("INI monitoring: ReadDirectoryChangesW failed with error code {}", err);
		return 0;
	}

	spdlog::info("INI monitoring: watching for INI updates...");

	while (success)
	{
		DWORD result = WaitForSingleObject(overlapped.hEvent, INFINITE);

		if (result == WAIT_OBJECT_0)
		{
			DWORD bytes_transferred;
			GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

			FILE_NOTIFY_INFORMATION* evt = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(change_buf);

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
std::mutex processUniqueMutexGuard;

// Tries to create a process-unique Win32 mutex
// If mutex already exists then another instance of this module must have already been loaded in
BOOL CheckDllAlreadyLoaded()
{
	std::scoped_lock lock{ processUniqueMutexGuard };

	if (processUniqueMutex) // If already set we must be the owner, so exit out
		return FALSE;

	// Generate unique name for the mutex based on the current process ID
	TCHAR szMutexName[MAX_PATH];
	_sntprintf_s(szMutexName, MAX_PATH, _T("Local\\DLSSTweaksMutex_%lu"), GetCurrentProcessId());

	processUniqueMutex = CreateMutex(NULL, TRUE, szMutexName);
	if (processUniqueMutex == NULL)
		return FALSE;

	DWORD dwError = GetLastError();
	if (dwError != ERROR_ALREADY_EXISTS)
	{
		// Mutex was created successfully, no other instance of the DLL is loaded in this process
		return FALSE;
	}

	// Another instance of the DLL must already be loaded in this process
	CloseHandle(processUniqueMutex); // Not using ReleaseMutex since we want to keep the mutex alive, just closing the handle
	processUniqueMutex = NULL;
	return TRUE;
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

			// We'll alert user to the issue during InitThread, to prevent us from blocking game init
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
