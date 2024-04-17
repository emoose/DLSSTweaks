#pragma once
#include <SafetyHook.hpp>
#include "Utility.hpp"
#include <filesystem>
#include <unordered_map>
#include <nvsdk_ngx_defs.h>
#include <nvsdk_ngx_params.h>

// Certain settings which aren't currently included in DLSS SDK, but do seem checked by DLSS 3.1+ DLL files.
#ifndef NVSDK_NGX_Parameter_Disable_Watermark
#define NVSDK_NGX_Parameter_Disable_Watermark "Disable.Watermark"
#endif
#ifndef NVSDK_NGX_Parameter_DLSS_Get_Dynamic
#define NVSDK_NGX_Parameter_DLSS_Get_Dynamic "DLSS.Get.Dynamic."
#endif

constexpr float DLSS_MinScale = 0.0f;
constexpr float DLSS_MaxScale = 1.0f;

struct DlssNvidiaPresetOverrides
{
	uint32_t overrideDLAA;
	uint32_t overrideQuality;
	uint32_t overrideBalanced;
	uint32_t overridePerformance;
	uint32_t overrideUltraPerformance;
	
	void zero_customized_values();
};

struct QualityLevel
{
	std::string name;
	float scalingRatio;
	std::pair<int, int> resolution = { 0,0 };

	// The last resolution we told game about for the this level, so we can check against it later on
	// (based on either `scalingRatio` or `resolution` set by the user)
	std::pair<int, int> currentResolution = { 0,0 };

	unsigned int preset = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;

	std::string lastUserValue = "";
};

struct DlssSettings
{
	NVSDK_NGX_PerfQuality_Value prevQualityLevel; // the last quality level setting that game requested
	std::optional<ID3D12Resource*> prevExposureTexture;

	int featureCreateFlags = 0;
	std::optional<DlssNvidiaPresetOverrides> nvidiaOverrides;
	unsigned long long appId = 0;
	std::string projectId;

	// app ID displayed by DLSS overlay is xored for some reason
	unsigned long long appIdDlss() const
	{
		return appId ^ 0xE658703;
	}
};

struct UserSettings
{
	bool disableAllTweaks = false; // not exposed in INI, is set if a serious error is detected (eg. two versions loaded at once)

	std::unordered_map<NVSDK_NGX_PerfQuality_Value, QualityLevel> qualities =
	{
		{NVSDK_NGX_PerfQuality_Value_UltraPerformance, {"UltraPerformance", 0.33333334f}},
		{NVSDK_NGX_PerfQuality_Value_MaxPerf, {"Performance", 0.5f}},
		{NVSDK_NGX_PerfQuality_Value_Balanced, {"Balanced", 0.58f}},
		{NVSDK_NGX_PerfQuality_Value_MaxQuality, {"Quality", 0.66666667f}},
		{NVSDK_NGX_PerfQuality_Value_DLAA, {"DLAA", 1.0f}},

		// note: if NVSDK_NGX_PerfQuality_Value_UltraQuality is non-zero, some games may detect that we're passing a valid resolution and show an Ultra Quality option as a result
		// very few games support this though, and right now DLSS seems to refuse to render if UltraQuality gets passed to it
		// our SetI hook in HooksNvngx can override the quality passed to DLSS if this gets used by the game, letting it think this is MaxQuality instead
		// but we'll only do that if user has overridden this in the INI to a non-zero value
		{NVSDK_NGX_PerfQuality_Value_UltraQuality, {"UltraQuality", 0.f}},
	};

	bool forceDLAA = false;
	int overrideAutoExposure = 0;
	int overrideAlphaUpscaling = 0;
	std::optional<float> overrideSharpening{};
	std::string overrideSharpeningString = "ignore";
	bool overrideSharpeningForceDisable = false;
	bool overrideAppId = false;
	int overrideDlssHud = 0;
	bool disableDevWatermark = false;
	bool verboseLogging = false;
	std::unordered_map<std::string, std::filesystem::path> dllPathOverrides;
	bool overrideQualityLevels = false;

	int resolutionOffset = 0; // user-defined offset to apply to DLAA / full-res rendering (some titles don't like DLAA rendering at full res, so small offset is needed)
	bool dynamicResolutionOverride = true;
	int dynamicResolutionMinOffset = -1;
	bool disableIniMonitoring = false;

	bool read(const std::filesystem::path& iniPath, int numInisRead = 0);
	void print_to_log();
	void watch_for_changes(const std::filesystem::path& iniPath);
};

// DllMain.cpp / UserSettings.cpp
extern UserSettings settings;
extern DlssSettings dlss;
void WaitForInitThread();

// HooksNvngx.cpp
namespace nvngx
{
void hook_params(NVSDK_NGX_Parameter* params);
void init_from_proxy();
void init(HMODULE ngx_module);
};

// module_hooks/*
namespace nvngx_dlss
{
void init(HMODULE ngx_module);
};
namespace nvngx_dlssg
{
void settings_changed();
void init(HMODULE ngx_module);
};
namespace nvngx_dlssd
{
void init(HMODULE ngx_module);
};

// wrapper struct on top of SafetyHookInline
// can either call the hook trampoline via SafetyHookInline, or call a specified function if dest_proc is set
struct HookOrigFn
{
	SafetyHookInline hook{};
	FARPROC dest_proc = nullptr;

	std::recursive_mutex m_mutex{};

	// only resets inline hook, as resetting both hook & dest_proc would leave this with no function to call, causing issues
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
