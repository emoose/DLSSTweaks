#pragma once

namespace utility
{
BOOL HookIAT(HMODULE callerModule, char const* targetModule, void* targetFunction, void* detourFunction);
inline void* ModuleEntryPoint(HMODULE hmod)
{
	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hmod;
	PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((PBYTE)hmod + dos_header->e_lfanew);
	return (PBYTE)hmod + nt_headers->OptionalHeader.AddressOfEntryPoint;
}
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

enum NVSDK_NGX_PerfQuality_Value
{
	NVSDK_NGX_PerfQuality_Value_MaxPerf,
	NVSDK_NGX_PerfQuality_Value_Balanced,
	NVSDK_NGX_PerfQuality_Value_MaxQuality,
	NVSDK_NGX_PerfQuality_Value_UltraPerformance,
	NVSDK_NGX_PerfQuality_Value_UltraQuality,
};

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

namespace nvngx
{
	void hook_params(NVSDK_NGX_Parameter* params);
}

// defs below from chromium: https://source.chromium.org/chromium/chromium/src/+/main:chrome/common/conflicts/module_watcher_win.cc

// These structures and functions are documented in MSDN, see
// http://msdn.microsoft.com/en-us/library/gg547638(v=vs.85).aspx
// there are however no headers or import libraries available in the
// Platform SDK. They are declared outside of the anonymous namespace to
// allow them to be forward declared in the header file.
enum {
	// The DLL was loaded. The NotificationData parameter points to a
	// LDR_DLL_LOADED_NOTIFICATION_DATA structure.
	LDR_DLL_NOTIFICATION_REASON_LOADED = 1,
	// The DLL was unloaded. The NotificationData parameter points to a
	// LDR_DLL_UNLOADED_NOTIFICATION_DATA structure.
	LDR_DLL_NOTIFICATION_REASON_UNLOADED = 2,
};

// Structure that is used for module load notifications.
struct LDR_DLL_LOADED_NOTIFICATION_DATA {
	// Reserved.
	ULONG Flags;
	// The full path name of the DLL module.
	PCUNICODE_STRING FullDllName;
	// The base file name of the DLL module.
	PCUNICODE_STRING BaseDllName;
	// A pointer to the base address for the DLL in memory.
	PVOID DllBase;
	// The size of the DLL image, in bytes.
	ULONG SizeOfImage;
};
using PLDR_DLL_LOADED_NOTIFICATION_DATA = LDR_DLL_LOADED_NOTIFICATION_DATA*;

// Structure that is used for module unload notifications.
struct LDR_DLL_UNLOADED_NOTIFICATION_DATA {
	// Reserved.
	ULONG Flags;
	// The full path name of the DLL module.
	PCUNICODE_STRING FullDllName;
	// The base file name of the DLL module.
	PCUNICODE_STRING BaseDllName;
	// A pointer to the base address for the DLL in memory.
	PVOID DllBase;
	// The size of the DLL image, in bytes.
	ULONG SizeOfImage;
};
using PLDR_DLL_UNLOADED_NOTIFICATION_DATA = LDR_DLL_UNLOADED_NOTIFICATION_DATA*;

// Union that is used for notifications.
union LDR_DLL_NOTIFICATION_DATA {
	LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
	LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
};
using PLDR_DLL_NOTIFICATION_DATA = LDR_DLL_NOTIFICATION_DATA*;

// Signature of the notification callback function.
using PLDR_DLL_NOTIFICATION_FUNCTION =
VOID(CALLBACK*)(ULONG notification_reason,
	const LDR_DLL_NOTIFICATION_DATA* notification_data,
	PVOID context);

// Signatures of the functions used for registering DLL notification callbacks.
using LdrRegisterDllNotificationFunc =
NTSTATUS(NTAPI*)(ULONG flags,
	PLDR_DLL_NOTIFICATION_FUNCTION notification_function,
	PVOID context,
	PVOID* cookie);
using LdrUnregisterDllNotificationFunc = NTSTATUS(NTAPI*)(PVOID cookie);
