#include <spdlog/spdlog.h>
#include <ini.h>

#include "DLSSTweaks.hpp"

constexpr int qualityLevelCount = 5;

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

	if (!settings.dllPathOverrides.empty())
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

bool UserSettings::read(const std::filesystem::path& iniPath, int numInisRead)
{
	using namespace utility;

	const std::wstring iniPathStr = iniPath.wstring();

	// Read INI via FILE* since INIReader doesn't support wstring
	FILE* iniFile;
	if (_wfopen_s(&iniFile, iniPathStr.c_str(), L"r") != 0 || !iniFile)
		return false;

	inih::INIReader ini(iniFile);
	fclose(iniFile);

	// [DLSSTweaks]

	// BaseINI: specifies an INI file which will be read in before the rest of the INI
	// acting as a sort of global config file if the path has been set up
	auto baseIni = utility::ini_get_string_safe(ini, "DLSSTweaks", "BaseINI", "");
	if (!baseIni.empty())
	{
		// make sure we aren't trying to read in the current INI that we're reading...
		if (std::filesystem::absolute(iniPath) != std::filesystem::absolute(baseIni))
		{
			if (numInisRead > 10)
				spdlog::error("BaseINI: followed too many base INIs, might be caught in a loop, skipping further base INIs..");
			else
			{
				if (read(baseIni, numInisRead + 1))
					spdlog::info("Config read from {}", baseIni);
			}
		}
	}

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
			if (!value.empty())
			{
				if (value[0] == '"')
					value = value.substr(1);
				if (value[value.length() - 1] == '"')
					value = value.substr(0, value.length() - 1);
			}

			// Try parsing users string as a resolution
			const auto res = utility::ParseResolution(value);
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

void UserSettings::watch_for_changes(const std::filesystem::path& iniPath)
{
	const std::wstring iniFileName = iniPath.filename().wstring();
	const std::wstring iniFolder = iniPath.parent_path().wstring();
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
		spdlog::error("INI monitoring: CreateFileW \"{}\" failed with error code {}", iniPath.parent_path().string(), err);
		return;
	}

	OVERLAPPED overlapped;
	overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
	if (!overlapped.hEvent)
	{
		DWORD err = GetLastError();
		spdlog::error("INI monitoring: CreateEvent failed with error code {}", err);
		CloseHandle(file);
		return;
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
		return;
	}

	spdlog::info("INI monitoring: watching for INI updates...");

	while (success)
	{
		DWORD result = WaitForSingleObject(overlapped.hEvent, INFINITE);
		if (result == WAIT_OBJECT_0)
		{
			DWORD bytes_transferred;
			GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

			auto* evt = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(change_buf);

			for (;;)
			{
				if (evt->Action == FILE_ACTION_MODIFIED)
				{
					// evt->FileName isn't null-terminated, so construct wstring for it based on FileNameLength
					std::wstring name = std::wstring(evt->FileName, evt->FileNameLength / sizeof(WCHAR));
					if (!_wcsicmp(name.c_str(), iniFileName.c_str()))
					{
						// INI read might fail if it's still being updated by a text editor etc
						// so try attempting a few times, Sleep(1000) between attempts should hopefully let us read it fine
						int attempts = 3;
						while (attempts--)
						{
							if (settings.read(iniPath))
							{
								spdlog::info("Config updated from {}", iniPath.string());
								break;
							}
							Sleep(1000);
						}
					}
				}

				// Any more events to handle?
				if (evt->NextEntryOffset)
					*(uint8_t**)&evt += evt->NextEntryOffset;
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
}
