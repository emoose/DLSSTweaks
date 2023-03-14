#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>

#include <spdlog/spdlog.h>

#include "DLSSTweaks.hpp"
#include "Proxy.hpp"

const char* projectIdOverride = "24480451-f00d-face-1304-0308dabad187";
const unsigned long long appIdOverride = 0x24480451;

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

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D11_Init_Hook.unsafe_call<uint64_t>(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
HookOrigFn NVSDK_NGX_D3D11_Init_Ext_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D11_Init_Ext_Hook.unsafe_call<uint64_t>(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
HookOrigFn NVSDK_NGX_D3D11_Init_ProjectID_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, enum NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const struct NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, enum NVSDK_NGX_Version InSDKVersion)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InProjectId = projectIdOverride;

	return NVSDK_NGX_D3D11_Init_ProjectID_Hook.unsafe_call<uint64_t>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

HookOrigFn NVSDK_NGX_D3D12_Init_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const void* InFeatureInfo, void* InSDKVersion)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D12_Init_Hook.unsafe_call<uint64_t>(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
HookOrigFn NVSDK_NGX_D3D12_Init_Ext_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D12_Init_Ext_Hook.unsafe_call<uint64_t>(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
HookOrigFn NVSDK_NGX_D3D12_Init_ProjectID_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_D3D12_Init_ProjectID(const char* InProjectId, enum NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const struct NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, enum NVSDK_NGX_Version InSDKVersion)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InProjectId = projectIdOverride;

	return NVSDK_NGX_D3D12_Init_ProjectID_Hook.unsafe_call<uint64_t>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

HookOrigFn NVSDK_NGX_VULKAN_Init_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_VULKAN_Init_Hook.unsafe_call<uint64_t>(InApplicationId, a2, a3, a4, a5, a6);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_Ext_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Ext(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_VULKAN_Init_Ext_Hook.unsafe_call<uint64_t>(InApplicationId, a2, a3, a4, a5, a6, a7);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_ProjectID_Hook;
PLUGIN_API uint64_t __cdecl NVSDK_NGX_VULKAN_Init_ProjectID(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InProjectId = projectIdOverride;

	return NVSDK_NGX_VULKAN_Init_ProjectID_Hook.unsafe_call<uint64_t>(InProjectId, a2, a3, a4, a5, a6, a7, a8, a9);
}

// VULKAN_Init_Ext2 / VULKAN_Init_ProjectID_Ext are _nvngx.dll only, not included in nvngx.dll
HookOrigFn NVSDK_NGX_VULKAN_Init_Ext2_Hook;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Ext2(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9)
{
	if (settings.overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_VULKAN_Init_Ext2_Hook.unsafe_call<uint64_t>(InApplicationId, a2, a3, a4, a5, a6, a7, a8, a9);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_ProjectID_Ext(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9, void* a10, void* a11)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook.unsafe_call<uint64_t>(InProjectId, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

SafetyHookInline NVSDK_NGX_Parameter_SetI_Hook;
void __cdecl NVSDK_NGX_Parameter_SetI(void* InParameter, const char* InName, int InValue)
{
	if (settings.overrideAutoExposure != 0 && !_stricmp(InName, NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags))
	{
		if (settings.overrideAutoExposure >= 1) // force auto-exposure
			InValue |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
		else if (settings.overrideAutoExposure < 0) // force disable auto-exposure
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

	if (settings.presetDLAA != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, settings.presetDLAA);
	if (settings.presetQuality != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, settings.presetQuality);
	if (settings.presetBalanced != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, settings.presetBalanced);
	if (settings.presetPerformance != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, settings.presetPerformance);
	if (settings.presetUltraPerformance != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, settings.presetUltraPerformance);

	NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Disable_Watermark, settings.disableDevWatermark ? 1 : 0);
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
	bool overrideWidth = settings.forceDLAA && isOutWidth;
	bool overrideHeight = settings.forceDLAA && isOutHeight;
	if (overrideWidth || overrideHeight)
	{
		if (overrideWidth && *OutValue != 0)
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Width, OutValue);
		if (overrideHeight && *OutValue != 0)
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Height, OutValue);
	}

	// Override with DLSSQualityLevels value if user set it
	if (settings.overrideQualityLevels && qualityLevelRatios.count(prevQualityValue))
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
		spdlog::info(" - ForceDLAA: {}", settings.forceDLAA ? "true" : "false");
		spdlog::info(" - OverrideAutoExposure: {}", settings.overrideAutoExposure == 0 ? "default" : (settings.overrideAutoExposure > 0 ? "enable" : "disable"));
		spdlog::info(" - OverrideAppId: {}", settings.overrideAppId ? "true" : "false");

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
