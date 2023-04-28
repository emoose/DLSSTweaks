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

// the hooks defined below can work using two different methods
// method 0: if we were loaded in via nvngx wrapper (requires reg edit), the hooks below are exported as the nvngx functions themselves, which do our work and then call back to original DLL
// method 1: if we were loaded in via other DLL wrapper (dxgi etc), then _nvngx.dll is hooked to call these functions instead, and call back to _nvngx via safetyhook unsafe_call
// these are handled via either nvngx::hook or nvngx::init setting up the HookOrigFn structs for each function

// Note: some titles are only stable with DLSSTweaks when the VS compiler decides to let these functions exit via JMP instead of via CALL
// When exiting via CALL some titles just seem to start having really strange issues, guess maybe stack space of the thread is very limited and CALL makes it go over the edge, but not completely sure
// After some experiments the unsafe_call/call functions below now fortunately let it exit via JMP, but any compiler change (or even building in Debug mode) may change that
// If any crashes are noticed after these are called it might be worth checking these functions in a disassembler first
// (https://github.com/emoose/DLSSTweaks/issues/44#issuecomment-1468518380 for more info... if anyone has any idea for a better fix I'd be happy to hear it)
namespace nvngx
{
NVSDK_NGX_PerfQuality_Value prevQualityValue; // the last quality level setting that game requested
std::optional<ID3D12Resource*> prevExposureTexture;

void on_evaluate_feature(const NVSDK_NGX_Parameter* InParameters)
{
	// Check the current ExposureTexture value and see if game has set one or not
	// Depending on value we'll recommend what user should change the OverrideAutoExposure setting to
	ID3D12Resource* pInExposureTexture;
	if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &pInExposureTexture) != NVSDK_NGX_Result_Success)
		pInExposureTexture = nullptr;

	// Some games seem to rapidly change between two different textures (https://github.com/emoose/DLSSTweaks/issues/67)
	// But we're only interested in whether game is using a pInExposureTexture or not
	// So we'll only print user warning about exposure texture if:
	// - We haven't looked at it before
	// - Game has switched from non-null texture to null
	// - Game has switched from null texture to non-null
	if (!prevExposureTexture.has_value() || 
		(*prevExposureTexture != nullptr && pInExposureTexture == nullptr) ||
		(*prevExposureTexture == nullptr && pInExposureTexture != nullptr))
	{
		// if we recommended user to change settings previously, make sure that we _always_ log later exposure changes into game log...
		// in case game changed exposure settings after the first recommendation
		static bool userBeenWarned = false; 

		if (pInExposureTexture)
		{
			spdlog::log(userBeenWarned ? spdlog::level::warn : spdlog::level::debug, 
				"NVSDK_NGX_EvaluateFeature: pInExposureTexture changed to texture at {}, game using custom exposure value", (void*)pInExposureTexture);

			if (settings.dlss.featureCreateFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure)
			{
				spdlog::warn("NVSDK_NGX_EvaluateFeature: game is using custom exposure value, but is also setting AutoExposure flag itself - changing OverrideAutoExposure to -1 may be beneficial");
				userBeenWarned = true;
			}

			if (settings.overrideAutoExposure > 0)
			{
				spdlog::warn("NVSDK_NGX_EvaluateFeature: game is using custom exposure value but OverrideAutoExposure is enabled, recommend setting to 0 or -1!");
				userBeenWarned = true;
			}
		}
		else
		{
			spdlog::log(userBeenWarned ? spdlog::level::warn : spdlog::level::debug,
				"NVSDK_NGX_EvaluateFeature: pInExposureTexture set to 0, game might not be using custom exposure value");

			if (settings.overrideAutoExposure <= 0 && !(settings.dlss.featureCreateFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure))
			{
				spdlog::warn("NVSDK_NGX_EvaluateFeature: game not using custom exposure value or AutoExposure, recommend setting OverrideAutoExposure to 1!");
				userBeenWarned = true;
			}
		}
	}

	prevExposureTexture = pInExposureTexture;
}

