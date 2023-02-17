#pragma once

namespace utility
{
BOOL HookIAT(HMODULE callerModule, char const* targetModule, void* targetFunction, void* detourFunction);
};

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
#define NVSDK_NGX_Parameter_Disable_Watermark "Disable.Watermark"

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
