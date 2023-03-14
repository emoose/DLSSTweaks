#pragma once
#include <SafetyHook.hpp>
#include "Utility.hpp"
#include <filesystem>
#include <unordered_map>

struct UserSettings
{
	bool watchIniUpdates = false;
	bool forceDLAA = false;
	int overrideAutoExposure = 0;
	int overrideDlssHud = 0;
	bool disableDevWatermark = false;
	bool overrideAppId = false;
	std::filesystem::path overrideDlssDll = "";
	bool overrideQualityLevels = false;
	unsigned int presetDLAA = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetQuality = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetBalanced = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetUltraPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
};

// DllMain.cpp
extern UserSettings settings;
extern std::unordered_map<NVSDK_NGX_PerfQuality_Value, float> qualityLevelRatios;
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

// wrapper struct on top of SafetyHookInline
// can either call the hook trampoline via SafetyHookInline, or call a specified function if dest_proc is set
struct HookOrigFn
{
	SafetyHookInline hook{};
	FARPROC dest_proc = nullptr;

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
		if (dest_proc)
			return ((RetT(*)(Args...))dest_proc)(args...);
		return hook.call<RetT>(args...);
	}

	template <typename RetT = void, typename... Args> auto unsafe_call(Args... args) {
		if (dest_proc)
			return ((RetT(*)(Args...))dest_proc)(args...);
		return hook.unsafe_call<RetT>(args...);
	}
};
