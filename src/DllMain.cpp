#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdio>
#include <cstdint>
#include <string>
#include <psapi.h>
#include <filesystem>
#include <fstream>
#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <SafetyHook.hpp>
#include <Patterns.h>
#include <ini.h>

#include "Utility.hpp"
#include "Proxy.hpp"

const wchar_t* LogFileName = L"dlsstweaks.log";
const wchar_t* IniFileName = L"dlsstweaks.ini";

std::filesystem::path LogPath;
std::filesystem::path IniPath;

bool watchIniUpdates = false;
bool forceDLAA = false;
int overrideAutoExposure = 0;
int overrideDlssHud = 0;
bool disableDevWatermark = false;
bool overrideAppId = false;
bool overrideQualityLevels = false;
unsigned int presetDLAA = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
unsigned int presetQuality = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
unsigned int presetBalanced = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
unsigned int presetPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
unsigned int presetUltraPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;

float qualityLevelRatios[] = {
	0.5f, // NVSDK_NGX_PerfQuality_Value_MaxPerf
	0.58f, // NVSDK_NGX_PerfQuality_Value_Balanced
	0.66666667f, // NVSDK_NGX_PerfQuality_Value_MaxQuality
	0.33333334f, // NVSDK_NGX_PerfQuality_Value_UltraPerformance
	0.77f, // UNSURE: NVSDK_NGX_PerfQuality_Value_UltraQuality
};

const char* projectIdOverride = "24480451-f00d-face-1304-0308dabad187";
const unsigned long long appIdOverride = 0x24480451;

