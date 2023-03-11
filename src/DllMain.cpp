#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <unordered_map>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <SafetyHook.hpp>
#include <Patterns.h>
#include <ini.h>

#include "Utility.hpp"
#include "Proxy.hpp"

const wchar_t* NgxFileName = L"_nvngx.dll";
const wchar_t* DlssFileName = L"nvngx_dlss.dll";

const wchar_t* LogFileName = L"dlsstweaks.log";
const wchar_t* IniFileName = L"dlsstweaks.ini";

std::filesystem::path ExePath;
std::filesystem::path DllPath;
std::filesystem::path LogPath;
std::filesystem::path IniPath;

// User settings
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

std::unordered_map<NVSDK_NGX_PerfQuality_Value, float> qualityLevelRatios =
{
	{NVSDK_NGX_PerfQuality_Value_UltraPerformance, 0.33333334f},
	{NVSDK_NGX_PerfQuality_Value_MaxPerf, 0.5f},
	{NVSDK_NGX_PerfQuality_Value_Balanced, 0.58f},
	{NVSDK_NGX_PerfQuality_Value_MaxQuality, 0.66666667f},

	// note: if NVSDK_NGX_PerfQuality_Value_UltraQuality is non-zero, some games may detect that we're passing a valid resolution and show an Ultra Quality option as a result
	// very few games support this though, and right now DLSS seems to refuse to render if UltraQuality gets passed to it
	// our SetI hook below can override the quality passed to DLSS if this gets used by the game, letting it think this is MaxQuality instead
	// but we'll only do that if user overrides this in the INI to a non-zero value
	{NVSDK_NGX_PerfQuality_Value_UltraQuality, 0.f},
};

const char* projectIdOverride = "24480451-f00d-face-1304-0308dabad187";
const unsigned long long appIdOverride = 0x24480451;

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

