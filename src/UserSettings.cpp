#include <spdlog/spdlog.h>
#include <ini.h>

#include "DLSSTweaks.hpp"

void UserSettings::print_to_log()
{
	using namespace utility;

	spdlog::info("Settings:");
	spdlog::info(" - VerboseLogging: {}", verboseLogging ? "true" : "false");
	spdlog::info(" - ForceDLAA: {}{}", forceDLAA ? "true" : "false", overrideQualityLevels ? " (overridden by DLSSQualityLevels section)" : "");
	spdlog::info(" - OverrideAutoExposure: {}", overrideAutoExposure == 0 ? "default" : (overrideAutoExposure > 0 ? "enable" : "disable"));
	spdlog::info(" - OverrideAlphaUpscaling: {}", overrideAlphaUpscaling == 0 ? "default" : (overrideAlphaUpscaling > 0 ? "enable" : "disable"));
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
		for(const auto& quality : settings.qualities)
		{
			auto& res = quality.second.resolution;
			if (utility::ValidResolution(res))
				spdlog::info("  - {} resolution: {}x{}", quality.second.name, res.first, res.second);
			else
				spdlog::info("  - {} ratio: {}", quality.second.name, quality.second.scalingRatio);
		}
	}

	// only print presets if any of them have been changed
	bool changed = false;
	for (const auto& [level, quality] : qualities)
	{
		if (quality.preset != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		{
			changed = true;
			break;
		}
	}

	if (changed)
	{
		spdlog::info(" - DLSSPresets:");
		for (const auto& kvp : qualities)
		{
			if(kvp.second.preset != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
			{
				spdlog::info("  - {}: {}", kvp.second.name, DLSS_PresetEnumToName(kvp.second.preset));
			}
		}
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
	overrideAlphaUpscaling = ini.Get<int>("DLSS", "OverrideAlphaUpscaling", std::move(overrideAlphaUpscaling));

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
		for(auto& kvp : settings.qualities)
		{
			auto& quality = kvp.second;

			auto value = utility::ini_get_string_safe(ini, "DLSSQualityLevels", quality.name, std::move(quality.lastUserValue));

			// Try parsing users string as a resolution
			const auto res = utility::ParseResolution(value);
			if (utility::ValidResolution(res))
			{
				quality.resolution = res;
				quality.lastUserValue = value;
				continue;
			}
			 
			// Not a resolution, try parsing as float
			try
			{
				float floatValue = utility::stof_nolocale(value, true);
				quality.scalingRatio = floatValue;
				quality.lastUserValue = value;
			}
			catch (const std::exception&)
			{
				spdlog::error(R"(DLSSQualityLevels: level "{}" has invalid value "{}" specified, leaving value as {})", quality.name, value, quality.scalingRatio);
				continue;
			}

			// Clamp value between 0.0 - 1.0
			quality.scalingRatio = std::clamp(quality.scalingRatio, DLSS_MinScale, DLSS_MaxScale);
		}
	}

	// [DLSSPresets]
	for (auto& kvp : qualities)
	{
		kvp.second.preset = DLSS_PresetNameToEnum(utility::ini_get_string_safe(ini, "DLSSPresets", kvp.second.name, DLSS_PresetEnumToName(kvp.second.preset)));
	}

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
							if (read(iniPath))
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

void DlssNvidiaPresetOverrides::zero_customized_values()
{
	// DlssNvidiaPresetOverrides struct we change seems to last the whole lifetime of the game
	// So we'll back up the orig values in case user later decides to set them back to default
	if (!dlss.nvidiaOverrides.has_value())
	{
		dlss.nvidiaOverrides = *this;

		if (overrideDLAA || overrideQuality || overrideBalanced || overridePerformance || overrideUltraPerformance)
		{
			spdlog::debug("NVIDIA default presets for current app:");
			if (overrideDLAA)
				spdlog::debug(" - DLAA: {}", utility::DLSS_PresetEnumToName(overrideDLAA));
			if (overrideQuality)
				spdlog::debug(" - Quality: {}", utility::DLSS_PresetEnumToName(overrideQuality));
			if (overrideBalanced)
				spdlog::debug(" - Balanced: {}", utility::DLSS_PresetEnumToName(overrideBalanced));
			if (overridePerformance)
				spdlog::debug(" - Performance: {}", utility::DLSS_PresetEnumToName(overridePerformance));
			if (overrideUltraPerformance)
				spdlog::debug(" - UltraPerformance: {}", utility::DLSS_PresetEnumToName(overrideUltraPerformance));
		}
	}

	// Then zero out NV-provided override if user has set their own override for that level
	overrideDLAA = settings.qualities[NVSDK_NGX_PerfQuality_Value_DLAA].preset ? 0 : dlss.nvidiaOverrides->overrideDLAA;
	overrideQuality = settings.qualities[NVSDK_NGX_PerfQuality_Value_MaxQuality].preset ? 0 : dlss.nvidiaOverrides->overrideQuality;
	overrideBalanced = settings.qualities[NVSDK_NGX_PerfQuality_Value_Balanced].preset ? 0 : dlss.nvidiaOverrides->overrideBalanced;
	overridePerformance = settings.qualities[NVSDK_NGX_PerfQuality_Value_MaxPerf].preset ? 0 : dlss.nvidiaOverrides->overridePerformance;
	overrideUltraPerformance = settings.qualities[NVSDK_NGX_PerfQuality_Value_UltraPerformance].preset ? 0 : dlss.nvidiaOverrides->overrideUltraPerformance;
}
