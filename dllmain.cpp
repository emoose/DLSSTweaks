#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <psapi.h>
#include "MinHook/MinHook.h"
#include <filesystem>
#include <fstream>
#include <mutex>

// proxy.cpp
bool Proxy_Attach();
void Proxy_Detach();

// Method of operation:
// - Our DLL gets loaded in via DLL wrapping, with our code being ran right before program execution
//  - At our entrypoint we spawn a new thread which then hooks the LoadLibraryExW win32 function, which is used by NGX library code to load in the NGX DLL
// 
// - In our LoadLibraryExW hook, we check whether _nvngx.dll has been loaded in, once it has:
//  - Hooks are applied to a bunch of _nvngx.dll exports that games call into to init & setup DLSS
//  - The LoadLibraryExW hook is disabled since we no longer need it
// 
// - One of the main exports we hook are the NVSDK_NGX_XXX_XXXParameters functions
//  - Once one of those is called by the game, NVNGX populates a pointer to the NVSDK_NGX_Parameter class
//  - From that populated class pointer we fetch the vftable (which should always remain the same order), and find the function pointers for the SetUI & GetUI functions
//  - With those pointers in hand we can hook those two functions, so that we can override settings that the game tries to apply (DLSS3.1 presets), or requests back from DLSS (target resolutions)
//  - After finding those pointers we can disable all the XXXParameters hooks as they're no longer needed
// 
// - We also hook the NVSDK_NGX_XXX_Init functions, as those are used by the game to tell DLSS what the games app ID is, which DLSS sometimes uses to enforce certain DLSS3.1 presets
//   (by hooking that we can pretend the game is a generic app instead, letting us apply any preset we like)

// Misc notes about DLSS hooking:
// - nvngx_dlss.dll can't be DLL wrapped, the NV-provided library code that games use to call functions from it
//   includes a bunch of signature-verification code inside it for whatever reason
//
// - some games fail to have presets set on them - seems that nvngx is forcing presets based on appid?
//   was able to workaround that with the NGX_XXX_Init hooks below, not fully sure whether override will work across all 3 types though (_Init, _Init_Ext, _Init_ProjectId...)
//   very strange though, seems devs can't change their preset at all once NV has forced one on them...

std::filesystem::path LogPath;
bool DebugLog = true;
void dlog(const char* Format, ...)
{
	if (!DebugLog)
		return;

	char str[4096] = { 0 };
	va_list ap;
	va_start(ap, Format);

	vsnprintf(str, 4096, Format, ap);
	va_end(ap);

	std::ofstream file;
	file.open(LogPath, std::ofstream::out | std::ofstream::app);
	if (!file.is_open())
		return;

	file << str;

	file.close();
}

#define NVSDK_NGX_Parameter_Width "Width"
#define NVSDK_NGX_Parameter_Height "Height"
#define NVSDK_NGX_Parameter_OutWidth "OutWidth"
#define NVSDK_NGX_Parameter_OutHeight "OutHeight"
#define NVSDK_NGX_Parameter_PerfQualityValue "PerfQualityValue"
#define NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags "DLSS.Feature.Create.Flags"
#define NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA "DLSS.Hint.Render.Preset.DLAA"
#define NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality "DLSS.Hint.Render.Preset.Quality"
#define NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced "DLSS.Hint.Render.Preset.Balanced"
#define NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance "DLSS.Hint.Render.Preset.Performance"
#define NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance "DLSS.Hint.Render.Preset.UltraPerformance"

enum NVSDK_NGX_DLSS_Hint_Render_Preset
{
	NVSDK_NGX_DLSS_Hint_Render_Preset_Default,
	NVSDK_NGX_DLSS_Hint_Render_Preset_A,
	NVSDK_NGX_DLSS_Hint_Render_Preset_B,
	NVSDK_NGX_DLSS_Hint_Render_Preset_C,
	NVSDK_NGX_DLSS_Hint_Render_Preset_D,
	NVSDK_NGX_DLSS_Hint_Render_Preset_E,
	NVSDK_NGX_DLSS_Hint_Render_Preset_F,
};

bool forceDLAA = false;
bool forceAutoExposure = false;
bool overrideAppId = false;
bool overrideQualityRatios = false;
unsigned int presetDLAA = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
unsigned int presetQuality = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
unsigned int presetBalanced = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
unsigned int presetPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
unsigned int presetUltraPerformance = NVSDK_NGX_DLSS_Hint_Render_Preset_Default;

const char* projectIdOverride = "24480451-f00d-face-1304-0308dabad187";
const unsigned long long appIdOverride = 0x24480451;