HookOrigFn NVSDK_NGX_D3D11_EvaluateFeature_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D11_EvaluateFeature(class ID3D11DeviceContext* InDevCtx, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, void* InCallback)
{
	on_evaluate_feature(InParameters);
	return NVSDK_NGX_D3D11_EvaluateFeature_Hook.unsafe_call<NVSDK_NGX_Result>(InDevCtx, InFeatureHandle, InParameters, InCallback);
}
// TODO: InFeatureInfo might also hold project ID related fields, maybe should change those too...
HookOrigFn NVSDK_NGX_D3D11_Init_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D11_Init_Hook.unsafe_call<NVSDK_NGX_Result>(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
HookOrigFn NVSDK_NGX_D3D11_Init_Ext_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D11_Init_Ext_Hook.unsafe_call<NVSDK_NGX_Result>(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
HookOrigFn NVSDK_NGX_D3D11_Init_ProjectID_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InProjectId = projectIdOverride;

	return NVSDK_NGX_D3D11_Init_ProjectID_Hook.unsafe_call<NVSDK_NGX_Result>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

HookOrigFn NVSDK_NGX_D3D12_EvaluateFeature_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D12_EvaluateFeature(class ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, void* InCallback)
{
	on_evaluate_feature(InParameters);
	return NVSDK_NGX_D3D12_EvaluateFeature_Hook.unsafe_call<NVSDK_NGX_Result>(InCmdList, InFeatureHandle, InParameters, InCallback);
}
HookOrigFn NVSDK_NGX_D3D12_Init_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D12_Init_Hook.unsafe_call<NVSDK_NGX_Result>(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}
HookOrigFn NVSDK_NGX_D3D12_Init_Ext_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, void* a3, void* a4, void* a5)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_D3D12_Init_Ext_Hook.unsafe_call<NVSDK_NGX_Result>(InApplicationId, InApplicationDataPath, a3, a4, a5);
}
HookOrigFn NVSDK_NGX_D3D12_Init_ProjectID_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D12_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, class ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InProjectId = projectIdOverride;

	return NVSDK_NGX_D3D12_Init_ProjectID_Hook.unsafe_call<NVSDK_NGX_Result>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
}

HookOrigFn NVSDK_NGX_VULKAN_EvaluateFeature_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_VULKAN_EvaluateFeature(void* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, void* InCallback)
{
	on_evaluate_feature(InParameters);
	return NVSDK_NGX_VULKAN_EvaluateFeature_Hook.unsafe_call<NVSDK_NGX_Result>(InCmdList, InFeatureHandle, InParameters, InCallback);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, struct VkInstance* InInstance, struct VkPhysicalDevice* InPD, struct VkDevice* InDevice, NVSDK_NGX_Version InSDKVersion)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_VULKAN_Init_Hook.unsafe_call<NVSDK_NGX_Result>(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InSDKVersion);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_Ext_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_VULKAN_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, struct VkInstance* InInstance, struct VkPhysicalDevice* InPD, struct VkDevice* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InApplicationId = appIdOverride;

	return NVSDK_NGX_VULKAN_Init_Ext_Hook.unsafe_call<NVSDK_NGX_Result>(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InSDKVersion, InFeatureInfo);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_ProjectID_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_VULKAN_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, struct VkInstance* InInstance, struct VkPhysicalDevice* InPD, struct VkDevice* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InProjectId = projectIdOverride;

	return NVSDK_NGX_VULKAN_Init_ProjectID_Hook.unsafe_call<NVSDK_NGX_Result>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InInstance, InPD, InDevice, InSDKVersion, InFeatureInfo);
}

// VULKAN_Init_Ext2 / VULKAN_Init_ProjectID_Ext are _nvngx.dll only, not included in nvngx.dll
HookOrigFn NVSDK_NGX_VULKAN_Init_Ext2_Hook;
NVSDK_NGX_Result __cdecl NVSDK_NGX_VULKAN_Init_Ext2(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, struct VkInstance* InInstance, struct VkPhysicalDevice* InPD, struct VkDevice* InDevice, void* InGIPA, void* InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
	if (settings.overrideAppId)
		InApplicationId = appIdOverride;
	return NVSDK_NGX_VULKAN_Init_Ext2_Hook.unsafe_call<NVSDK_NGX_Result>(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InSDKVersion, InFeatureInfo);
}
HookOrigFn NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook;
NVSDK_NGX_Result __cdecl NVSDK_NGX_VULKAN_Init_ProjectID_Ext(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, struct VkInstance* InInstance, struct VkPhysicalDevice* InPD, struct VkDevice* InDevice, void* InGIPA, void* InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
	WaitForInitThread();

	if (settings.overrideAppId)
		InProjectId = projectIdOverride;
	return NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook.unsafe_call<NVSDK_NGX_Result>(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InSDKVersion, InFeatureInfo);
}

