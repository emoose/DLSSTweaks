#pragma once
#include <SafetyHook.hpp>
#include "Utility.hpp"
#include <filesystem>
#include <unordered_map>
#include <nvsdk_ngx_defs.h>
#include <nvsdk_ngx_params.h>

// Not included in DLSS SDK, but is mentioned inside & seems to be checked by the DLSS 3.1 DLLs
// (unfortunately not every version checks it though...)
#define NVSDK_NGX_Parameter_Disable_Watermark "Disable.Watermark"

struct DlssNvidiaPresetOverrides
{
	uint32_t overrideDLAA;
	uint32_t overrideQuality;
	uint32_t overrideBalanced;
	uint32_t overridePerformance;
	uint32_t overrideUltraPerformance;
	
	void zero_customized_values();
};
struct DlssSettings
{
	int featureCreateFlags = 0;
	std::optional<DlssNvidiaPresetOverrides> nvidiaOverrides;
};
struct UserSettings
{
	DlssSettings dlss; // DLSS related settings, setup by game or DLSS itself
	bool disableAllTweaks = false; // not exposed in INI, is set if a serious error is detected (eg. two versions loaded at once)

	bool forceDLAA = false;
	int overrideAutoExposure = 0;
	bool overrideAppId = false;
	int overrideDlssHud = 0;
	bool disableDevWatermark = false;
	bool verboseLogging = false;
	std::unordered_map<std::string, std::filesystem::path> dllPathOverrides;
	bool overrideQualityLevels = false;
	std::string qualityLevelStrings[5] = { 
		"0.5", 
		"0.58", 
		"0.66666667",
		"0.33333334",
		"0"
	};
	unsigned int presetDLAA = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetQuality = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetBalanced = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetUltraPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	int resolutionOffset = 0; // user-defined offset to apply to DLAA / full-res rendering (some titles don't like DLAA rendering at full res, so small offset is needed)
	bool disableIniMonitoring = false;

	bool read(const std::filesystem::path& iniPath);
	void print_to_log();
};

// DllMain.cpp
extern UserSettings settings;
extern std::unordered_map<NVSDK_NGX_PerfQuality_Value, float> qualityLevelRatios;
extern std::unordered_map<NVSDK_NGX_PerfQuality_Value, std::pair<int, int>> qualityLevelResolutions;
void WaitForInitThread();

// HooksNvngx.cpp
namespace nvngx
{
void hook_params(NVSDK_NGX_Parameter* params);
void init_from_proxy();
void init(HMODULE ngx_module);
};

// HooksNvngxDlss.cpp
namespace nvngx_dlss
{
void init(HMODULE ngx_module);
};
namespace nvngx_dlssg
{
void settings_changed();
void init(HMODULE ngx_module);
};

// wrapper struct on top of SafetyHookInline
// can either call the hook trampoline via SafetyHookInline, or call a specified function if dest_proc is set
struct HookOrigFn
{
	SafetyHookInline hook{};
	FARPROC dest_proc = nullptr;

	std::recursive_mutex m_mutex{};

	// only resets inline hook, as unsetting both hook & dest_proc would leave this with no function to call, causing issues
	void reset()
	{
		hook.reset();
	}

	HookOrigFn& operator=(SafetyHookInline other) noexcept
	{
		reset();
		dest_proc = nullptr;
		std::swap(hook, other);
		return *this;
	}

	HookOrigFn& operator=(FARPROC other) noexcept
	{
		reset();
		std::swap(dest_proc, other);
		return *this;
	}

	template <typename RetT = void, typename... Args> auto call(Args... args) {
		std::scoped_lock lock{m_mutex};

		uintptr_t dest = dest_proc ? uintptr_t(dest_proc) : hook.trampoline().address();
		return ((RetT(*)(Args...))dest)(args...);
	}

	template <typename RetT = void, typename... Args> auto unsafe_call(Args... args) {
		uintptr_t dest = dest_proc ? uintptr_t(dest_proc) : hook.trampoline().address();
		return ((RetT(*)(Args...))dest)(args...);
	}
};