// TODO: InFeatureInfo might also hold project ID related fields, maybe should change those too...
typedef uint64_t(__cdecl* NVSDK_NGX_D3D_Init_Fn)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const void* InFeatureInfo, void* InSDKVersion);
NVSDK_NGX_D3D_Init_Fn NVSDK_NGX_D3D11_Init;
uint64_t __cdecl NVSDK_NGX_D3D11_Init_Hook(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const void* InFeatureInfo, void* InSDKVersion)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_D3D11_Init(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
typedef uint64_t(__cdecl* NVSDK_NGX_D3D_Init_Ext_Fn)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5);
NVSDK_NGX_D3D_Init_Ext_Fn NVSDK_NGX_D3D11_Init_Ext;
uint64_t __cdecl NVSDK_NGX_D3D11_Init_Ext_Hook(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_D3D11_Init_Ext(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
typedef uint64_t(__cdecl* NVSDK_NGX_D3D_Init_ProjectID_Fn)(const char* InProjectId, enum NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const struct NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, enum NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_D3D_Init_ProjectID_Fn NVSDK_NGX_D3D11_Init_ProjectID;
uint64_t __cdecl NVSDK_NGX_D3D11_Init_ProjectID_Hook(const char* InProjectId, enum NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const struct NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, enum NVSDK_NGX_Version InSDKVersion)
{
	if (overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_D3D11_Init_ProjectID(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

NVSDK_NGX_D3D_Init_Fn NVSDK_NGX_D3D12_Init;
uint64_t __cdecl NVSDK_NGX_D3D12_Init_Hook(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const void* InFeatureInfo, void* InSDKVersion)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_D3D12_Init(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
NVSDK_NGX_D3D_Init_Ext_Fn NVSDK_NGX_D3D12_Init_Ext;
uint64_t __cdecl NVSDK_NGX_D3D12_Init_Ext_Hook(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_D3D12_Init_Ext(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
NVSDK_NGX_D3D_Init_ProjectID_Fn NVSDK_NGX_D3D12_Init_ProjectID;
uint64_t __cdecl NVSDK_NGX_D3D12_Init_ProjectID_Hook(const char* InProjectId, enum NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const struct NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, enum NVSDK_NGX_Version InSDKVersion)
{
	if (overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_D3D12_Init_ProjectID(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

typedef uint64_t(__cdecl* NVSDK_NGX_VULKAN_Init_Fn)(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6);
NVSDK_NGX_VULKAN_Init_Fn NVSDK_NGX_VULKAN_Init;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Hook(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_VULKAN_Init(InApplicationId, a2, a3, a4, a5, a6);
}
typedef uint64_t(__cdecl* NVSDK_NGX_VULKAN_Init_Ext_Fn)(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7);
NVSDK_NGX_VULKAN_Init_Ext_Fn NVSDK_NGX_VULKAN_Init_Ext;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Ext_Hook(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_VULKAN_Init_Ext(InApplicationId, a2, a3, a4, a5, a6, a7);
}
typedef uint64_t(__cdecl* NVSDK_NGX_VULKAN_Init_Ext2_Fn)(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9);
NVSDK_NGX_VULKAN_Init_Ext2_Fn NVSDK_NGX_VULKAN_Init_Ext2;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_Ext2_Hook(unsigned long long InApplicationId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9)
{
	if (overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_VULKAN_Init_Ext2(InApplicationId, a2, a3, a4, a5, a6, a7, a8, a9);
}
typedef uint64_t(__cdecl* NVSDK_NGX_VULKAN_Init_ProjectID_Fn)(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9);
NVSDK_NGX_VULKAN_Init_ProjectID_Fn NVSDK_NGX_VULKAN_Init_ProjectID;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_ProjectID_Hook(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9)
{
	if (overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_VULKAN_Init_ProjectID(InProjectId, a2, a3, a4, a5, a6, a7, a8, a9);
}
typedef uint64_t(__cdecl* NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Fn)(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9, void* a10, void* a11);
NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Fn NVSDK_NGX_VULKAN_Init_ProjectID_Ext;
uint64_t __cdecl NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook(const char* InProjectId, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9, void* a10, void* a11)
{
	if (overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_VULKAN_Init_ProjectID_Ext(InProjectId, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
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

typedef void(__cdecl* NVSDK_NGX_Parameter_SetI_Fn)(void* InParameter, const char* InName, int InValue);
NVSDK_NGX_Parameter_SetI_Fn NVSDK_NGX_Parameter_SetI;
void __cdecl NVSDK_NGX_Parameter_SetI_Hook(void* InParameter, const char* InName, int InValue)
{
	if (forceAutoExposure && !_stricmp(InName, NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags))
		InValue |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

	// Cache the chosen quality value so we can make decisions on it later on
	// TODO: maybe should setup NVSDK_NGX_Parameter class so we could call GetI to fetch this instead, might be more reliable...
	if (!_stricmp(InName, NVSDK_NGX_Parameter_PerfQualityValue))
	{
		//InValue = NVSDK_NGX_PerfQuality_Value_UltraQuality;
		auto val = NVSDK_NGX_PerfQuality_Value(InValue);
		if (prevQualityValue != val)
			prevQualityValue = val;
	}

	NVSDK_NGX_Parameter_SetI(InParameter, InName, InValue);
}

typedef void(__cdecl* NVSDK_NGX_Parameter_SetUI_Fn)(void* InParameter, const char* InName, unsigned int InValue);
NVSDK_NGX_Parameter_SetUI_Fn NVSDK_NGX_Parameter_SetUI;
void __cdecl NVSDK_NGX_Parameter_SetUI_Hook(void* InParameter, const char* InName, unsigned int InValue)
{
	NVSDK_NGX_Parameter_SetUI(InParameter, InName, InValue);

	if (presetDLAA != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, presetDLAA);
	if (presetQuality != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, presetQuality);
	if (presetBalanced != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, presetBalanced);
	if (presetPerformance != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, presetPerformance);
	if (presetUltraPerformance != NVSDK_NGX_DLSS_Hint_Render_Preset_Default)
		NVSDK_NGX_Parameter_SetUI(InParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, presetUltraPerformance);
}

float qualityLevelRatios[] = {
	0.5f, // NVSDK_NGX_PerfQuality_Value_MaxPerf
	0.58f, // NVSDK_NGX_PerfQuality_Value_Balanced
	0.66666667f, // NVSDK_NGX_PerfQuality_Value_MaxQuality
	0.33333334f, // NVSDK_NGX_PerfQuality_Value_UltraPerformance
	0.77f, // UNSURE: NVSDK_NGX_PerfQuality_Value_UltraQuality
};

typedef uint64_t(__cdecl* NVSDK_NGX_Parameter_GetUI_Fn)(void* InParameter, const char* InName, unsigned int* OutValue);
NVSDK_NGX_Parameter_GetUI_Fn NVSDK_NGX_Parameter_GetUI;
uint64_t __cdecl NVSDK_NGX_Parameter_GetUI_Hook(void* InParameter, const char* InName, unsigned int* OutValue)
{
	uint64_t ret = NVSDK_NGX_Parameter_GetUI(InParameter, InName, OutValue);

	if (ret == 1)
	{
		bool isOutWidth = !_stricmp(InName, NVSDK_NGX_Parameter_OutWidth);
		bool isOutHeight = !_stricmp(InName, NVSDK_NGX_Parameter_OutHeight);
		if (overrideQualityRatios)
		{
			if (isOutWidth)
			{
				unsigned int targetWidth = 0;
				NVSDK_NGX_Parameter_GetUI(InParameter, NVSDK_NGX_Parameter_Width, &targetWidth); // fetch full screen width
				targetWidth = unsigned int(roundf(float(targetWidth) * qualityLevelRatios[int(prevQualityValue)])); // calculate new width from custom ratio
				*OutValue = targetWidth;
			}
			if (isOutHeight)
			{
				unsigned int targetHeight = 0;
				NVSDK_NGX_Parameter_GetUI(InParameter, NVSDK_NGX_Parameter_Height, &targetHeight); // fetch full screen height
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
				NVSDK_NGX_Parameter_GetUI(InParameter, NVSDK_NGX_Parameter_Width, OutValue);
			if (overrideHeight && *OutValue != 0)
				NVSDK_NGX_Parameter_GetUI(InParameter, NVSDK_NGX_Parameter_Height, OutValue);
		}
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

void* NVSDK_NGX_D3D11_AllocateParameters_Target = nullptr;
void* NVSDK_NGX_D3D11_GetCapabilityParameters_Target = nullptr;
void* NVSDK_NGX_D3D11_GetParameters_Target = nullptr;
void* NVSDK_NGX_D3D12_AllocateParameters_Target = nullptr;
void* NVSDK_NGX_D3D12_GetCapabilityParameters_Target = nullptr;
void* NVSDK_NGX_D3D12_GetParameters_Target = nullptr;
void* NVSDK_NGX_VULKAN_AllocateParameters_Target = nullptr;
void* NVSDK_NGX_VULKAN_GetCapabilityParameters_Target = nullptr;
void* NVSDK_NGX_VULKAN_GetParameters_Target = nullptr;

std::mutex paramHookMutex;
bool isParamFuncsHooked = false;
bool DLSSApplyParamFunctionHooks(NVSDK_NGX_Parameter_vftable* vftable)
{
	std::lock_guard<std::mutex> lock(paramHookMutex);

	if (isParamFuncsHooked)
		return true;

	auto* NVSDK_NGX_Parameter_SetI_orig = vftable->SetI;
	auto* NVSDK_NGX_Parameter_SetUI_orig = vftable->SetUI;
	auto* NVSDK_NGX_Parameter_GetUI_orig = vftable->GetUI;

	if (NVSDK_NGX_Parameter_SetI_orig && NVSDK_NGX_Parameter_SetUI_orig && NVSDK_NGX_Parameter_GetUI_orig)
	{
		MH_CreateHook(NVSDK_NGX_Parameter_SetI_orig, NVSDK_NGX_Parameter_SetI_Hook, (LPVOID*)&NVSDK_NGX_Parameter_SetI);
		MH_CreateHook(NVSDK_NGX_Parameter_SetUI_orig, NVSDK_NGX_Parameter_SetUI_Hook, (LPVOID*)&NVSDK_NGX_Parameter_SetUI);
		MH_CreateHook(NVSDK_NGX_Parameter_GetUI_orig, NVSDK_NGX_Parameter_GetUI_Hook, (LPVOID*)&NVSDK_NGX_Parameter_GetUI);
		MH_EnableHook(NVSDK_NGX_Parameter_SetI_orig);
		MH_EnableHook(NVSDK_NGX_Parameter_SetUI_orig);
		MH_EnableHook(NVSDK_NGX_Parameter_GetUI_orig);

		dlog("DLSS functions found & parameter hooks applied!\n");
		dlog("Settings:\n - ForceDLAA: %s\n - ForceAutoExposure: %s\n - OverrideAppId: %s\n", 
			forceDLAA ? "true" : "false", 
			forceAutoExposure ? "true" : "false",
			overrideAppId ? "true" : "false");

		// disable NGX param export hooks since they aren't needed now
		MH_DisableHook(NVSDK_NGX_D3D11_AllocateParameters_Target);
		MH_DisableHook(NVSDK_NGX_D3D11_GetCapabilityParameters_Target);
		MH_DisableHook(NVSDK_NGX_D3D11_GetParameters_Target);
		MH_DisableHook(NVSDK_NGX_D3D12_AllocateParameters_Target);
		MH_DisableHook(NVSDK_NGX_D3D12_GetCapabilityParameters_Target);
		MH_DisableHook(NVSDK_NGX_D3D12_GetParameters_Target);
		MH_DisableHook(NVSDK_NGX_VULKAN_AllocateParameters_Target);
		MH_DisableHook(NVSDK_NGX_VULKAN_GetCapabilityParameters_Target);
		MH_DisableHook(NVSDK_NGX_VULKAN_GetParameters_Target);

		isParamFuncsHooked = true;
	}

	return isParamFuncsHooked;
}

typedef uint64_t(__cdecl* NVSDK_NGX_D3D_AllocateParameters_Fn)(NVSDK_NGX_Parameter** OutParameters);
NVSDK_NGX_D3D_AllocateParameters_Fn NVSDK_NGX_D3D12_AllocateParameters;
uint64_t __cdecl NVSDK_NGX_D3D12_AllocateParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D12_AllocateParameters(OutParameters);
	if (*OutParameters)
		DLSSApplyParamFunctionHooks((*OutParameters)->_vftable);
	return ret;
}
NVSDK_NGX_D3D_AllocateParameters_Fn NVSDK_NGX_D3D12_GetCapabilityParameters;
uint64_t __cdecl NVSDK_NGX_D3D12_GetCapabilityParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D12_GetCapabilityParameters(OutParameters);
	if (*OutParameters)
		DLSSApplyParamFunctionHooks((*OutParameters)->_vftable);
	return ret;
}
NVSDK_NGX_D3D_AllocateParameters_Fn NVSDK_NGX_D3D12_GetParameters;
uint64_t __cdecl NVSDK_NGX_D3D12_GetParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D12_GetParameters(OutParameters);
	if (*OutParameters)
		DLSSApplyParamFunctionHooks((*OutParameters)->_vftable);
	return ret;
}

NVSDK_NGX_D3D_AllocateParameters_Fn NVSDK_NGX_D3D11_AllocateParameters;
uint64_t __cdecl NVSDK_NGX_D3D11_AllocateParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D11_AllocateParameters(OutParameters);
	if (*OutParameters)
		DLSSApplyParamFunctionHooks((*OutParameters)->_vftable);
	return ret;
}
NVSDK_NGX_D3D_AllocateParameters_Fn NVSDK_NGX_D3D11_GetCapabilityParameters;
uint64_t __cdecl NVSDK_NGX_D3D11_GetCapabilityParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D11_GetCapabilityParameters(OutParameters);
	if (*OutParameters)
		DLSSApplyParamFunctionHooks((*OutParameters)->_vftable);
	return ret;
}
NVSDK_NGX_D3D_AllocateParameters_Fn NVSDK_NGX_D3D11_GetParameters;
uint64_t __cdecl NVSDK_NGX_D3D11_GetParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_D3D11_GetParameters(OutParameters);
	if (*OutParameters)
		DLSSApplyParamFunctionHooks((*OutParameters)->_vftable);
	return ret;
}

NVSDK_NGX_D3D_AllocateParameters_Fn NVSDK_NGX_VULKAN_AllocateParameters;
uint64_t __cdecl NVSDK_NGX_VULKAN_AllocateParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_VULKAN_AllocateParameters(OutParameters);
	if (*OutParameters)
		DLSSApplyParamFunctionHooks((*OutParameters)->_vftable);
	return ret;
}
NVSDK_NGX_D3D_AllocateParameters_Fn NVSDK_NGX_VULKAN_GetCapabilityParameters;
uint64_t __cdecl NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_VULKAN_GetCapabilityParameters(OutParameters);
	if (*OutParameters)
		DLSSApplyParamFunctionHooks((*OutParameters)->_vftable);
	return ret;
}
NVSDK_NGX_D3D_AllocateParameters_Fn NVSDK_NGX_VULKAN_GetParameters;
uint64_t __cdecl NVSDK_NGX_VULKAN_GetParameters_Hook(NVSDK_NGX_Parameter** OutParameters)
{
	uint64_t ret = NVSDK_NGX_VULKAN_GetParameters(OutParameters);
	if (*OutParameters)
		DLSSApplyParamFunctionHooks((*OutParameters)->_vftable);
	return ret;
}

void* LoadLibraryExW_Target = nullptr;

std::mutex scanMutex;
bool isNgxInitHooked = false;
bool ScanForDLSSFunctions()
{
	std::lock_guard<std::mutex> lock(scanMutex);

	if (isNgxInitHooked)
		return isNgxInitHooked;

	HMODULE ngx_module = GetModuleHandleA("_nvngx.dll");
	if (!ngx_module)
		return isNgxInitHooked;

	auto* NVSDK_NGX_D3D11_Init_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_Init");
	auto* NVSDK_NGX_D3D11_Init_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_Init_Ext");
	auto* NVSDK_NGX_D3D11_Init_ProjectID_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_Init_ProjectID");
	NVSDK_NGX_D3D11_AllocateParameters_Target = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_AllocateParameters");
	NVSDK_NGX_D3D11_GetCapabilityParameters_Target = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_GetCapabilityParameters");
	NVSDK_NGX_D3D11_GetParameters_Target = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_GetParameters");

	auto* NVSDK_NGX_D3D12_Init_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_Init");
	auto* NVSDK_NGX_D3D12_Init_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_Init_Ext");
	auto* NVSDK_NGX_D3D12_Init_ProjectID_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_Init_ProjectID");
	NVSDK_NGX_D3D12_AllocateParameters_Target = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_AllocateParameters");
	NVSDK_NGX_D3D12_GetCapabilityParameters_Target = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_GetCapabilityParameters");
	NVSDK_NGX_D3D12_GetParameters_Target = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_GetParameters");

	auto* NVSDK_NGX_VULKAN_Init_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init");
	auto* NVSDK_NGX_VULKAN_Init_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_Ext");
	auto* NVSDK_NGX_VULKAN_Init_Ext2_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_Ext2");
	auto* NVSDK_NGX_VULKAN_Init_ProjectID_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_ProjectID");
	auto* NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_ProjectID_Ext");
	NVSDK_NGX_VULKAN_AllocateParameters_Target = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_AllocateParameters");
	NVSDK_NGX_VULKAN_GetCapabilityParameters_Target = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_GetCapabilityParameters");
	NVSDK_NGX_VULKAN_GetParameters_Target = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_GetParameters");

	// Make sure we only try hooking if we found all the procs above...
	if (NVSDK_NGX_D3D11_Init_orig && NVSDK_NGX_D3D11_Init_Ext_orig && NVSDK_NGX_D3D11_Init_ProjectID_orig &&
		NVSDK_NGX_D3D11_AllocateParameters_Target && NVSDK_NGX_D3D11_GetCapabilityParameters_Target && NVSDK_NGX_D3D11_GetParameters_Target &&
		NVSDK_NGX_D3D12_Init_orig && NVSDK_NGX_D3D12_Init_Ext_orig && NVSDK_NGX_D3D12_Init_ProjectID_orig &&
		NVSDK_NGX_D3D12_AllocateParameters_Target && NVSDK_NGX_D3D12_GetCapabilityParameters_Target && NVSDK_NGX_D3D12_GetParameters_Target &&
		NVSDK_NGX_VULKAN_Init_orig && NVSDK_NGX_VULKAN_Init_Ext_orig && NVSDK_NGX_VULKAN_Init_Ext2_orig && NVSDK_NGX_VULKAN_Init_ProjectID_orig && 
		NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig &&
		NVSDK_NGX_VULKAN_AllocateParameters_Target && NVSDK_NGX_VULKAN_GetCapabilityParameters_Target && NVSDK_NGX_VULKAN_GetParameters_Target)
	{
		MH_CreateHook(NVSDK_NGX_D3D11_Init_orig, NVSDK_NGX_D3D11_Init_Hook, (LPVOID*)&NVSDK_NGX_D3D11_Init);
		MH_EnableHook(NVSDK_NGX_D3D11_Init_orig);
		MH_CreateHook(NVSDK_NGX_D3D11_Init_Ext_orig, NVSDK_NGX_D3D11_Init_Ext_Hook, (LPVOID*)&NVSDK_NGX_D3D11_Init_Ext);
		MH_EnableHook(NVSDK_NGX_D3D11_Init_Ext_orig);
		MH_CreateHook(NVSDK_NGX_D3D11_Init_ProjectID_orig, NVSDK_NGX_D3D11_Init_ProjectID_Hook, (LPVOID*)&NVSDK_NGX_D3D11_Init_ProjectID);
		MH_EnableHook(NVSDK_NGX_D3D11_Init_ProjectID_orig);

		MH_CreateHook(NVSDK_NGX_D3D11_AllocateParameters_Target, NVSDK_NGX_D3D11_AllocateParameters_Hook, (LPVOID*)&NVSDK_NGX_D3D11_AllocateParameters);
		MH_EnableHook(NVSDK_NGX_D3D11_AllocateParameters_Target);
		MH_CreateHook(NVSDK_NGX_D3D11_GetCapabilityParameters_Target, NVSDK_NGX_D3D11_GetCapabilityParameters_Hook, (LPVOID*)&NVSDK_NGX_D3D11_GetCapabilityParameters);
		MH_EnableHook(NVSDK_NGX_D3D11_GetCapabilityParameters_Target);
		MH_CreateHook(NVSDK_NGX_D3D11_GetParameters_Target, NVSDK_NGX_D3D11_GetParameters_Hook, (LPVOID*)&NVSDK_NGX_D3D11_GetParameters);
		MH_EnableHook(NVSDK_NGX_D3D11_GetParameters_Target);

		MH_CreateHook(NVSDK_NGX_D3D12_Init_orig, NVSDK_NGX_D3D12_Init_Hook, (LPVOID*)&NVSDK_NGX_D3D12_Init);
		MH_EnableHook(NVSDK_NGX_D3D12_Init_orig);
		MH_CreateHook(NVSDK_NGX_D3D12_Init_Ext_orig, NVSDK_NGX_D3D12_Init_Ext_Hook, (LPVOID*)&NVSDK_NGX_D3D12_Init_Ext);
		MH_EnableHook(NVSDK_NGX_D3D12_Init_Ext_orig);
		MH_CreateHook(NVSDK_NGX_D3D12_Init_ProjectID_orig, NVSDK_NGX_D3D12_Init_ProjectID_Hook, (LPVOID*)&NVSDK_NGX_D3D12_Init_ProjectID);
		MH_EnableHook(NVSDK_NGX_D3D12_Init_ProjectID_orig);

		MH_CreateHook(NVSDK_NGX_D3D12_AllocateParameters_Target, NVSDK_NGX_D3D12_AllocateParameters_Hook, (LPVOID*)&NVSDK_NGX_D3D12_AllocateParameters);
		MH_EnableHook(NVSDK_NGX_D3D12_AllocateParameters_Target);
		MH_CreateHook(NVSDK_NGX_D3D12_GetCapabilityParameters_Target, NVSDK_NGX_D3D12_GetCapabilityParameters_Hook, (LPVOID*)&NVSDK_NGX_D3D12_GetCapabilityParameters);
		MH_EnableHook(NVSDK_NGX_D3D12_GetCapabilityParameters_Target);
		MH_CreateHook(NVSDK_NGX_D3D12_GetParameters_Target, NVSDK_NGX_D3D12_GetParameters_Hook, (LPVOID*)&NVSDK_NGX_D3D12_GetParameters);
		MH_EnableHook(NVSDK_NGX_D3D12_GetParameters_Target);

		MH_CreateHook(NVSDK_NGX_VULKAN_Init_orig, NVSDK_NGX_VULKAN_Init_Hook, (LPVOID*)&NVSDK_NGX_VULKAN_Init);
		MH_EnableHook(NVSDK_NGX_VULKAN_Init_orig);
		MH_CreateHook(NVSDK_NGX_VULKAN_Init_Ext_orig, NVSDK_NGX_VULKAN_Init_Ext_Hook, (LPVOID*)&NVSDK_NGX_VULKAN_Init_Ext);
		MH_EnableHook(NVSDK_NGX_VULKAN_Init_Ext_orig);
		MH_CreateHook(NVSDK_NGX_VULKAN_Init_Ext2_orig, NVSDK_NGX_VULKAN_Init_Ext2_Hook, (LPVOID*)&NVSDK_NGX_VULKAN_Init_Ext2);
		MH_EnableHook(NVSDK_NGX_VULKAN_Init_Ext2_orig);
		MH_CreateHook(NVSDK_NGX_VULKAN_Init_ProjectID_orig, NVSDK_NGX_VULKAN_Init_ProjectID_Hook, (LPVOID*)&NVSDK_NGX_VULKAN_Init_ProjectID);
		MH_EnableHook(NVSDK_NGX_VULKAN_Init_ProjectID_orig);
		MH_CreateHook(NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig, NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook, (LPVOID*)&NVSDK_NGX_VULKAN_Init_ProjectID_Ext);
		MH_EnableHook(NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig);

		MH_CreateHook(NVSDK_NGX_VULKAN_AllocateParameters_Target, NVSDK_NGX_VULKAN_AllocateParameters_Hook, (LPVOID*)&NVSDK_NGX_VULKAN_AllocateParameters);
		MH_EnableHook(NVSDK_NGX_VULKAN_AllocateParameters_Target);
		MH_CreateHook(NVSDK_NGX_VULKAN_GetCapabilityParameters_Target, NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook, (LPVOID*)&NVSDK_NGX_VULKAN_GetCapabilityParameters);
		MH_EnableHook(NVSDK_NGX_VULKAN_GetCapabilityParameters_Target);
		MH_CreateHook(NVSDK_NGX_VULKAN_GetParameters_Target, NVSDK_NGX_VULKAN_GetParameters_Hook, (LPVOID*)&NVSDK_NGX_VULKAN_GetParameters);
		MH_EnableHook(NVSDK_NGX_VULKAN_GetParameters_Target);

		dlog("Applied _nvngx.dll DLL export hooks, waiting for game to call them...\n");

		isNgxInitHooked = true;
	}
	else
	{
		dlog("Failed to locate all _nvngx.dll functions, may have been changed by driver update :/\n");
	}

	// _nvngx.dll was found & hook attempted, disable LoadLibraryExW hook now
	MH_DisableHook(LoadLibraryExW_Target);

	return isNgxInitHooked;
}

// Hook LoadLibraryExW so we can scan it for the DLSS funcs immediately after the module is loaded in
HMODULE(__stdcall* LoadLibraryExW_Orig)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE __stdcall LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	HMODULE ret = LoadLibraryExW_Orig(lpLibFileName, hFile, dwFlags);

	ScanForDLSSFunctions();

	return ret;
}

bool GetPrivateProfileBool(const wchar_t* path, const wchar_t* app, const wchar_t* key, bool default_val)
{
	WCHAR val[256];
	const wchar_t* default_val_str = default_val ? L"true" : L"false";
	GetPrivateProfileStringW(app, key, default_val_str, val, 256, path);

	if (!_wcsicmp(val, L"true"))
		return true;
	if (!_wcsicmp(val, L"yes"))
		return true;
	if (!_wcsicmp(val, L"1"))
		return true;

	return false;
}

float GetPrivateProfileFloat(const wchar_t* path, const wchar_t* app, const wchar_t* key, float default_val)
{
	WCHAR val[256];
	const wchar_t* default_val_str = default_val ? L"true" : L"false";
	GetPrivateProfileStringW(app, key, default_val_str, val, 256, path);

	float ret = default_val;
	try
	{
		ret = std::stof(val);
	}
	catch (const std::exception&)
	{
		ret = default_val;
	}
	return ret;
}

unsigned int GetPrivateProfileDlssPreset(const wchar_t* path, const wchar_t* app, const wchar_t* key)
{
	WCHAR val[256];
	const wchar_t* default_val = L"Default";
	GetPrivateProfileStringW(app, key, default_val, val, 256, path);

	if (!_wcsicmp(val, default_val))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	if (!_wcsicmp(val, L"A"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_A;
	if (!_wcsicmp(val, L"B"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_B;
	if (!_wcsicmp(val, L"C"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_C;
	if (!_wcsicmp(val, L"D"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_D;
	if (!_wcsicmp(val, L"E"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_E;
	if (!_wcsicmp(val, L"F"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_F;

	return NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
}

DWORD WINAPI HookThread(LPVOID lpParam)
{
	printf("DLSSTweaks v0.123.8, by emoose\n");
	printf("https://github.com/emoose/DLSSTweaks\n");

	WCHAR exePath[4096];
	GetModuleFileNameW(GetModuleHandleA(0), exePath, 4096);

	LogPath = std::filesystem::path(exePath).parent_path() / "dlsstweaks.log";

	std::filesystem::path iniPath = std::filesystem::path(exePath).parent_path() / "dlsstweaks.ini";
	std::wstring iniPathStr = iniPath.wstring();
	const wchar_t* cfg_IniName = iniPathStr.c_str();

	DebugLog = GetPrivateProfileBool(cfg_IniName, L"DLSS", L"DebugLog", DebugLog);
	forceDLAA = GetPrivateProfileBool(cfg_IniName, L"DLSS", L"ForceDLAA", forceDLAA);
	forceAutoExposure = GetPrivateProfileBool(cfg_IniName, L"DLSS", L"ForceAutoExposure", forceAutoExposure);
	overrideAppId = GetPrivateProfileBool(cfg_IniName, L"DLSS", L"OverrideAppId", overrideAppId);
	overrideQualityRatios = GetPrivateProfileBool(cfg_IniName, L"DLSSQualityRatios", L"Enable", overrideQualityRatios);
	if (overrideQualityRatios)
	{
		qualityLevelRatios[0] = GetPrivateProfileFloat(cfg_IniName, L"DLSSQualityRatios", L"Performance", qualityLevelRatios[0]); // NVSDK_NGX_PerfQuality_Value_MaxPerf
		qualityLevelRatios[1] = GetPrivateProfileFloat(cfg_IniName, L"DLSSQualityRatios", L"Balanced", qualityLevelRatios[1]); // NVSDK_NGX_PerfQuality_Value_Balanced
		qualityLevelRatios[2] = GetPrivateProfileFloat(cfg_IniName, L"DLSSQualityRatios", L"Quality", qualityLevelRatios[2]); // NVSDK_NGX_PerfQuality_Value_MaxQuality
		qualityLevelRatios[3] = GetPrivateProfileFloat(cfg_IniName, L"DLSSQualityRatios", L"UltraPerformance", qualityLevelRatios[3]); // NVSDK_NGX_PerfQuality_Value_UltraPerformance
		qualityLevelRatios[4] = GetPrivateProfileFloat(cfg_IniName, L"DLSSQualityRatios", L"UltraQuality", qualityLevelRatios[4]); // NVSDK_NGX_PerfQuality_Value_UltraQuality
	}
	presetDLAA = GetPrivateProfileDlssPreset(cfg_IniName, L"DLSSPresets", L"DLAA");
	presetQuality = GetPrivateProfileDlssPreset(cfg_IniName, L"DLSSPresets", L"Quality");
	presetBalanced = GetPrivateProfileDlssPreset(cfg_IniName, L"DLSSPresets", L"Balanced");
	presetPerformance = GetPrivateProfileDlssPreset(cfg_IniName, L"DLSSPresets", L"Performance");
	presetUltraPerformance = GetPrivateProfileDlssPreset(cfg_IniName, L"DLSSPresets", L"UltraPerformance");

	dlog("\nDLSSTweaks v0.123.8, by emoose: DLL wrapper loaded, watching for DLSS library load...\n");

	MH_Initialize();
	MH_CreateHookApiEx(L"kernel32", "LoadLibraryExW", LoadLibraryExW_Hook, (LPVOID*)&LoadLibraryExW_Orig, &LoadLibraryExW_Target);
	MH_EnableHook(LoadLibraryExW_Target);

	return 0;
}

HMODULE ourModule = 0;
BOOL APIENTRY DllMain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		ourModule = hModule;
		Proxy_Attach();

		CreateThread(NULL, 0, HookThread, NULL, 0, NULL);
	}
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Proxy_Detach();
	}

	return TRUE;
}