// TODO: InFeatureInfo might also hold project ID related fields, maybe should change those too...
SafetyHookInline NVSDK_NGX_D3D11_Init;
uint64_t __cdecl NVSDK_NGX_D3D11_Init_Hook(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const void* InFeatureInfo, void* InSDKVersion)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_D3D11_Init->call<uint64_t>(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
SafetyHookInline NVSDK_NGX_D3D11_Init_Ext;
uint64_t __cdecl NVSDK_NGX_D3D11_Init_Ext_Hook(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_D3D11_Init_Ext->call<uint64_t>(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
SafetyHookInline NVSDK_NGX_D3D11_Init_ProjectID;
uint64_t __cdecl NVSDK_NGX_D3D11_Init_ProjectID_Hook(const char* InProjectId, enum NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const struct NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, enum NVSDK_NGX_Version InSDKVersion)
{
	if (overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_D3D11_Init_ProjectID->call<uint64_t>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

SafetyHookInline NVSDK_NGX_D3D12_Init;
uint64_t __cdecl NVSDK_NGX_D3D12_Init_Hook(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const void* InFeatureInfo, void* InSDKVersion)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_D3D12_Init->call<uint64_t>(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
SafetyHookInline NVSDK_NGX_D3D12_Init_Ext;
uint64_t __cdecl NVSDK_NGX_D3D12_Init_Ext_Hook(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_D3D12_Init_Ext->call<uint64_t>(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
SafetyHookInline NVSDK_NGX_D3D12_Init_ProjectID;
uint64_t __cdecl NVSDK_NGX_D3D12_Init_ProjectID_Hook(const char* InProjectId, enum NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const struct NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, enum NVSDK_NGX_Version InSDKVersion)
{
	if (overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_D3D12_Init_ProjectID->call<uint64_t>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

SafetyHookInline NVSDK_NGX_VULKAN_Init;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Hook(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_VULKAN_Init->call<uint64_t>(InApplicationId, a2, a3, a4, a5, a6);
}
SafetyHookInline NVSDK_NGX_VULKAN_Init_Ext;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Ext_Hook(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_VULKAN_Init_Ext->call<uint64_t>(InApplicationId, a2, a3, a4, a5, a6, a7);
}
SafetyHookInline NVSDK_NGX_VULKAN_Init_Ext2;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Ext2_Hook(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_VULKAN_Init_Ext2->call<uint64_t>(InApplicationId, a2, a3, a4, a5, a6, a7, a8, a9);
}
SafetyHookInline NVSDK_NGX_VULKAN_Init_ProjectID;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_ProjectID_Hook(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9)
{
	if (overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_VULKAN_Init_ProjectID->call<uint64_t>(InProjectId, a2, a3, a4, a5, a6, a7, a8, a9);
}
SafetyHookInline NVSDK_NGX_VULKAN_Init_ProjectID_Ext;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9, void* a10, void* a11)
{
	if (overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_VULKAN_Init_ProjectID_Ext->call<uint64_t>(InProjectId, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

enum NVSDK_NGX_DLSS_Feature_Flags
{
	NVSDK_NGX_DLSS_Feature_Flags_None = 0,
	NVSDK_NGX_DLSS_Feature_Flags_IsHDR = 1 << 0,
	NVSDK_NGX_DLSS_Feature_Flags_MVLowRes = 1 << 1,
	NVSDK_NGX_DLSS_Feature_Flags_MVJittered = 1 << 2,
	NVSDK_NGX_DLSS_Feature_Flags_DepthInverted = 1 << 3,
	NVSDK_NGX_DLSS_Feature_Flags_Reserved_0 = 1 << 4,
	NVSDK_NGX_DLSS_Feature_Flags_DoSharpening = 1 << 5,
	NVSDK_NGX_DLSS_Feature_Flags_AutoExposure = 1 << 6,
};

enum NVSDK_NGX_PerfQuality_Value
{
	NVSDK_NGX_PerfQuality_Value_MaxPerf,
	NVSDK_NGX_PerfQuality_Value_Balanced,
	NVSDK_NGX_PerfQuality_Value_MaxQuality,
	NVSDK_NGX_PerfQuality_Value_UltraPerformance,
	NVSDK_NGX_PerfQuality_Value_UltraQuality,
};

NVSDK_NGX_PerfQuality_Value prevQualityValue;

SafetyHookInline NVSDK_NGX_Parameter_SetI;
void __cdecl NVSDK_NGX_Parameter_SetI_Hook(void* InParameter, const char* InName, int InValue)
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
		prevQualityValue = NVSDK_NGX_PerfQuality_Value(InValue);

	NVSDK_NGX_Parameter_SetI->call(InParameter, InName, InValue);
}

SafetyHookInline NVSDK_NGX_Parameter_SetUI;
void __cdecl NVSDK_NGX_Parameter_SetUI_Hook(void* InParameter, const char* InName, unsigned int InValue)
{
	NVSDK_NGX_Parameter_SetUI->call(InParameter, InName, InValue);

	if (presetDLAA != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI->call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, presetDLAA);
	if (presetQuality != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI->call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, presetQuality);
	if (presetBalanced != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI->call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, presetBalanced);
	if (presetPerformance != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI->call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, presetPerformance);
	if (presetUltraPerformance != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI->call(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, presetUltraPerformance);

	NVSDK_NGX_Parameter_SetUI->call(InParameter, NVSDK_NGX_Parameter_Disable_Watermark, disableDevWatermark ? 1 : 0);
}

SafetyHookInline NVSDK_NGX_Parameter_GetUI;
uint64_t __cdecl NVSDK_NGX_Parameter_GetUI_Hook(void* InParameter, const char* InName, unsigned int* OutValue)
{
	uint64_t ret = NVSDK_NGX_Parameter_GetUI->call<uint64_t>(InParameter, InName, OutValue);
	if (ret != 1)
		return ret;

	bool isOutWidth = !_stricmp(InName, NVSDK_NGX_Parameter_OutWidth);
	bool isOutHeight = !_stricmp(InName, NVSDK_NGX_Parameter_OutHeight);
	if (overrideQualityLevels)
	{
		if (isOutWidth)
		{
			unsigned int targetWidth = 0;
			NVSDK_NGX_Parameter_GetUI->call(InParameter, NVSDK_NGX_Parameter_Width, &targetWidth); // fetch full screen width
			targetWidth = unsigned int(roundf(float(targetWidth) * qualityLevelRatios[int(prevQualityValue)])); // calculate new width from custom ratio
			*OutValue = targetWidth;
		}
		if (isOutHeight)
		{
			unsigned int targetHeight = 0;
			NVSDK_NGX_Parameter_GetUI->call(InParameter, NVSDK_NGX_Parameter_Height, &targetHeight); // fetch full screen height
			targetHeight = unsigned int(roundf(float(targetHeight) * qualityLevelRatios[int(prevQualityValue)])); // calculate new height from custom ratio
			*OutValue = targetHeight;
		}
	}

	// DLAA force by overwriting OutWidth/OutHeight with the full res
	bool overrideWidth = forceDLAA && isOutWidth;
	bool overrideHeight = forceDLAA && isOutHeight;
	if (overrideWidth || overrideHeight)
	{
		if (overrideWidth && *OutValue != 0)
			NVSDK_NGX_Parameter_GetUI->call(InParameter, NVSDK_NGX_Parameter_Width, OutValue);
		if (overrideHeight && *OutValue != 0)
			NVSDK_NGX_Parameter_GetUI->call(InParameter, NVSDK_NGX_Parameter_Height, OutValue);
	}

	return ret;
}

// Matches the order of NVSDK_NGX_Parameter vftable inside _nvngx.dll (which should never change unless they want to break compatibility)
struct NVSDK_NGX_Parameter_vftable
{
	void* SetVoidPointer;
	void* SetD3d12Resource;
	void* SetD3d11Resource;
	void* SetI;
	void* SetUI;
	void* SetD;
	void* SetF;
	void* SetULL;
	void* GetVoidPointer;
	void* GetD3d12Resource;
	void* GetD3d11Resource;
	void* GetI;
	void* GetUI;
	void* GetD;
	void* GetF;
	void* GetULL;
	void* Reset;
};
struct NVSDK_NGX_Parameter
{
	NVSDK_NGX_Parameter_vftable* _vftable;
};
bool DLSS_HookParamFunctions(NVSDK_NGX_Parameter* params);

SafetyHookInline NVSDK_NGX_D3D12_AllocateParameters;
uint64_t __cdecl NVSDK_NGX_D3D12_AllocateParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D12_AllocateParameters->call<uint64_t>(OutParameters);
	if (*OutParameters)
		DLSS_HookParamFunctions(*OutParameters);
	return ret;
}
SafetyHookInline NVSDK_NGX_D3D12_GetCapabilityParameters;
uint64_t __cdecl NVSDK_NGX_D3D12_GetCapabilityParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D12_GetCapabilityParameters->call<uint64_t>(OutParameters);
	if (*OutParameters)
		DLSS_HookParamFunctions(*OutParameters);
	return ret;
}
SafetyHookInline NVSDK_NGX_D3D12_GetParameters;
uint64_t __cdecl NVSDK_NGX_D3D12_GetParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D12_GetParameters->call<uint64_t>(OutParameters);
	if (*OutParameters)
		DLSS_HookParamFunctions(*OutParameters);
	return ret;
}

SafetyHookInline NVSDK_NGX_D3D11_AllocateParameters;
uint64_t __cdecl NVSDK_NGX_D3D11_AllocateParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D11_AllocateParameters->call<uint64_t>(OutParameters);
	if (*OutParameters)
		DLSS_HookParamFunctions(*OutParameters);
	return ret;
}
SafetyHookInline NVSDK_NGX_D3D11_GetCapabilityParameters;
uint64_t __cdecl NVSDK_NGX_D3D11_GetCapabilityParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D11_GetCapabilityParameters->call<uint64_t>(OutParameters);
	if (*OutParameters)
		DLSS_HookParamFunctions(*OutParameters);
	return ret;
}
SafetyHookInline NVSDK_NGX_D3D11_GetParameters;
uint64_t __cdecl NVSDK_NGX_D3D11_GetParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D11_GetParameters->call<uint64_t>(OutParameters);
	if (*OutParameters)
		DLSS_HookParamFunctions(*OutParameters);
	return ret;
}

SafetyHookInline NVSDK_NGX_VULKAN_AllocateParameters;
uint64_t __cdecl NVSDK_NGX_VULKAN_AllocateParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_VULKAN_AllocateParameters->call<uint64_t>(OutParameters);
	if (*OutParameters)
		DLSS_HookParamFunctions(*OutParameters);
	return ret;
}
SafetyHookInline NVSDK_NGX_VULKAN_GetCapabilityParameters;
uint64_t __cdecl NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_VULKAN_GetCapabilityParameters->call<uint64_t>(OutParameters);
	if (*OutParameters)
		DLSS_HookParamFunctions(*OutParameters);
	return ret;
}
SafetyHookInline NVSDK_NGX_VULKAN_GetParameters;
uint64_t __cdecl NVSDK_NGX_VULKAN_GetParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_VULKAN_GetParameters->call<uint64_t>(OutParameters);
	if (*OutParameters)
		DLSS_HookParamFunctions(*OutParameters);
	return ret;
}

std::mutex paramHookMutex;
bool isParamFuncsHooked = false;
bool DLSS_HookParamFunctions(NVSDK_NGX_Parameter* params)
{
	std::lock_guard<std::mutex> lock(paramHookMutex);

	if (isParamFuncsHooked)
		return true;

	auto* NVSDK_NGX_Parameter_SetI_orig = params ? (params->_vftable ? params->_vftable->SetI : nullptr) : nullptr;
	auto* NVSDK_NGX_Parameter_SetUI_orig = params ? (params->_vftable ? params->_vftable->SetUI : nullptr) : nullptr;
	auto* NVSDK_NGX_Parameter_GetUI_orig = params ? (params->_vftable ? params->_vftable->GetUI : nullptr) : nullptr;

	if (NVSDK_NGX_Parameter_SetI_orig && NVSDK_NGX_Parameter_SetUI_orig && NVSDK_NGX_Parameter_GetUI_orig)
	{
		{
			auto builder = SafetyHookFactory::acquire();
			NVSDK_NGX_Parameter_SetI = builder.create_inline(NVSDK_NGX_Parameter_SetI_orig, NVSDK_NGX_Parameter_SetI_Hook);
			NVSDK_NGX_Parameter_SetUI = builder.create_inline(NVSDK_NGX_Parameter_SetUI_orig, NVSDK_NGX_Parameter_SetUI_Hook);
			NVSDK_NGX_Parameter_GetUI = builder.create_inline(NVSDK_NGX_Parameter_GetUI_orig, NVSDK_NGX_Parameter_GetUI_Hook);
		}

		spdlog::info("DLSS functions found & parameter hooks applied!");
		spdlog::info("Settings:\n - ForceDLAA: {}\n - OverrideAutoExposure: {}\n - OverrideAppId: {}",
			forceDLAA ? "true" : "false",
			overrideAutoExposure == 0 ? "default" : (overrideAutoExposure > 0 ? "enable" : "disable"),
			overrideAppId ? "true" : "false");

		// disable NGX param export hooks since they aren't needed now

		if (NVSDK_NGX_D3D11_AllocateParameters)
			NVSDK_NGX_D3D11_AllocateParameters.reset();
		if (NVSDK_NGX_D3D11_GetCapabilityParameters)
			NVSDK_NGX_D3D11_GetCapabilityParameters.reset();
		if (NVSDK_NGX_D3D11_GetParameters)
			NVSDK_NGX_D3D11_GetParameters.reset();

		if (NVSDK_NGX_D3D12_AllocateParameters)
			NVSDK_NGX_D3D12_AllocateParameters.reset();
		if (NVSDK_NGX_D3D12_GetCapabilityParameters)
			NVSDK_NGX_D3D12_GetCapabilityParameters.reset();
		if (NVSDK_NGX_D3D12_GetParameters)
			NVSDK_NGX_D3D12_GetParameters.reset();

		if (NVSDK_NGX_VULKAN_AllocateParameters)
			NVSDK_NGX_VULKAN_AllocateParameters.reset();
		if (NVSDK_NGX_VULKAN_GetCapabilityParameters)
			NVSDK_NGX_VULKAN_GetCapabilityParameters.reset();
		if (NVSDK_NGX_VULKAN_GetParameters)
			NVSDK_NGX_VULKAN_GetParameters.reset();

		isParamFuncsHooked = true;
	}

	return isParamFuncsHooked;
}

std::mutex hookMutex;
bool isNgxHookAttempted = false;
bool isNgxDlssHookAttemped = false;
bool DLSS_HookNGX()
{
	std::lock_guard<std::mutex> lock(hookMutex);

	if (isNgxHookAttempted)
		return isNgxHookAttempted;

	HMODULE ngx_module = GetModuleHandleA("_nvngx.dll");
	if (!ngx_module)
		return isNgxHookAttempted;

	isNgxHookAttempted = true;

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
		NVSDK_NGX_VULKAN_Init_orig && NVSDK_NGX_VULKAN_Init_Ext_orig && NVSDK_NGX_VULKAN_Init_Ext2_orig && NVSDK_NGX_VULKAN_Init_ProjectID_orig &&
		NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig &&
		NVSDK_NGX_VULKAN_AllocateParameters_orig && NVSDK_NGX_VULKAN_GetCapabilityParameters_orig && NVSDK_NGX_VULKAN_GetParameters_orig)
	{
		auto builder = SafetyHookFactory::acquire();

		NVSDK_NGX_D3D11_Init = builder.create_inline(NVSDK_NGX_D3D11_Init_orig, NVSDK_NGX_D3D11_Init_Hook);
		NVSDK_NGX_D3D11_Init_Ext = builder.create_inline(NVSDK_NGX_D3D11_Init_Ext_orig, NVSDK_NGX_D3D11_Init_Ext_Hook);
		NVSDK_NGX_D3D11_Init_ProjectID = builder.create_inline(NVSDK_NGX_D3D11_Init_ProjectID_orig, NVSDK_NGX_D3D11_Init_ProjectID_Hook);

		NVSDK_NGX_D3D11_AllocateParameters = builder.create_inline(NVSDK_NGX_D3D11_AllocateParameters_orig, NVSDK_NGX_D3D11_AllocateParameters_Hook);
		NVSDK_NGX_D3D11_GetCapabilityParameters = builder.create_inline(NVSDK_NGX_D3D11_GetCapabilityParameters_orig, NVSDK_NGX_D3D11_GetCapabilityParameters_Hook);
		NVSDK_NGX_D3D11_GetParameters = builder.create_inline(NVSDK_NGX_D3D11_GetParameters_orig, NVSDK_NGX_D3D11_GetParameters_Hook);

		NVSDK_NGX_D3D12_Init = builder.create_inline(NVSDK_NGX_D3D12_Init_orig, NVSDK_NGX_D3D12_Init_Hook);
		NVSDK_NGX_D3D12_Init_Ext = builder.create_inline(NVSDK_NGX_D3D12_Init_Ext_orig, NVSDK_NGX_D3D12_Init_Ext_Hook);
		NVSDK_NGX_D3D12_Init_ProjectID = builder.create_inline(NVSDK_NGX_D3D12_Init_ProjectID_orig, NVSDK_NGX_D3D12_Init_ProjectID_Hook);

		NVSDK_NGX_D3D12_AllocateParameters = builder.create_inline(NVSDK_NGX_D3D12_AllocateParameters_orig, NVSDK_NGX_D3D12_AllocateParameters_Hook);
		NVSDK_NGX_D3D12_GetCapabilityParameters = builder.create_inline(NVSDK_NGX_D3D12_GetCapabilityParameters_orig, NVSDK_NGX_D3D12_GetCapabilityParameters_Hook);
		NVSDK_NGX_D3D12_GetParameters = builder.create_inline(NVSDK_NGX_D3D12_GetParameters_orig, NVSDK_NGX_D3D12_GetParameters_Hook);

		NVSDK_NGX_VULKAN_Init = builder.create_inline(NVSDK_NGX_VULKAN_Init_orig, NVSDK_NGX_VULKAN_Init_Hook);
		NVSDK_NGX_VULKAN_Init_Ext = builder.create_inline(NVSDK_NGX_VULKAN_Init_Ext_orig, NVSDK_NGX_VULKAN_Init_Ext_Hook);
		NVSDK_NGX_VULKAN_Init_Ext2 = builder.create_inline(NVSDK_NGX_VULKAN_Init_Ext2_orig, NVSDK_NGX_VULKAN_Init_Ext2_Hook);
		NVSDK_NGX_VULKAN_Init_ProjectID = builder.create_inline(NVSDK_NGX_VULKAN_Init_ProjectID_orig, NVSDK_NGX_VULKAN_Init_ProjectID_Hook);
		NVSDK_NGX_VULKAN_Init_ProjectID_Ext = builder.create_inline(NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig, NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook);

		NVSDK_NGX_VULKAN_AllocateParameters = builder.create_inline(NVSDK_NGX_VULKAN_AllocateParameters_orig, NVSDK_NGX_VULKAN_AllocateParameters_Hook);
		NVSDK_NGX_VULKAN_GetCapabilityParameters = builder.create_inline(NVSDK_NGX_VULKAN_GetCapabilityParameters_orig, NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook);
		NVSDK_NGX_VULKAN_GetParameters = builder.create_inline(NVSDK_NGX_VULKAN_GetParameters_orig, NVSDK_NGX_VULKAN_GetParameters_Hook);

		spdlog::info("Applied _nvngx.dll DLL export hooks, waiting for game to call them...");
	}
	else
	{
		spdlog::error("Failed to locate all _nvngx.dll functions, may have been changed by driver update...");
	}

	return isNgxHookAttempted;
}

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

SafetyHookMid dlssIndicatorHudHook{};

bool DLSS_HookNGXDLSS()
{
	std::lock_guard<std::mutex> lock(hookMutex);

	if (isNgxDlssHookAttemped)
		return isNgxDlssHookAttemped;

	HMODULE ngx_module = GetModuleHandleA("nvngx_dlss.dll");
	if (!ngx_module)
		return isNgxDlssHookAttemped;

	isNgxDlssHookAttemped = true;

	// TODO: older DLSS versions use a different offset than 0x110, preventing this pattern from working
	auto indicatorValueCheck = hook::pattern((void*)ngx_module, "75 ? 8B 83 10 01 00 00 89");
	if (watchIniUpdates && indicatorValueCheck.size())
	{
		auto builder = SafetyHookFactory::acquire();

		dlssIndicatorHudHook = builder.create_mid(indicatorValueCheck.count(1).get_first(2), [](safetyhook::Context& ctx)
			{
				if (overrideDlssHud == 0)
					return;
				*(uint32_t*)(ctx.rbx + 0x110) = overrideDlssHud > 0 ? 0x400 : 0;
			});
	}
	else
	{
		// Failed to locate indicator value check inside DLSS
		// Fallback to RegQueryValueExW hook so we can override the ShowDlssIndicator value
		HMODULE advapi = GetModuleHandleA("advapi32.dll");
		RegQueryValueExW_Orig = advapi ? (decltype(RegQueryValueExW_Orig))GetProcAddress(advapi, "RegQueryValueExW") : nullptr;
		if (RegQueryValueExW_Orig)
			utility::HookIAT(ngx_module, "advapi32.dll", RegQueryValueExW_Orig, RegQueryValueExW_Hook);
	}

	return isNgxDlssHookAttemped;
}

// Hook LoadLibraryExW so we can scan it for the DLSS funcs immediately after the module is loaded in
SafetyHookInline LoadLibraryExW_Orig;
HMODULE __stdcall LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	HMODULE ret = LoadLibraryExW_Orig->stdcall<HMODULE>(lpLibFileName, hFile, dwFlags);

	DLSS_HookNGX();
	DLSS_HookNGXDLSS();

	if (isNgxDlssHookAttemped && isNgxHookAttempted)
	{
		// we've looked at both nvngx & nvngx_dlss, no need for the LoadLibraryExW hook anymore
		std::lock_guard<std::mutex> lock(hookMutex);
		if (LoadLibraryExW_Orig)
			LoadLibraryExW_Orig.reset();
	}

	return ret;
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
	overrideQualityLevels = ini.Get<bool>("DLSSQualityLevels", "Enable", std::move(overrideQualityLevels));
	if (overrideQualityLevels)
	{
		qualityLevelRatios[0] = ini.Get<float>("DLSSQualityLevels", "Performance", std::move(qualityLevelRatios[0])); // NVSDK_NGX_PerfQuality_Value_MaxPerf
		qualityLevelRatios[1] = ini.Get<float>("DLSSQualityLevels", "Balanced", std::move(qualityLevelRatios[1])); // NVSDK_NGX_PerfQuality_Value_Balanced
		qualityLevelRatios[2] = ini.Get<float>("DLSSQualityLevels", "Quality", std::move(qualityLevelRatios[2])); // NVSDK_NGX_PerfQuality_Value_MaxQuality
		qualityLevelRatios[3] = ini.Get<float>("DLSSQualityLevels", "UltraPerformance", std::move(qualityLevelRatios[3])); // NVSDK_NGX_PerfQuality_Value_UltraPerformance
		qualityLevelRatios[4] = ini.Get<float>("DLSSQualityLevels", "UltraQuality", std::move(qualityLevelRatios[4])); // NVSDK_NGX_PerfQuality_Value_UltraQuality
	}
	presetDLAA = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "DLAA");
	presetQuality = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "Quality");
	presetBalanced = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "Balanced");
	presetPerformance = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "Performance");
	presetUltraPerformance = DLSS_ReadPresetFromIni(ini, "DLSSPresets", "UltraPerformance");

	return true;
}

