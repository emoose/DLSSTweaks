#pragma once

namespace proxy
{
extern HMODULE origModule;
extern bool is_wrapping_nvngx;
bool on_attach(HMODULE ourModule);
void on_detach();
};

// externs from ProxyNvngx.cpp
extern FARPROC NVSDK_NGX_D3D11_EvaluateFeature_Orig;
extern FARPROC NVSDK_NGX_D3D11_Init_Orig;
extern FARPROC NVSDK_NGX_D3D11_Init_Ext_Orig;
extern FARPROC NVSDK_NGX_D3D11_Init_ProjectID_Orig;
extern FARPROC NVSDK_NGX_D3D12_EvaluateFeature_Orig;
extern FARPROC NVSDK_NGX_D3D12_Init_Orig;
extern FARPROC NVSDK_NGX_D3D12_Init_Ext_Orig;
extern FARPROC NVSDK_NGX_D3D12_Init_ProjectID_Orig;
extern FARPROC NVSDK_NGX_VULKAN_EvaluateFeature_Orig;
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

namespace proxy_nvngx
{
bool on_attach(HMODULE ourModule);
void on_detach();
};

#define PLUGIN_API extern "C" __declspec(dllexport)
