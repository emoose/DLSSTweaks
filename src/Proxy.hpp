#pragma once

// Pointers to functions inside original nvngx.dll
extern FARPROC NVSDK_NGX_D3D11_Init_Orig;
extern FARPROC NVSDK_NGX_D3D11_Init_Ext_Orig;
extern FARPROC NVSDK_NGX_D3D11_Init_ProjectID_Orig;
extern FARPROC NVSDK_NGX_D3D12_Init_Orig;
extern FARPROC NVSDK_NGX_D3D12_Init_Ext_Orig;
extern FARPROC NVSDK_NGX_D3D12_Init_ProjectID_Orig;
extern FARPROC NVSDK_NGX_VULKAN_Init_Orig;
extern FARPROC NVSDK_NGX_VULKAN_Init_Ext_Orig;
extern FARPROC NVSDK_NGX_VULKAN_Init_ProjectID_Orig;
extern FARPROC NVSDK_NGX_D3D12_AllocateParameters_Orig;
extern FARPROC NVSDK_NGX_D3D12_GetCapabilityParameters_Orig;
extern FARPROC NVSDK_NGX_D3D12_GetParameters_Orig;
extern FARPROC NVSDK_NGX_D3D11_AllocateParameters_Orig;
extern FARPROC NVSDK_NGX_D3D11_GetCapabilityParameters_Orig;
extern FARPROC NVSDK_NGX_D3D11_GetParameters_Orig;
extern FARPROC NVSDK_NGX_VULKAN_AllocateParameters_Orig;
extern FARPROC NVSDK_NGX_VULKAN_GetCapabilityParameters_Orig;
extern FARPROC NVSDK_NGX_VULKAN_GetParameters_Orig;

namespace proxy
{
bool on_attach(HMODULE ourModule);
void on_detach();
};

#define PLUGIN_API extern "C" __declspec(dllexport)
