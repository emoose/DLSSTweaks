#pragma once
#include <SafetyHook.hpp>
#include "Utility.hpp"
#include <filesystem>
#include <unordered_map>

struct UserSettings
{
	bool forceDLAA = false;
	int overrideAutoExposure = 0;
	bool overrideAppId = false;
	int overrideDlssHud = 0;
	bool disableDevWatermark = false;
	std::filesystem::path overrideDlssDll = "";
	bool watchIniUpdates = false;
	bool overrideQualityLevels = false;
	unsigned int presetDLAA = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetQuality = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetBalanced = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	unsigned int presetUltraPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;

	void print_to_log();
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

		uintptr_t dest = dest_proc ? uintptr_t(dest_proc) : hook.trampoline();
		return ((RetT(*)(Args...))dest)(args...);
	}

	template <typename RetT = void, typename... Args> auto unsafe_call(Args... args) {
		uintptr_t dest = dest_proc ? uintptr_t(dest_proc) : hook.trampoline();
		return ((RetT(*)(Args...))dest)(args...);
	}
};