SafetyHookInline NVSDK_NGX_Parameter_SetF_Hook;
void __cdecl NVSDK_NGX_Parameter_SetF(NVSDK_NGX_Parameter* InParameter, const char* InName, float InValue)
{
	// Sharpening override (pre-2.5.1 only)
	if (settings.overrideSharpening.has_value() && !_stricmp(InName, NVSDK_NGX_Parameter_Sharpness))
		InValue = *settings.overrideSharpening;

	NVSDK_NGX_Parameter_SetF_Hook.call(InParameter, InName, InValue);
}

SafetyHookInline NVSDK_NGX_Parameter_SetI_Hook;
void __cdecl NVSDK_NGX_Parameter_SetI(NVSDK_NGX_Parameter* InParameter, const char* InName, int InValue)
{
	if (!_stricmp(InName, NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags))
	{
		settings.dlss.featureCreateFlags = InValue;
		spdlog::debug("NVSDK_NGX_Parameter_SetI: FeatureCreateFlags = 0x{:X}", settings.dlss.featureCreateFlags);
		if (settings.dlss.featureCreateFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR)
			spdlog::debug("NVSDK_NGX_Parameter_SetI: - NVSDK_NGX_DLSS_Feature_Flags_IsHDR");
		if (settings.dlss.featureCreateFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes)
			spdlog::debug("NVSDK_NGX_Parameter_SetI: - NVSDK_NGX_DLSS_Feature_Flags_MVLowRes");
		if (settings.dlss.featureCreateFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered)
			spdlog::debug("NVSDK_NGX_Parameter_SetI: - NVSDK_NGX_DLSS_Feature_Flags_MVJittered");
		if (settings.dlss.featureCreateFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted)
			spdlog::debug("NVSDK_NGX_Parameter_SetI: - NVSDK_NGX_DLSS_Feature_Flags_DepthInverted");
		if (settings.dlss.featureCreateFlags & NVSDK_NGX_DLSS_Feature_Flags_Reserved_0)
			spdlog::debug("NVSDK_NGX_Parameter_SetI: - NVSDK_NGX_DLSS_Feature_Flags_Reserved_0");
		if (settings.dlss.featureCreateFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening)
			spdlog::debug("NVSDK_NGX_Parameter_SetI: - NVSDK_NGX_DLSS_Feature_Flags_DoSharpening (use \"OverrideSharpening = disable\" to force disable)");
		if (settings.dlss.featureCreateFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure)
			spdlog::debug("NVSDK_NGX_Parameter_SetI: - NVSDK_NGX_DLSS_Feature_Flags_AutoExposure (use \"OverrideAutoExposure = -1\" to force disable)");

		const int lastKnownFlag = NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
		auto remainder = settings.dlss.featureCreateFlags & ~((lastKnownFlag << 1) - 1);
		if (remainder)
			spdlog::debug("NVSDK_NGX_Parameter_SetI: - unknown flags: 0x{:X}", remainder);

		if (settings.overrideAutoExposure != 0)
		{
			if (settings.overrideAutoExposure >= 1) // force auto-exposure
			{
				spdlog::debug("OverrideAutoExposure: force enabling flag NVSDK_NGX_DLSS_Feature_Flags_AutoExposure");
				InValue |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
			}
			else if (settings.overrideAutoExposure < 0) // force disable auto-exposure
			{
				spdlog::debug("OverrideAutoExposure: force disabling flag NVSDK_NGX_DLSS_Feature_Flags_AutoExposure");
				InValue = InValue & ~NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
			}
		}

		if (settings.overrideSharpeningForceDisable)
		{
			if (InValue & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening)
			{
				spdlog::info("OverrideSharpening: force disabling flag NVSDK_NGX_DLSS_Feature_Flags_DoSharpening");
				InValue = InValue & ~NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
			}
		}
		else if (settings.overrideSharpening.has_value())
		{
			if ((InValue & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening) != NVSDK_NGX_DLSS_Feature_Flags_DoSharpening)
			{
				spdlog::debug("OverrideSharpening: force enabling flag NVSDK_NGX_DLSS_Feature_Flags_DoSharpening");
				InValue |= NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
			}
		}
	}

	// Cache the chosen quality value so we can make decisions on it later on
	if (!_stricmp(InName, NVSDK_NGX_Parameter_PerfQualityValue))
	{
		prevQualityValue = NVSDK_NGX_PerfQuality_Value(InValue);

		// Some games may expose an UltraQuality option if we returned a valid resolution for it
		// DLSS usually doesn't like being asked to use UltraQuality though, and will break rendering/crash altogether if set
		// So we'll just tell DLSS to use MaxQuality instead, while keeping UltraQuality stored in prevQualityValue
		if (prevQualityValue == NVSDK_NGX_PerfQuality_Value_UltraQuality)
		{
			auto& res = qualityLevelResolutions[NVSDK_NGX_PerfQuality_Value_UltraQuality];
			if (utility::ValidResolution(res) || qualityLevelRatios[NVSDK_NGX_PerfQuality_Value_UltraQuality] > 0.f)
				InValue = int(NVSDK_NGX_PerfQuality_Value_MaxQuality);
		}
	}

	NVSDK_NGX_Parameter_SetI_Hook.call(InParameter, InName, InValue);
}