unsigned int __stdcall InitThread(void* param)
{
	WCHAR exePath[4096];
	GetModuleFileNameW(GetModuleHandleA(0), exePath, 4096);

	LogPath = std::filesystem::path(exePath).parent_path() / LogFileName;
	IniPath = std::filesystem::path(exePath).parent_path() / IniFileName;
	INIReadSettings();

	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
	sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(LogPath.string(), true));

	auto combined_logger = std::make_shared<spdlog::logger>("", begin(sinks), end(sinks));
	combined_logger->set_level(spdlog::level::trace);
	spdlog::set_default_logger(combined_logger);

	spdlog::info("DLSSTweaks v0.123.10, by emoose: DLL wrapper loaded, watching for DLSS library load...");	

	auto* kernel32 = GetModuleHandleA("kernel32.dll");
	auto* LoadLibraryExW_addr = kernel32 ? GetProcAddress(kernel32, "LoadLibraryExW") : nullptr;
	if (!LoadLibraryExW_addr)
	{
		spdlog::error("Failed to find LoadLibraryExW address?");
		return 0;
	}

	// Hook LoadLibraryExW so we can watch for nvngx library load
	{
		auto builder = SafetyHookFactory::acquire();
		LoadLibraryExW_Orig = builder.create_inline(LoadLibraryExW_addr, LoadLibraryExW_Hook);
	}

	if (!watchIniUpdates)
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
					DWORD name_len = evt->FileNameLength / sizeof(wchar_t);
					std::wstring name = std::wstring(evt->FileName, name_len);
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

HMODULE ourModule = 0;
BOOL APIENTRY DllMain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		ourModule = hModule;
		proxy::on_attach(ourModule);

		_beginthreadex(NULL, 0, InitThread, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		proxy::on_detach();
	}

	return TRUE;
}
