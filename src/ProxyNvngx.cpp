#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winternl.h>

#include <filesystem>
#include "Proxy.hpp"

// nvngx.dll
FARPROC NVSDK_NGX_CUDA_AllocateParameters_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_CreateFeature_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_DestroyParameters_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_EvaluateFeature_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_GetCapabilityParameters_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_GetParameters_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_GetScratchBufferSize_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_Init_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_Init_Ext_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_Init_ProjectID_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_ReleaseFeature_Orig = nullptr;
FARPROC NVSDK_NGX_CUDA_Shutdown_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_AllocateParameters_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_CreateFeature_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_DestroyParameters_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_EvaluateFeature_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_GetCapabilityParameters_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_GetFeatureRequirements_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_GetParameters_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_GetScratchBufferSize_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_Init_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_Init_Ext_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_Init_ProjectID_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_ReleaseFeature_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_Shutdown_Orig = nullptr;
FARPROC NVSDK_NGX_D3D11_Shutdown1_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_AllocateParameters_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_CreateFeature_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_DestroyParameters_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_EvaluateFeature_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_GetCapabilityParameters_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_GetFeatureRequirements_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_GetParameters_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_GetScratchBufferSize_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_Init_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_Init_Ext_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_Init_ProjectID_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_ReleaseFeature_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_Shutdown_Orig = nullptr;
FARPROC NVSDK_NGX_D3D12_Shutdown1_Orig = nullptr;
FARPROC NVSDK_NGX_OTA_UPDATES_CheckForUpdate_Orig = nullptr;
FARPROC NVSDK_NGX_OTA_UPDATES_GetPath_Orig = nullptr;
FARPROC NVSDK_NGX_OTA_UPDATES_Install_Orig = nullptr;
FARPROC NVSDK_NGX_OTA_UPDATES_Register_Orig = nullptr;
FARPROC NVSDK_NGX_OTA_UPDATES_Unregister_Orig = nullptr;
FARPROC NVSDK_NGX_OTA_UPDATES_Update_Orig = nullptr;
FARPROC NVSDK_NGX_UpdateFeature_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_AllocateParameters_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_CreateFeature_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_CreateFeature1_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_DestroyParameters_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_EvaluateFeature_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_GetCapabilityParameters_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_GetFeatureRequirements_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_GetParameters_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_GetScratchBufferSize_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_Init_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_Init_Ext_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_Init_Ext2_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_Init_ProjectID_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_ReleaseFeature_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_RequiredExtensions_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_Shutdown_Orig = nullptr;
FARPROC NVSDK_NGX_VULKAN_Shutdown1_Orig = nullptr;