SafetyHookInline NVSDK_NGX_Parameter_SetUI_Hook;
void __cdecl NVSDK_NGX_Parameter_SetUI(NVSDK_NGX_Parameter* InParameter, const char* InName, unsigned int InValue)
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
	if (settings.overrideSharpening.has_value())
		NVSDK_NGX_Parameter_SetF_Hook.call(InParameter, NVSDK_NGX_Parameter_Sharpness, *settings.overrideSharpening);

	NVSDK_NGX_Parameter_SetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Disable_Watermark, settings.disableDevWatermark ? 1 : 0);
}

SafetyHookInline NVSDK_NGX_Parameter_GetUI_Hook;
NVSDK_NGX_Result __cdecl NVSDK_NGX_Parameter_GetUI(NVSDK_NGX_Parameter* InParameter, const char* InName, unsigned int* OutValue)
{
	NVSDK_NGX_Result ret = NVSDK_NGX_Parameter_GetUI_Hook.call<NVSDK_NGX_Result>(InParameter, InName, OutValue);
	if (ret != NVSDK_NGX_Result_Success)
		return ret;

	bool isOutWidth = !_stricmp(InName, NVSDK_NGX_Parameter_OutWidth);
	bool isOutHeight = !_stricmp(InName, NVSDK_NGX_Parameter_OutHeight);

	// DLAA force by overwriting OutWidth/OutHeight with the full res
	bool overrideWidth = settings.forceDLAA && isOutWidth;
	bool overrideHeight = settings.forceDLAA && isOutHeight;
	if (overrideWidth || overrideHeight)
	{
		if (overrideWidth && *OutValue != 0)
		{
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Width, OutValue);
			*OutValue += settings.resolutionOffset;
		}
		if (overrideHeight && *OutValue != 0)
		{
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Height, OutValue);
			*OutValue += settings.resolutionOffset;
		}
	}

	// Override with DLSSQualityLevels value if user set it
	if (settings.overrideQualityLevels)
	{
		if (isOutWidth || isOutHeight)
		{
			unsigned int targetWidth = 0;
			unsigned int targetHeight = 0;
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Width, &targetWidth); // fetch full screen width
			NVSDK_NGX_Parameter_GetUI_Hook.call(InParameter, NVSDK_NGX_Parameter_Height, &targetHeight); // fetch full screen height
			
			unsigned int renderWidth = 0;
			unsigned int renderHeight = 0;
			if (qualityLevelRatios.count(prevQualityValue))
			{
				auto ratio = qualityLevelRatios[prevQualityValue];

				// calculate width/height from custom ratio
				renderWidth = unsigned int(roundf(float(targetWidth) * ratio));
				renderHeight = unsigned int(roundf(float(targetHeight) * ratio));
			}
			if (qualityLevelResolutions.count(prevQualityValue))
			{
				// if custom res is set for this level, override it with that
				auto& res = qualityLevelResolutions[prevQualityValue];
				if (utility::ValidResolution(res))
				{
					renderWidth = res.first;
					renderHeight = res.second;
				}
			}

			if (renderWidth >= targetWidth)
			{
				renderWidth = targetWidth; // DLSS can't render above the target res
				renderWidth += settings.resolutionOffset; // apply resolutionOffset compatibility hack
			}
			if (renderHeight >= targetHeight)
			{
				renderHeight = targetHeight; // DLSS can't render above the target res
				renderHeight += settings.resolutionOffset; // apply resolutionOffset compatibility hack
			}

			if (renderWidth != 0 && renderHeight != 0)
				qualityLevelResolutionsCurrent[prevQualityValue] = std::pair<int, int>(renderWidth, renderHeight);

			if (isOutWidth)
				*OutValue = renderWidth;
			if (isOutHeight)
				*OutValue = renderHeight;
		}
	}

	return ret;
}