// the hooks defined below work in two different methods
// method 0: if we were loaded in via nvngx wrapper (requires reg edit), the hooks below are exported as the nvngx functions themselves, so we can do our work and then call back to original DLL
// method 1: if we were loaded in via other DLL wrapper (dxgi etc), then _nvngx.dll was hooked to call these functions instead, and call back via safetyhook unsafe_call
// these are handled via either nvngx::hook or nvngx::init setting up the HookOrigFn structs for each function
namespace nvngx
{
NVSDK_NGX_PerfQuality_Value prevQualityValue; // prev value set by game

// TODO: InFeatureInfo might also hold project ID related fields, maybe should change those too...
HookOrigFn NVSDK_NGX_D3D11_Init_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const void* InFeatureInfo, void* InSDKVersion)
{
	WaitForInitThread();

	if (overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D11_Init_Hook.unsafe_call<uint64_t>(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
HookOrigFn NVSDK_NGX_D3D11_Init_Ext_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	WaitForInitThread();

	if (overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D11_Init_Ext_Hook.unsafe_call<uint64_t>(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
HookOrigFn NVSDK_NGX_D3D11_Init_ProjectID_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, enum NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const struct NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, enum NVSDK_NGX_Version InSDKVersion)
{
	WaitForInitThread();

	if (overrideAppId)
		InProjectId = projectIdOverride;

	return NVSDK_NGX_D3D11_Init_ProjectID_Hook.unsafe_call<uint64_t>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

HookOrigFn NVSDK_NGX_D3D12_Init_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const void* InFeatureInfo, void* InSDKVersion)
{
	WaitForInitThread();

	if (overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D12_Init_Hook.unsafe_call<uint64_t>(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
HookOrigFn NVSDK_NGX_D3D12_Init_Ext_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	WaitForInitThread();

	if (overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D12_Init_Ext_Hook.unsafe_call<uint64_t>(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
HookOrigFn NVSDK_NGX_D3D12_Init_ProjectID_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D12_Init_ProjectID(const char* InProjectId, enum NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const struct NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, enum NVSDK_NGX_Version InSDKVersion)
{
	WaitForInitThread();

	if (overrideAppId)
		InProjectId = projectIdOverride;

	return NVSDK_NGX_D3D12_Init_ProjectID_Hook.unsafe_call<uint64_t>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

HookOrigFn NVSDK_NGX_VULKAN_Init_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6)
{
	WaitForInitThread();

	if (overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_VULKAN_Init_Hook.unsafe_call<uint64_t>(InApplicationId, a2, a3, a4, a5, a6);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_Ext_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Ext(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
{
	WaitForInitThread();

	if (overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_VULKAN_Init_Ext_Hook.unsafe_call<uint64_t>(InApplicationId, a2, a3, a4, a5, a6, a7);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_Ext2_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Ext2(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_VULKAN_Init_Ext2_Hook.unsafe_call<uint64_t>(InApplicationId, a2, a3, a4, a5, a6, a7, a8, a9);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_ProjectID_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_Init_ProjectID(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9)
{
	WaitForInitThread();

	if (overrideAppId)
		InProjectId = projectIdOverride;

	return NVSDK_NGX_VULKAN_Init_ProjectID_Hook.unsafe_call<uint64_t>(InProjectId, a2, a3, a4, a5, a6, a7, a8, a9);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_Init_ProjectID_Ext(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9, void* a10, void* a11)
{
	WaitForInitThread();

	if (overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook.unsafe_call<uint64_t>(InProjectId, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

SafetyHookInline NVSDK_NGX_Parameter_SetI_Hook;
void __cdecl NVSDK_NGX_Parameter_SetI(void* InParameter, const char* InName, int InValue)
{
	if (overrideAutoExposure != 0 && !_stricmp(InName, NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags))
	{
		if (overrideAutoExposure >= 1) // force auto-exposure
			InValue |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
		else if (overrideAutoExposure < 0) // force disable auto-exposure
			InValue = InValue & ~NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
	}

	// Cache the chosen quality value so we can make decisions on it later on
	// TODO: maybe should setup NVSDK_NGX_Parameter class so we could call GetI to fetch this instead, might be more reliable...
	if (!_stricmp(InName, NVSDK_NGX_Parameter_PerfQualityValue))
	{
		auto value = NVSDK_NGX_PerfQuality_Value(InValue);
		prevQualityValue = value;

		// Some games may expose an UltraQuality option if we returned a valid resolution for it
		// DLSS usually doesn't like UltraQuality being applied though, and will break rendering/crash altogether if set
		// So we'll just tell DLSS to use MaxQuality instead, while keeping UltraQuality stored in prevQualityValue
		if (value == NVSDK_NGX_PerfQuality_Value_UltraQuality && qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraQuality] > 0.f)
			InValue = int(NVSDK_NGX_PerfQuality_Value_MaxQuality);
	}

	NVSDK_NGX_Parameter_SetI_Hook.call(InParameter, InName, InValue);
}

SafetyHookInline NVSDK_NGX_Parameter_SetUI_Hook;
void __cdecl NVSDK_NGX_Parameter_SetUI(void* InParameter, const char* InName, unsigned int InValue)
{
	NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, InName, InValue);

	if (presetDLAA != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, presetDLAA);
	if (presetQuality != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, presetQuality);
	if (presetBalanced != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, presetBalanced);
	if (presetPerformance != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, presetPerformance);
	if (presetUltraPerformance != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, presetUltraPerformance);

	NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Disable_Watermark, disableDevWatermark ? 1 : 0);
}

SafetyHookInline NVSDK_NGX_Parameter_GetUI_Hook;
uint64_t __cdecl NVSDK_NGX_Parameter_GetUI(void* InParameter, const char* InName, unsigned int* OutValue)
{
	uint64_t ret = NVSDK_NGX_Parameter_GetUI_Hook.call<uint64_t>(InParameter, InName, OutValue);
	if (ret != 1)
		return ret;

	bool isOutWidth = !_stricmp(InName, NVSDK_NGX_Parameter_OutWidth);
	bool isOutHeight = !_stricmp(InName, NVSDK_NGX_Parameter_OutHeight);

	// DLAA force by overwriting OutWidth/OutHeight with the full res
	bool overrideWidth = forceDLAA && isOutWidth;
	bool overrideHeight = forceDLAA && isOutHeight;
	if (overrideWidth || overrideHeight)
	{
		if (overrideWidth && *OutValue != 0)
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Width, OutValue);
		if (overrideHeight && *OutValue != 0)
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Height, OutValue);
	}

	// Override with DLSSQualityLevels value if user set it
	if (overrideQualityLevels && qualityLevelRatios.count(prevQualityValue))
	{
		if (isOutWidth)
		{
			unsigned int targetWidth = 0;
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Width, &targetWidth); // fetch full screen width
			targetWidth = unsigned int(roundf(float(targetWidth) * qualityLevelRatios[prevQualityValue])); // calculate new width from custom ratio
			*OutValue = targetWidth;
		}
		if (isOutHeight)
		{
			unsigned int targetHeight = 0;
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Height, &targetHeight); // fetch full screen height
			targetHeight = unsigned int(roundf(float(targetHeight) * qualityLevelRatios[prevQualityValue])); // calculate new height from custom ratio
			*OutValue = targetHeight;
		}
	}

	return ret;
}

void hook_params(NVSDK_NGX_Parameter* params);

HookOrigFn NVSDK_NGX_D3D12_AllocateParameters_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D12_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	uint64_t ret = NVSDK_NGX_D3D12_AllocateParameters_Hook.call<uint64_t>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_D3D12_GetCapabilityParameters_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D12_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	uint64_t ret = NVSDK_NGX_D3D12_GetCapabilityParameters_Hook.call<uint64_t>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_D3D12_GetParameters_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D12_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	uint64_t ret = NVSDK_NGX_D3D12_GetParameters_Hook.call<uint64_t>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}

HookOrigFn NVSDK_NGX_D3D11_AllocateParameters_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D11_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	uint64_t ret = NVSDK_NGX_D3D11_AllocateParameters_Hook.call<uint64_t>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_D3D11_GetCapabilityParameters_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D11_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	uint64_t ret = NVSDK_NGX_D3D11_GetCapabilityParameters_Hook.call<uint64_t>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_D3D11_GetParameters_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	uint64_t ret = NVSDK_NGX_D3D11_GetParameters_Hook.call<uint64_t>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}

HookOrigFn NVSDK_NGX_VULKAN_AllocateParameters_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	uint64_t ret = NVSDK_NGX_VULKAN_AllocateParameters_Hook.call<uint64_t>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	uint64_t ret = NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook.call<uint64_t>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_VULKAN_GetParameters_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	uint64_t ret = NVSDK_NGX_VULKAN_GetParameters_Hook.call<uint64_t>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}

std::mutex paramHookMutex;
void hook_params(NVSDK_NGX_Parameter* params)
{
	std::scoped_lock lock{paramHookMutex};

	if (NVSDK_NGX_Parameter_SetI_Hook && NVSDK_NGX_Parameter_SetUI_Hook && NVSDK_NGX_Parameter_GetUI_Hook)
		return;

	auto* NVSDK_NGX_Parameter_SetI_orig = params ? (params->_vftable ? params->_vftable->SetI : nullptr) : nullptr;
	auto* NVSDK_NGX_Parameter_SetUI_orig = params ? (params->_vftable ? params->_vftable->SetUI : nullptr) : nullptr;
	auto* NVSDK_NGX_Parameter_GetUI_orig = params ? (params->_vftable ? params->_vftable->GetUI : nullptr) : nullptr;

	if (NVSDK_NGX_Parameter_SetI_orig && NVSDK_NGX_Parameter_SetUI_orig && NVSDK_NGX_Parameter_GetUI_orig)
	{
		{
			auto builder = SafetyHookFactory::acquire();
			NVSDK_NGX_Parameter_SetI_Hook = builder.create_inline(NVSDK_NGX_Parameter_SetI_orig, NVSDK_NGX_Parameter_SetI);
			NVSDK_NGX_Parameter_SetUI_Hook = builder.create_inline(NVSDK_NGX_Parameter_SetUI_orig, NVSDK_NGX_Parameter_SetUI);
			NVSDK_NGX_Parameter_GetUI_Hook = builder.create_inline(NVSDK_NGX_Parameter_GetUI_orig, NVSDK_NGX_Parameter_GetUI);
		}

		spdlog::info("DLSS functions found & parameter hooks applied!");
		spdlog::info("Settings:");
		spdlog::info(" - ForceDLAA: {}", forceDLAA ? "true" : "false");
		spdlog::info(" - OverrideAutoExposure: {}", overrideAutoExposure == 0 ? "default" : (overrideAutoExposure > 0 ? "enable" : "disable"));
		spdlog::info(" - OverrideAppId: {}", overrideAppId ? "true" : "false");

		// disable NGX param export hooks since they aren't needed now
		NVSDK_NGX_D3D11_AllocateParameters_Hook.reset();
		NVSDK_NGX_D3D11_GetCapabilityParameters_Hook.reset();
		NVSDK_NGX_D3D11_GetParameters_Hook.reset();

		NVSDK_NGX_D3D12_AllocateParameters_Hook.reset();
		NVSDK_NGX_D3D12_GetCapabilityParameters_Hook.reset();
		NVSDK_NGX_D3D12_GetParameters_Hook.reset();

		NVSDK_NGX_VULKAN_AllocateParameters_Hook.reset();
		NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook.reset();
		NVSDK_NGX_VULKAN_GetParameters_Hook.reset();
	}
}

void hook(HMODULE ngx_module)
{
	auto* NVSDK_NGX_D3D11_Init_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_Init");
	auto* NVSDK_NGX_D3D11_Init_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_Init_Ext");
	auto* NVSDK_NGX_D3D11_Init_ProjectID_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_Init_ProjectID");
	auto* NVSDK_NGX_D3D11_AllocateParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_AllocateParameters");
	auto* NVSDK_NGX_D3D11_GetCapabilityParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_GetCapabilityParameters");
	auto* NVSDK_NGX_D3D11_GetParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_GetParameters");

	auto* NVSDK_NGX_D3D12_Init_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_Init");
	auto* NVSDK_NGX_D3D12_Init_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_Init_Ext");
	auto* NVSDK_NGX_D3D12_Init_ProjectID_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_Init_ProjectID");
	auto* NVSDK_NGX_D3D12_AllocateParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_AllocateParameters");
	auto* NVSDK_NGX_D3D12_GetCapabilityParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_GetCapabilityParameters");
	auto* NVSDK_NGX_D3D12_GetParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_GetParameters");

	auto* NVSDK_NGX_VULKAN_Init_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init");
	auto* NVSDK_NGX_VULKAN_Init_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_Ext");
	auto* NVSDK_NGX_VULKAN_Init_Ext2_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_Ext2");
	auto* NVSDK_NGX_VULKAN_Init_ProjectID_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_ProjectID");
	auto* NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_ProjectID_Ext");
	auto* NVSDK_NGX_VULKAN_AllocateParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_AllocateParameters");
	auto* NVSDK_NGX_VULKAN_GetCapabilityParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_GetCapabilityParameters");
	auto* NVSDK_NGX_VULKAN_GetParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_GetParameters");

	// Make sure we only try hooking if we found all the procs above...
	if (NVSDK_NGX_D3D11_Init_orig && NVSDK_NGX_D3D11_Init_Ext_orig && NVSDK_NGX_D3D11_Init_ProjectID_orig &&
		NVSDK_NGX_D3D11_AllocateParameters_orig && NVSDK_NGX_D3D11_GetCapabilityParameters_orig && NVSDK_NGX_D3D11_GetParameters_orig &&
		NVSDK_NGX_D3D12_Init_orig && NVSDK_NGX_D3D12_Init_Ext_orig && NVSDK_NGX_D3D12_Init_ProjectID_orig &&
		NVSDK_NGX_D3D12_AllocateParameters_orig && NVSDK_NGX_D3D12_GetCapabilityParameters_orig && NVSDK_NGX_D3D12_GetParameters_orig &&
		NVSDK_NGX_VULKAN_Init_orig && NVSDK_NGX_VULKAN_Init_Ext_orig && NVSDK_NGX_VULKAN_Init_ProjectID_orig &&
		NVSDK_NGX_VULKAN_AllocateParameters_orig && NVSDK_NGX_VULKAN_GetCapabilityParameters_orig && NVSDK_NGX_VULKAN_GetParameters_orig)
	{
		auto builder = SafetyHookFactory::acquire();

		NVSDK_NGX_D3D11_Init_Hook = builder.create_inline(NVSDK_NGX_D3D11_Init_orig, NVSDK_NGX_D3D11_Init);
		NVSDK_NGX_D3D11_Init_Ext_Hook = builder.create_inline(NVSDK_NGX_D3D11_Init_Ext_orig, NVSDK_NGX_D3D11_Init_Ext);
		NVSDK_NGX_D3D11_Init_ProjectID_Hook = builder.create_inline(NVSDK_NGX_D3D11_Init_ProjectID_orig, NVSDK_NGX_D3D11_Init_ProjectID);

		NVSDK_NGX_D3D11_AllocateParameters_Hook = builder.create_inline(NVSDK_NGX_D3D11_AllocateParameters_orig, NVSDK_NGX_D3D11_AllocateParameters);
		NVSDK_NGX_D3D11_GetCapabilityParameters_Hook = builder.create_inline(NVSDK_NGX_D3D11_GetCapabilityParameters_orig, NVSDK_NGX_D3D11_GetCapabilityParameters);
		NVSDK_NGX_D3D11_GetParameters_Hook = builder.create_inline(NVSDK_NGX_D3D11_GetParameters_orig, NVSDK_NGX_D3D11_GetParameters);

		NVSDK_NGX_D3D12_Init_Hook = builder.create_inline(NVSDK_NGX_D3D12_Init_orig, NVSDK_NGX_D3D12_Init);
		NVSDK_NGX_D3D12_Init_Ext_Hook = builder.create_inline(NVSDK_NGX_D3D12_Init_Ext_orig, NVSDK_NGX_D3D12_Init_Ext);
		NVSDK_NGX_D3D12_Init_ProjectID_Hook = builder.create_inline(NVSDK_NGX_D3D12_Init_ProjectID_orig, NVSDK_NGX_D3D12_Init_ProjectID);

		NVSDK_NGX_D3D12_AllocateParameters_Hook = builder.create_inline(NVSDK_NGX_D3D12_AllocateParameters_orig, NVSDK_NGX_D3D12_AllocateParameters);
		NVSDK_NGX_D3D12_GetCapabilityParameters_Hook = builder.create_inline(NVSDK_NGX_D3D12_GetCapabilityParameters_orig, NVSDK_NGX_D3D12_GetCapabilityParameters);
		NVSDK_NGX_D3D12_GetParameters_Hook = builder.create_inline(NVSDK_NGX_D3D12_GetParameters_orig, NVSDK_NGX_D3D12_GetParameters);

		NVSDK_NGX_VULKAN_Init_Hook = builder.create_inline(NVSDK_NGX_VULKAN_Init_orig, NVSDK_NGX_VULKAN_Init);
		NVSDK_NGX_VULKAN_Init_Ext_Hook = builder.create_inline(NVSDK_NGX_VULKAN_Init_Ext_orig, NVSDK_NGX_VULKAN_Init_Ext);
		NVSDK_NGX_VULKAN_Init_ProjectID_Hook = builder.create_inline(NVSDK_NGX_VULKAN_Init_ProjectID_orig, NVSDK_NGX_VULKAN_Init_ProjectID);

		// Only in later drivers
		if (NVSDK_NGX_VULKAN_Init_Ext2_orig)
			NVSDK_NGX_VULKAN_Init_Ext2_Hook = builder.create_inline(NVSDK_NGX_VULKAN_Init_Ext2_orig, NVSDK_NGX_VULKAN_Init_Ext2);
		if (NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig)
			NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook = builder.create_inline(NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig, NVSDK_NGX_VULKAN_Init_ProjectID_Ext);

		NVSDK_NGX_VULKAN_AllocateParameters_Hook = builder.create_inline(NVSDK_NGX_VULKAN_AllocateParameters_orig, NVSDK_NGX_VULKAN_AllocateParameters);
		NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook = builder.create_inline(NVSDK_NGX_VULKAN_GetCapabilityParameters_orig, NVSDK_NGX_VULKAN_GetCapabilityParameters);
		NVSDK_NGX_VULKAN_GetParameters_Hook = builder.create_inline(NVSDK_NGX_VULKAN_GetParameters_orig, NVSDK_NGX_VULKAN_GetParameters);

		spdlog::info("nvngx: applied export hooks, waiting for game to call them...");
	}
	else
	{
		spdlog::error("nvngx: failed to locate some functions, may require driver update!");
	}
}

void unhook(HMODULE ngx_module)
{
	spdlog::debug("nvngx: begin unhook");

	NVSDK_NGX_D3D11_Init_Hook.reset();
	NVSDK_NGX_D3D11_Init_Ext_Hook.reset();
	NVSDK_NGX_D3D11_Init_ProjectID_Hook.reset();
	NVSDK_NGX_D3D12_Init_Hook.reset();
	NVSDK_NGX_D3D12_Init_Ext_Hook.reset();
	NVSDK_NGX_D3D12_Init_ProjectID_Hook.reset();
	NVSDK_NGX_VULKAN_Init_Hook.reset();
	NVSDK_NGX_VULKAN_Init_Ext_Hook.reset();
	NVSDK_NGX_VULKAN_Init_Ext2_Hook.reset();
	NVSDK_NGX_VULKAN_Init_ProjectID_Hook.reset();
	NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook.reset();
	NVSDK_NGX_Parameter_SetI_Hook.reset();
	NVSDK_NGX_Parameter_SetUI_Hook.reset();
	NVSDK_NGX_Parameter_GetUI_Hook.reset();
	NVSDK_NGX_D3D12_AllocateParameters_Hook.reset();
	NVSDK_NGX_D3D12_GetCapabilityParameters_Hook.reset();
	NVSDK_NGX_D3D12_GetParameters_Hook.reset();
	NVSDK_NGX_D3D11_AllocateParameters_Hook.reset();
	NVSDK_NGX_D3D11_GetCapabilityParameters_Hook.reset();
	NVSDK_NGX_D3D11_GetParameters_Hook.reset();
	NVSDK_NGX_VULKAN_AllocateParameters_Hook.reset();
	NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook.reset();
	NVSDK_NGX_VULKAN_GetParameters_Hook.reset();

	spdlog::debug("nvngx: finished unhook");
}

SafetyHookInline dllmain;
BOOL APIENTRY hooked_dllmain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		hook(hModule);

	BOOL res = dllmain.stdcall<BOOL>(hModule, ul_reason_for_call, lpReserved);

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		unhook(hModule);
		dllmain.reset();
	}

	return res;
}

void init_from_proxy()
{
	// setup our HookFnOrig instances with the original func pointers
	NVSDK_NGX_D3D11_Init_Hook = NVSDK_NGX_D3D11_Init_Orig;
	NVSDK_NGX_D3D11_Init_Ext_Hook = NVSDK_NGX_D3D11_Init_Ext_Orig;
	NVSDK_NGX_D3D11_Init_ProjectID_Hook = NVSDK_NGX_D3D11_Init_ProjectID_Orig;
	NVSDK_NGX_D3D12_Init_Hook = NVSDK_NGX_D3D12_Init_Orig;
	NVSDK_NGX_D3D12_Init_Ext_Hook = NVSDK_NGX_D3D12_Init_Ext_Orig;
	NVSDK_NGX_D3D12_Init_ProjectID_Hook = NVSDK_NGX_D3D12_Init_ProjectID_Orig;
	NVSDK_NGX_VULKAN_Init_Hook = NVSDK_NGX_VULKAN_Init_Orig;
	NVSDK_NGX_VULKAN_Init_Ext_Hook = NVSDK_NGX_VULKAN_Init_Ext_Orig;
	NVSDK_NGX_VULKAN_Init_ProjectID_Hook = NVSDK_NGX_VULKAN_Init_ProjectID_Orig;
	NVSDK_NGX_D3D12_AllocateParameters_Hook = NVSDK_NGX_D3D12_AllocateParameters_Orig;
	NVSDK_NGX_D3D12_GetCapabilityParameters_Hook = NVSDK_NGX_D3D12_GetCapabilityParameters_Orig;
	NVSDK_NGX_D3D12_GetParameters_Hook = NVSDK_NGX_D3D12_GetParameters_Orig;
	NVSDK_NGX_D3D11_AllocateParameters_Hook = NVSDK_NGX_D3D11_AllocateParameters_Orig;
	NVSDK_NGX_D3D11_GetCapabilityParameters_Hook = NVSDK_NGX_D3D11_GetCapabilityParameters_Orig;
	NVSDK_NGX_D3D11_GetParameters_Hook = NVSDK_NGX_D3D11_GetParameters_Orig;
	NVSDK_NGX_VULKAN_AllocateParameters_Hook = NVSDK_NGX_VULKAN_AllocateParameters_Orig;
	NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook = NVSDK_NGX_VULKAN_GetCapabilityParameters_Orig;
	NVSDK_NGX_VULKAN_GetParameters_Hook = NVSDK_NGX_VULKAN_GetParameters_Orig;
}

// Installs DllMain hook onto NVNGX
void init(HMODULE ngx_module)
{
	if (proxy::is_wrapping_nvngx)
		return;

	// aren't wrapping nvngx, apply dllmain hook to the module
	auto builder = SafetyHookFactory::acquire();
	dllmain = builder.create_inline(utility::ModuleEntryPoint(ngx_module), hooked_dllmain);
}
};

namespace nvngx_dlss
{
// Allow force enabling/disabling the DLSS debug display via RegQueryValueExW hook
// (fallback method if we couldn't find the indicator value check pattern in the DLL)
LSTATUS(__stdcall* RegQueryValueExW_Orig)(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LSTATUS __stdcall RegQueryValueExW_Hook(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	LSTATUS ret = RegQueryValueExW_Orig(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	if (overrideDlssHud != 0 && !_wcsicmp(lpValueName, L"ShowDlssIndicator"))
		if (lpcbData && *lpcbData >= 4 && lpData)
		{
			DWORD* outData = (DWORD*)lpData;
			if (overrideDlssHud >= 1)
			{
				*outData = 0x400;
				return ERROR_SUCCESS; // always return success for DLSS to accept the result
			}
			else if (overrideDlssHud < 0)
				*outData = 0;
		}

	return ret;
}

SafetyHookInline DLSS_GetIndicatorValue_Hook;
uint32_t __fastcall DLSS_GetIndicatorValue(void* thisptr, uint32_t* OutValue)
{
	auto ret = DLSS_GetIndicatorValue_Hook.thiscall<uint32_t>(thisptr, OutValue);
	if (overrideDlssHud == 0)
		return ret;
	*OutValue = overrideDlssHud > 0 ? 0x400 : 0;
	return ret;
}

SafetyHookMid dlssIndicatorHudHook{};
bool hook(HMODULE ngx_module)
{
	// This pattern finds 2 matches in latest DLSS, but only 1 in older ones
	// The one we're trying to find seems to always be first match
	// TODO: scan for RegQueryValue call and grab the offset from that, use offset instead of wildcards below

	auto indicatorValueCheck = hook::pattern((void*)ngx_module, "8B 81 ? ? ? ? 89 02 33 C0 C3");
	if (watchIniUpdates && indicatorValueCheck.size())
	{
		spdlog::debug("nvngx_dlss: WatchIniUpdates enabled, applying hud hook via vftable hook...");

		auto indicatorValueCheck_addr = indicatorValueCheck.get(0).get<void>();
		{
			auto builder = SafetyHookFactory::acquire();
			DLSS_GetIndicatorValue_Hook = builder.create_inline(indicatorValueCheck_addr, DLSS_GetIndicatorValue);
		}

		// Unfortunately it's not enough to just hook the function, HUD render code seems to have an optimization where it checks funcptr and inlines code if it matches
		// So we also need to search for the address of the function, find vftable that holds it, and overwrite entry to point at our hook

		// Ugly way of converting our address to a pattern string...
		uint8_t* p = (uint8_t*)&indicatorValueCheck_addr;

		std::stringstream ss;
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)p[0];
		for (int i = 1; i < 8; ++i)
			ss << " " << std::setw(2) << std::setfill('0') << (int)p[i];

		auto pattern = ss.str();

		auto indicatorValueCheckMemberVf = hook::pattern((void*)ngx_module, pattern);
		for (int i = 0; i < indicatorValueCheckMemberVf.size(); i++)
		{
			auto vfAddr = indicatorValueCheckMemberVf.get(i).get<uintptr_t>(0);
			UnprotectMemory unprotect{ (uintptr_t)vfAddr, sizeof(uintptr_t) };
			*vfAddr = (uintptr_t)&DLSS_GetIndicatorValue;
		}

		spdlog::debug("nvngx_dlss: applied hud hook via vftable hook");
	}
	else
	{
		// Failed to locate indicator value check inside DLSS
		// Fallback to RegQueryValueExW hook so we can override the ShowDlssIndicator value
		HMODULE advapi = GetModuleHandleA("advapi32.dll");
		RegQueryValueExW_Orig = advapi ? (decltype(RegQueryValueExW_Orig))GetProcAddress(advapi, "RegQueryValueExW") : nullptr;
		if (!RegQueryValueExW_Orig || !utility::HookIAT(ngx_module, "advapi32.dll", RegQueryValueExW_Orig, RegQueryValueExW_Hook))
			spdlog::warn("nvngx_dlss: failed to hook DLSS HUD functions, OverrideDlssHud might not be available.");
		else
			spdlog::debug("nvngx_dlss: applied hud hook via registry");
	}

	return true;
}

void unhook(HMODULE ngx_module)
{
	dlssIndicatorHudHook.reset();

	spdlog::debug("nvngx_dlss: finished unhook");
}

SafetyHookInline dllmain;
BOOL APIENTRY hooked_dllmain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		hook(hModule);

	BOOL res = dllmain.stdcall<BOOL>(hModule, ul_reason_for_call, lpReserved);

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		unhook(hModule);
		dllmain.reset();
	}

	return res;
}

// Installs DllMain hook onto nvngx_dlss
void init(HMODULE ngx_module)
{
	auto builder = SafetyHookFactory::acquire();
	dllmain = builder.create_inline(utility::ModuleEntryPoint(ngx_module), hooked_dllmain);
}
};

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

	if (!_wcsicmp(filenameStr.c_str(), DlssFileName) && !overrideDlssDll.empty())
	{
		std::call_once(dlssOverrideMessage, []() { 
			spdlog::info("Game is loading DLSS, overriding with DLL path: {}", overrideDlssDll.string());
		});

		libPath = overrideDlssDll;
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
		if (!overrideDlssDll.empty())
			dlssName = overrideDlssDll.filename().wstring();

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

bool INIReadSettings()
{
	using namespace utility;

	std::wstring iniPathStr = IniPath.wstring();

	// Read INI via FILE* since INIReader doesn't support wstring
	FILE* iniFile;
	if (_wfopen_s(&iniFile, iniPathStr.c_str(), L"r") != 0 || !iniFile)
		return false;

	inih::INIReader ini(iniFile);
	fclose(iniFile);

	watchIniUpdates = ini.Get<bool>("DLSS", "WatchIniUpdates", std::move(watchIniUpdates));
	forceDLAA = ini.Get<bool>("DLSS", "ForceDLAA", std::move(forceDLAA));
	overrideAutoExposure = ini.Get<int>("DLSS", "OverrideAutoExposure", std::move(overrideAutoExposure));
	overrideDlssHud = ini.Get<int>("DLSS", "OverrideDlssHud", std::move(overrideDlssHud));
	disableDevWatermark = ini.Get<bool>("DLSS", "DisableDevWatermark", std::move(disableDevWatermark));
	overrideAppId = ini.Get<bool>("DLSS", "OverrideAppId", std::move(overrideAppId));
	overrideDlssDll = ini.Get<std::filesystem::path>("DLSS", "OverrideDlssDll", "");
	overrideQualityLevels = ini.Get<bool>("DLSSQualityLevels", "Enable", std::move(overrideQualityLevels));
	if (overrideQualityLevels)
	{
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxPerf] = ini.Get<float>("DLSSQualityLevels", "Performance", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxPerf]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_Balanced] = ini.Get<float>("DLSSQualityLevels", "Balanced", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_Balanced]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxQuality] = ini.Get<float>("DLSSQualityLevels", "Quality", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_MaxQuality]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraPerformance] = ini.Get<float>("DLSSQualityLevels", "UltraPerformance", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraPerformance]));
		qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraQuality] = ini.Get<float>("DLSSQualityLevels", "UltraQuality", std::move(qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraQuality]));
	}
	presetDLAA = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "DLAA");
	presetQuality = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "Quality");
	presetBalanced = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "Balanced");
	presetPerformance = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "Performance");
	presetUltraPerformance = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "UltraPerformance");

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

	spdlog::info("DLSSTweaks v0.200.2, by emoose: {} wrapper loaded", DllPath.filename().string());
	spdlog::info("Game path: {}", ExePath.string());
	spdlog::info("DLL path: {}", DllPath.string());
	spdlog::info("Config path: {}", IniPath.string());

	spdlog::info("---");

	INIReadSettings();

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

	// Hook LoadLibrary so we can override DLSS path if desired
	if (!overrideDlssDll.empty())
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

	// Init finished, allow any DLSS calls to continue
	{
		std::lock_guard lock(initThreadFinishedMutex);
		initThreadFinished = true;
		initThreadFinishedVar.notify_all();
	}

	if (!watchIniUpdates)
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
							if (INIReadSettings())
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