PLUGIN_API void NVSDK_NGX_CUDA_AllocateParameters()
{
    NVSDK_NGX_CUDA_AllocateParameters_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_CreateFeature()
{
    NVSDK_NGX_CUDA_CreateFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_DestroyParameters()
{
    NVSDK_NGX_CUDA_DestroyParameters_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_EvaluateFeature()
{
    NVSDK_NGX_CUDA_EvaluateFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_GetCapabilityParameters()
{
    NVSDK_NGX_CUDA_GetCapabilityParameters_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_GetParameters()
{
    NVSDK_NGX_CUDA_GetParameters_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_GetScratchBufferSize()
{
    NVSDK_NGX_CUDA_GetScratchBufferSize_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_Init()
{
    NVSDK_NGX_CUDA_Init_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_Init_Ext()
{
    NVSDK_NGX_CUDA_Init_Ext_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_Init_ProjectID()
{
    NVSDK_NGX_CUDA_Init_ProjectID_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_ReleaseFeature()
{
    NVSDK_NGX_CUDA_ReleaseFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_CUDA_Shutdown()
{
    NVSDK_NGX_CUDA_Shutdown_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D11_CreateFeature()
{
    NVSDK_NGX_D3D11_CreateFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D11_DestroyParameters()
{
    NVSDK_NGX_D3D11_DestroyParameters_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D11_GetFeatureRequirements()
{
    NVSDK_NGX_D3D11_GetFeatureRequirements_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D11_GetScratchBufferSize()
{
    NVSDK_NGX_D3D11_GetScratchBufferSize_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D11_ReleaseFeature()
{
    NVSDK_NGX_D3D11_ReleaseFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D11_Shutdown()
{
    NVSDK_NGX_D3D11_Shutdown_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D11_Shutdown1()
{
    NVSDK_NGX_D3D11_Shutdown1_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D12_CreateFeature()
{
    NVSDK_NGX_D3D12_CreateFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D12_DestroyParameters()
{
    NVSDK_NGX_D3D12_DestroyParameters_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D12_GetFeatureRequirements()
{
    NVSDK_NGX_D3D12_GetFeatureRequirements_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D12_GetScratchBufferSize()
{
    NVSDK_NGX_D3D12_GetScratchBufferSize_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D12_ReleaseFeature()
{
    NVSDK_NGX_D3D12_ReleaseFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D12_Shutdown()
{
    NVSDK_NGX_D3D12_Shutdown_Orig();
}

PLUGIN_API void NVSDK_NGX_D3D12_Shutdown1()
{
    NVSDK_NGX_D3D12_Shutdown1_Orig();
}

PLUGIN_API void NVSDK_NGX_OTA_UPDATES_CheckForUpdate()
{
    NVSDK_NGX_OTA_UPDATES_CheckForUpdate_Orig();
}

PLUGIN_API void NVSDK_NGX_OTA_UPDATES_GetPath()
{
    NVSDK_NGX_OTA_UPDATES_GetPath_Orig();
}

PLUGIN_API void NVSDK_NGX_OTA_UPDATES_Install()
{
    NVSDK_NGX_OTA_UPDATES_Install_Orig();
}

PLUGIN_API void NVSDK_NGX_OTA_UPDATES_Register()
{
    NVSDK_NGX_OTA_UPDATES_Register_Orig();
}

PLUGIN_API void NVSDK_NGX_OTA_UPDATES_Unregister()
{
    NVSDK_NGX_OTA_UPDATES_Unregister_Orig();
}

PLUGIN_API void NVSDK_NGX_OTA_UPDATES_Update()
{
    NVSDK_NGX_OTA_UPDATES_Update_Orig();
}

PLUGIN_API void NVSDK_NGX_UpdateFeature()
{
    NVSDK_NGX_UpdateFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_CreateFeature()
{
    NVSDK_NGX_VULKAN_CreateFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_CreateFeature1()
{
    NVSDK_NGX_VULKAN_CreateFeature1_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_DestroyParameters()
{
    NVSDK_NGX_VULKAN_DestroyParameters_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements()
{
    NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements()
{
    NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_GetFeatureRequirements()
{
    NVSDK_NGX_VULKAN_GetFeatureRequirements_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_GetScratchBufferSize()
{
    NVSDK_NGX_VULKAN_GetScratchBufferSize_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_ReleaseFeature()
{
    NVSDK_NGX_VULKAN_ReleaseFeature_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_RequiredExtensions()
{
    NVSDK_NGX_VULKAN_RequiredExtensions_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_Shutdown()
{
    NVSDK_NGX_VULKAN_Shutdown_Orig();
}

PLUGIN_API void NVSDK_NGX_VULKAN_Shutdown1()
{
    NVSDK_NGX_VULKAN_Shutdown1_Orig();
}

namespace proxy_nvngx
{
bool on_attach(HMODULE ourModule)
{
    // Find path of original nvngx via NGXCore registry entry, same way that DLSS SDK seems to do it

    HKEY ngxCoreKey;
    LSTATUS status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\nvlddmkm\\NGXCore", 0, 0x20019u, &ngxCoreKey);
    if (status != ERROR_SUCCESS)
        return false;

    wchar_t ngxCorePath[0x104];
    DWORD ngxCorePathSize = 0x104;
    status = RegQueryValueExW(ngxCoreKey, L"NGXPath", nullptr, nullptr, (LPBYTE)ngxCorePath, &ngxCorePathSize);
    if (status != ERROR_SUCCESS)
        return false;

    
    auto nvngxModulePath = (std::filesystem::path(ngxCorePath) / "_nvngx.dll").wstring();
    proxy::origModule = LoadLibraryW(nvngxModulePath.c_str());
    if (!proxy::origModule)
    {
        nvngxModulePath = (std::filesystem::path(ngxCorePath) / "nvngx.dll").wstring();
        proxy::origModule = LoadLibraryW(nvngxModulePath.c_str());
        if (!proxy::origModule)
            return false;
    }

    NVSDK_NGX_CUDA_AllocateParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_AllocateParameters");
    NVSDK_NGX_CUDA_CreateFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_CreateFeature");
    NVSDK_NGX_CUDA_DestroyParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_DestroyParameters");
    NVSDK_NGX_CUDA_EvaluateFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_EvaluateFeature");
    NVSDK_NGX_CUDA_GetCapabilityParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_GetCapabilityParameters");
    NVSDK_NGX_CUDA_GetParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_GetParameters");
    NVSDK_NGX_CUDA_GetScratchBufferSize_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_GetScratchBufferSize");
    NVSDK_NGX_CUDA_Init_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_Init");
    NVSDK_NGX_CUDA_Init_Ext_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_Init_Ext");
    NVSDK_NGX_CUDA_Init_ProjectID_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_Init_ProjectID");
    NVSDK_NGX_CUDA_ReleaseFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_ReleaseFeature");
    NVSDK_NGX_CUDA_Shutdown_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_CUDA_Shutdown");
    NVSDK_NGX_D3D11_AllocateParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_AllocateParameters");
    NVSDK_NGX_D3D11_CreateFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_CreateFeature");
    NVSDK_NGX_D3D11_DestroyParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_DestroyParameters");
    NVSDK_NGX_D3D11_EvaluateFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_EvaluateFeature");
    NVSDK_NGX_D3D11_GetCapabilityParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_GetCapabilityParameters");
    NVSDK_NGX_D3D11_GetFeatureRequirements_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_GetFeatureRequirements");
    NVSDK_NGX_D3D11_GetParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_GetParameters");
    NVSDK_NGX_D3D11_GetScratchBufferSize_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_GetScratchBufferSize");
    NVSDK_NGX_D3D11_Init_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_Init");
    NVSDK_NGX_D3D11_Init_Ext_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_Init_Ext");
    NVSDK_NGX_D3D11_Init_ProjectID_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_Init_ProjectID");
    NVSDK_NGX_D3D11_ReleaseFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_ReleaseFeature");
    NVSDK_NGX_D3D11_Shutdown_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_Shutdown");
    NVSDK_NGX_D3D11_Shutdown1_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D11_Shutdown1");
    NVSDK_NGX_D3D12_AllocateParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_AllocateParameters");
    NVSDK_NGX_D3D12_CreateFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_CreateFeature");
    NVSDK_NGX_D3D12_DestroyParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_DestroyParameters");
    NVSDK_NGX_D3D12_EvaluateFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_EvaluateFeature");
    NVSDK_NGX_D3D12_GetCapabilityParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_GetCapabilityParameters");
    NVSDK_NGX_D3D12_GetFeatureRequirements_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_GetFeatureRequirements");
    NVSDK_NGX_D3D12_GetParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_GetParameters");
    NVSDK_NGX_D3D12_GetScratchBufferSize_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_GetScratchBufferSize");
    NVSDK_NGX_D3D12_Init_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_Init");
    NVSDK_NGX_D3D12_Init_Ext_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_Init_Ext");
    NVSDK_NGX_D3D12_Init_ProjectID_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_Init_ProjectID");
    NVSDK_NGX_D3D12_ReleaseFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_ReleaseFeature");
    NVSDK_NGX_D3D12_Shutdown_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_Shutdown");
    NVSDK_NGX_D3D12_Shutdown1_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_D3D12_Shutdown1");
    NVSDK_NGX_OTA_UPDATES_CheckForUpdate_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_OTA_UPDATES_CheckForUpdate");
    NVSDK_NGX_OTA_UPDATES_GetPath_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_OTA_UPDATES_GetPath");
    NVSDK_NGX_OTA_UPDATES_Install_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_OTA_UPDATES_Install");
    NVSDK_NGX_OTA_UPDATES_Register_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_OTA_UPDATES_Register");
    NVSDK_NGX_OTA_UPDATES_Unregister_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_OTA_UPDATES_Unregister");
    NVSDK_NGX_OTA_UPDATES_Update_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_OTA_UPDATES_Update");
    NVSDK_NGX_UpdateFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_UpdateFeature");
    NVSDK_NGX_VULKAN_AllocateParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_AllocateParameters");
    NVSDK_NGX_VULKAN_CreateFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_CreateFeature");
    NVSDK_NGX_VULKAN_CreateFeature1_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_CreateFeature1");
    NVSDK_NGX_VULKAN_DestroyParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_DestroyParameters");
    NVSDK_NGX_VULKAN_EvaluateFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_EvaluateFeature");
    NVSDK_NGX_VULKAN_GetCapabilityParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_GetCapabilityParameters");
    NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements");
    NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements");
    NVSDK_NGX_VULKAN_GetFeatureRequirements_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_GetFeatureRequirements");
    NVSDK_NGX_VULKAN_GetParameters_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_GetParameters");
    NVSDK_NGX_VULKAN_GetScratchBufferSize_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_GetScratchBufferSize");
    NVSDK_NGX_VULKAN_Init_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_Init");
    NVSDK_NGX_VULKAN_Init_Ext_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_Init_Ext");
    NVSDK_NGX_VULKAN_Init_Ext2_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_Init_Ext2");
    NVSDK_NGX_VULKAN_Init_ProjectID_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_Init_ProjectID");
    NVSDK_NGX_VULKAN_Init_ProjectID_Ext_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_Init_ProjectID_Ext");
    NVSDK_NGX_VULKAN_ReleaseFeature_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_ReleaseFeature");
    NVSDK_NGX_VULKAN_RequiredExtensions_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_RequiredExtensions");
    NVSDK_NGX_VULKAN_Shutdown_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_Shutdown");
    NVSDK_NGX_VULKAN_Shutdown1_Orig = GetProcAddress(proxy::origModule, "NVSDK_NGX_VULKAN_Shutdown1");

    return true;
}

void on_detach()
{
    if (!proxy::origModule)
        return;

    FreeLibrary(proxy::origModule);
    proxy::origModule = nullptr;
}

};