HookOrigFn NVSDK_NGX_D3D12_AllocateParameters_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D12_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	NVSDK_NGX_Result ret = NVSDK_NGX_D3D12_AllocateParameters_Hook.call<NVSDK_NGX_Result>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_D3D12_GetCapabilityParameters_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D12_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	NVSDK_NGX_Result ret = NVSDK_NGX_D3D12_GetCapabilityParameters_Hook.call<NVSDK_NGX_Result>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_D3D12_GetParameters_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D12_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	NVSDK_NGX_Result ret = NVSDK_NGX_D3D12_GetParameters_Hook.call<NVSDK_NGX_Result>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}

HookOrigFn NVSDK_NGX_D3D11_AllocateParameters_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D11_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	NVSDK_NGX_Result ret = NVSDK_NGX_D3D11_AllocateParameters_Hook.call<NVSDK_NGX_Result>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_D3D11_GetCapabilityParameters_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D11_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	NVSDK_NGX_Result ret = NVSDK_NGX_D3D11_GetCapabilityParameters_Hook.call<NVSDK_NGX_Result>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_D3D11_GetParameters_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	NVSDK_NGX_Result ret = NVSDK_NGX_D3D11_GetParameters_Hook.call<NVSDK_NGX_Result>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}

HookOrigFn NVSDK_NGX_VULKAN_AllocateParameters_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_VULKAN_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	NVSDK_NGX_Result ret = NVSDK_NGX_VULKAN_AllocateParameters_Hook.call<NVSDK_NGX_Result>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_VULKAN_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	NVSDK_NGX_Result ret = NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook.call<NVSDK_NGX_Result>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}
HookOrigFn NVSDK_NGX_VULKAN_GetParameters_Hook;
PLUGIN_API NVSDK_NGX_Result __cdecl NVSDK_NGX_VULKAN_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	WaitForInitThread();

	NVSDK_NGX_Result ret = NVSDK_NGX_VULKAN_GetParameters_Hook.call<NVSDK_NGX_Result>(OutParameters);

	if (*OutParameters)
		hook_params(*OutParameters);
	return ret;
}

std::mutex paramHookMutex;
void hook_params(NVSDK_NGX_Parameter* params)
{
	std::scoped_lock lock{paramHookMutex};

	if (settings.disableAllTweaks)
		return;

	if (NVSDK_NGX_Parameter_SetF_Hook && NVSDK_NGX_Parameter_SetI_Hook && NVSDK_NGX_Parameter_SetUI_Hook && NVSDK_NGX_Parameter_GetUI_Hook)
		return;

	NVSDK_NGX_Parameter_vftable** vftable = (NVSDK_NGX_Parameter_vftable**)params;

	if (!vftable || !*vftable)
		return;

	auto* NVSDK_NGX_Parameter_SetF_orig = (*vftable)->SetF;
	auto* NVSDK_NGX_Parameter_SetI_orig = (*vftable)->SetI;
	auto* NVSDK_NGX_Parameter_SetUI_orig = (*vftable)->SetUI;
	auto* NVSDK_NGX_Parameter_GetUI_orig = (*vftable)->GetUI;

	if (NVSDK_NGX_Parameter_SetF_orig && NVSDK_NGX_Parameter_SetI_orig && NVSDK_NGX_Parameter_SetUI_orig && NVSDK_NGX_Parameter_GetUI_orig)
	{
		NVSDK_NGX_Parameter_SetF_Hook = safetyhook::create_inline(NVSDK_NGX_Parameter_SetF_orig, NVSDK_NGX_Parameter_SetF);
		NVSDK_NGX_Parameter_SetI_Hook = safetyhook::create_inline(NVSDK_NGX_Parameter_SetI_orig, NVSDK_NGX_Parameter_SetI);
		NVSDK_NGX_Parameter_SetUI_Hook = safetyhook::create_inline(NVSDK_NGX_Parameter_SetUI_orig, NVSDK_NGX_Parameter_SetUI);
		NVSDK_NGX_Parameter_GetUI_Hook = safetyhook::create_inline(NVSDK_NGX_Parameter_GetUI_orig, NVSDK_NGX_Parameter_GetUI);

		spdlog::info("DLSS functions found & parameter hooks applied!");
		settings.print_to_log();

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
	auto* NVSDK_NGX_D3D11_EvaluateFeature_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_EvaluateFeature");
	auto* NVSDK_NGX_D3D11_Init_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_Init");
	auto* NVSDK_NGX_D3D11_Init_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_Init_Ext");
	auto* NVSDK_NGX_D3D11_Init_ProjectID_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_Init_ProjectID");
	auto* NVSDK_NGX_D3D11_AllocateParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_AllocateParameters");
	auto* NVSDK_NGX_D3D11_GetCapabilityParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_GetCapabilityParameters");
	auto* NVSDK_NGX_D3D11_GetParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D11_GetParameters");

	auto* NVSDK_NGX_D3D12_EvaluateFeature_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_EvaluateFeature");
	auto* NVSDK_NGX_D3D12_Init_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_Init");
	auto* NVSDK_NGX_D3D12_Init_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_Init_Ext");
	auto* NVSDK_NGX_D3D12_Init_ProjectID_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_Init_ProjectID");
	auto* NVSDK_NGX_D3D12_AllocateParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_AllocateParameters");
	auto* NVSDK_NGX_D3D12_GetCapabilityParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_GetCapabilityParameters");
	auto* NVSDK_NGX_D3D12_GetParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_D3D12_GetParameters");

	auto* NVSDK_NGX_VULKAN_EvaluateFeature_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_EvaluateFeature");
	auto* NVSDK_NGX_VULKAN_Init_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init");
	auto* NVSDK_NGX_VULKAN_Init_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_Ext");
	auto* NVSDK_NGX_VULKAN_Init_Ext2_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_Ext2");
	auto* NVSDK_NGX_VULKAN_Init_ProjectID_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_ProjectID");
	auto* NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_Init_ProjectID_Ext");
	auto* NVSDK_NGX_VULKAN_AllocateParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_AllocateParameters");
	auto* NVSDK_NGX_VULKAN_GetCapabilityParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_GetCapabilityParameters");
	auto* NVSDK_NGX_VULKAN_GetParameters_orig = GetProcAddress(ngx_module, "NVSDK_NGX_VULKAN_GetParameters");

	// Make sure we only try hooking if we found all the procs above...
	if (NVSDK_NGX_D3D11_EvaluateFeature_orig && NVSDK_NGX_D3D11_Init_orig && NVSDK_NGX_D3D11_Init_Ext_orig && NVSDK_NGX_D3D11_Init_ProjectID_orig &&
		NVSDK_NGX_D3D11_AllocateParameters_orig && NVSDK_NGX_D3D11_GetCapabilityParameters_orig && NVSDK_NGX_D3D11_GetParameters_orig &&
		NVSDK_NGX_D3D12_EvaluateFeature_orig && NVSDK_NGX_D3D12_Init_orig && NVSDK_NGX_D3D12_Init_Ext_orig && NVSDK_NGX_D3D12_Init_ProjectID_orig &&
		NVSDK_NGX_D3D12_AllocateParameters_orig && NVSDK_NGX_D3D12_GetCapabilityParameters_orig && NVSDK_NGX_D3D12_GetParameters_orig &&
		NVSDK_NGX_VULKAN_EvaluateFeature_orig && NVSDK_NGX_VULKAN_Init_orig && NVSDK_NGX_VULKAN_Init_Ext_orig && NVSDK_NGX_VULKAN_Init_ProjectID_orig &&
		NVSDK_NGX_VULKAN_AllocateParameters_orig && NVSDK_NGX_VULKAN_GetCapabilityParameters_orig && NVSDK_NGX_VULKAN_GetParameters_orig)
	{
		NVSDK_NGX_D3D11_EvaluateFeature_Hook = safetyhook::create_inline(NVSDK_NGX_D3D11_EvaluateFeature_orig, NVSDK_NGX_D3D11_EvaluateFeature);
		NVSDK_NGX_D3D11_Init_Hook = safetyhook::create_inline(NVSDK_NGX_D3D11_Init_orig, NVSDK_NGX_D3D11_Init);
		NVSDK_NGX_D3D11_Init_Ext_Hook = safetyhook::create_inline(NVSDK_NGX_D3D11_Init_Ext_orig, NVSDK_NGX_D3D11_Init_Ext);
		NVSDK_NGX_D3D11_Init_ProjectID_Hook = safetyhook::create_inline(NVSDK_NGX_D3D11_Init_ProjectID_orig, NVSDK_NGX_D3D11_Init_ProjectID);

		NVSDK_NGX_D3D11_AllocateParameters_Hook = safetyhook::create_inline(NVSDK_NGX_D3D11_AllocateParameters_orig, NVSDK_NGX_D3D11_AllocateParameters);
		NVSDK_NGX_D3D11_GetCapabilityParameters_Hook = safetyhook::create_inline(NVSDK_NGX_D3D11_GetCapabilityParameters_orig, NVSDK_NGX_D3D11_GetCapabilityParameters);
		NVSDK_NGX_D3D11_GetParameters_Hook = safetyhook::create_inline(NVSDK_NGX_D3D11_GetParameters_orig, NVSDK_NGX_D3D11_GetParameters);

		NVSDK_NGX_D3D12_EvaluateFeature_Hook = safetyhook::create_inline(NVSDK_NGX_D3D12_EvaluateFeature_orig, NVSDK_NGX_D3D12_EvaluateFeature);
		NVSDK_NGX_D3D12_Init_Hook = safetyhook::create_inline(NVSDK_NGX_D3D12_Init_orig, NVSDK_NGX_D3D12_Init);
		NVSDK_NGX_D3D12_Init_Ext_Hook = safetyhook::create_inline(NVSDK_NGX_D3D12_Init_Ext_orig, NVSDK_NGX_D3D12_Init_Ext);
		NVSDK_NGX_D3D12_Init_ProjectID_Hook = safetyhook::create_inline(NVSDK_NGX_D3D12_Init_ProjectID_orig, NVSDK_NGX_D3D12_Init_ProjectID);

		NVSDK_NGX_D3D12_AllocateParameters_Hook = safetyhook::create_inline(NVSDK_NGX_D3D12_AllocateParameters_orig, NVSDK_NGX_D3D12_AllocateParameters);
		NVSDK_NGX_D3D12_GetCapabilityParameters_Hook = safetyhook::create_inline(NVSDK_NGX_D3D12_GetCapabilityParameters_orig, NVSDK_NGX_D3D12_GetCapabilityParameters);
		NVSDK_NGX_D3D12_GetParameters_Hook = safetyhook::create_inline(NVSDK_NGX_D3D12_GetParameters_orig, NVSDK_NGX_D3D12_GetParameters);

		NVSDK_NGX_VULKAN_EvaluateFeature_Hook = safetyhook::create_inline(NVSDK_NGX_VULKAN_EvaluateFeature_orig, NVSDK_NGX_VULKAN_EvaluateFeature);
		NVSDK_NGX_VULKAN_Init_Hook = safetyhook::create_inline(NVSDK_NGX_VULKAN_Init_orig, NVSDK_NGX_VULKAN_Init);
		NVSDK_NGX_VULKAN_Init_Ext_Hook = safetyhook::create_inline(NVSDK_NGX_VULKAN_Init_Ext_orig, NVSDK_NGX_VULKAN_Init_Ext);
		NVSDK_NGX_VULKAN_Init_ProjectID_Hook = safetyhook::create_inline(NVSDK_NGX_VULKAN_Init_ProjectID_orig, NVSDK_NGX_VULKAN_Init_ProjectID);

		// Only in later drivers
		if (NVSDK_NGX_VULKAN_Init_Ext2_orig)
			NVSDK_NGX_VULKAN_Init_Ext2_Hook = safetyhook::create_inline(NVSDK_NGX_VULKAN_Init_Ext2_orig, NVSDK_NGX_VULKAN_Init_Ext2);
		if (NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig)
			NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook = safetyhook::create_inline(NVSDK_NGX_VULKAN_Init_ProjectID_Ext_orig, NVSDK_NGX_VULKAN_Init_ProjectID_Ext);

		NVSDK_NGX_VULKAN_AllocateParameters_Hook = safetyhook::create_inline(NVSDK_NGX_VULKAN_AllocateParameters_orig, NVSDK_NGX_VULKAN_AllocateParameters);
		NVSDK_NGX_VULKAN_GetCapabilityParameters_Hook = safetyhook::create_inline(NVSDK_NGX_VULKAN_GetCapabilityParameters_orig, NVSDK_NGX_VULKAN_GetCapabilityParameters);
		NVSDK_NGX_VULKAN_GetParameters_Hook = safetyhook::create_inline(NVSDK_NGX_VULKAN_GetParameters_orig, NVSDK_NGX_VULKAN_GetParameters);

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

	NVSDK_NGX_D3D11_EvaluateFeature_Hook.reset();
	NVSDK_NGX_D3D11_Init_Hook.reset();
	NVSDK_NGX_D3D11_Init_Ext_Hook.reset();
	NVSDK_NGX_D3D11_Init_ProjectID_Hook.reset();
	NVSDK_NGX_D3D12_EvaluateFeature_Hook.reset();
	NVSDK_NGX_D3D12_Init_Hook.reset();
	NVSDK_NGX_D3D12_Init_Ext_Hook.reset();
	NVSDK_NGX_D3D12_Init_ProjectID_Hook.reset();
	NVSDK_NGX_VULKAN_EvaluateFeature_Hook.reset();
	NVSDK_NGX_VULKAN_Init_Hook.reset();
	NVSDK_NGX_VULKAN_Init_Ext_Hook.reset();
	NVSDK_NGX_VULKAN_Init_Ext2_Hook.reset();
	NVSDK_NGX_VULKAN_Init_ProjectID_Hook.reset();
	NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Hook.reset();
	NVSDK_NGX_Parameter_SetF_Hook.reset();
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
	NVSDK_NGX_D3D11_EvaluateFeature_Hook = NVSDK_NGX_D3D11_EvaluateFeature_Orig;
	NVSDK_NGX_D3D11_Init_Hook = NVSDK_NGX_D3D11_Init_Orig;
	NVSDK_NGX_D3D11_Init_Ext_Hook = NVSDK_NGX_D3D11_Init_Ext_Orig;
	NVSDK_NGX_D3D11_Init_ProjectID_Hook = NVSDK_NGX_D3D11_Init_ProjectID_Orig;
	NVSDK_NGX_D3D12_EvaluateFeature_Hook = NVSDK_NGX_D3D12_EvaluateFeature_Orig;
	NVSDK_NGX_D3D12_Init_Hook = NVSDK_NGX_D3D12_Init_Orig;
	NVSDK_NGX_D3D12_Init_Ext_Hook = NVSDK_NGX_D3D12_Init_Ext_Orig;
	NVSDK_NGX_D3D12_Init_ProjectID_Hook = NVSDK_NGX_D3D12_Init_ProjectID_Orig;
	NVSDK_NGX_VULKAN_EvaluateFeature_Hook = NVSDK_NGX_VULKAN_EvaluateFeature_Orig;
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
	if (proxy::is_wrapping_nvngx || settings.disableAllTweaks)
		return;

	// aren't wrapping nvngx, apply hooks to module
	hook(ngx_module);
	dllmain = safetyhook::create_inline(utility::ModuleEntryPoint(ngx_module), hooked_dllmain);
}
};
