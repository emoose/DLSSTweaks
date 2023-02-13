#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>

#define PLUGIN_API extern "C" __declspec(dllexport)

// XAPOFX1_5.dll
typedef DWORD(WINAPI* CreateFX_ptr)(REFCLSID clsid, void* pEffect);

CreateFX_ptr CreateFX_orig;

PLUGIN_API DWORD WINAPI CreateFX(REFCLSID clsid, void* pEffect)
{
	return CreateFX_orig(clsid, pEffect);
}

// X3DAudio1_7
typedef DWORD(WINAPI* X3DAudioInitialize_ptr)(UINT32 SpeakerChannelMask, float SpeedOfSound, void* Instance);
typedef DWORD(WINAPI* X3DAudioCalculate_ptr)(void* Instance, void* pListener, void* pEmitter, UINT32 Flags, void* pDSPSettings);

X3DAudioInitialize_ptr X3DAudioInitialize_orig;
X3DAudioCalculate_ptr X3DAudioCalculate_orig;

PLUGIN_API DWORD WINAPI X3DAudioInitialize(UINT32 SpeakerChannelMask, float SpeedOfSound, void* Instance)
{
	return X3DAudioInitialize_orig(SpeakerChannelMask, SpeedOfSound, Instance);
}

PLUGIN_API DWORD WINAPI X3DAudioCalculate(void* Instance, void* pListener, void* pEmitter, UINT32 Flags, void* pDSPSettings)
{
	return X3DAudioCalculate_orig(Instance, pListener, pEmitter, Flags, pDSPSettings);
}

// XInput1_3 and XInput9_1_0
typedef DWORD(WINAPI* XInputGetState_ptr)(DWORD dwUserIndex, void* pState);
typedef DWORD(WINAPI* XInputSetState_ptr)(DWORD dwUserIndex, void* pVibration);
typedef DWORD(WINAPI* XInputGetCapabilities_ptr)(DWORD dwUserIndex, DWORD dwFlags, void* pCapabilities);
typedef void(WINAPI* XInputEnable_ptr)(BOOL enable);
typedef DWORD(WINAPI* XInputGetDSoundAudioDeviceGuids_ptr)(DWORD dwUserIndex, GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid);
typedef DWORD(WINAPI* XInputGetBatteryInformation_ptr)(DWORD dwUserIndex, BYTE devType, void* pBatteryInformation);
typedef DWORD(WINAPI* XInputGetKeystroke_ptr)(DWORD dwUserIndex, DWORD dwReserved, void* pKeystroke);

XInputGetState_ptr XInputGetState_orig;
XInputSetState_ptr XInputSetState_orig;
XInputGetCapabilities_ptr XInputGetCapabilities_orig;
XInputEnable_ptr XInputEnable_orig;
XInputGetDSoundAudioDeviceGuids_ptr XInputGetDSoundAudioDeviceGuids_orig;
XInputGetBatteryInformation_ptr XInputGetBatteryInformation_orig;
XInputGetKeystroke_ptr XInputGetKeystroke_orig;

// xinput1_3 needs everything at the proper ordinal, proxy.def handles that, but we need something for ordinal 1 so:
PLUGIN_API void DllMain_stub()
{

}

PLUGIN_API DWORD WINAPI XInputGetState(DWORD dwUserIndex, void* pState)
{
	return XInputGetState_orig(dwUserIndex, pState);
}

PLUGIN_API DWORD WINAPI XInputSetState(DWORD dwUserIndex, void* pVibration)
{
	return XInputSetState_orig(dwUserIndex, pVibration);
}

PLUGIN_API DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, void* pCapabilities)
{
	return XInputGetCapabilities_orig(dwUserIndex, dwFlags, pCapabilities);
}

PLUGIN_API void WINAPI XInputEnable(BOOL enable)
{
	XInputEnable_orig(enable);
}

PLUGIN_API DWORD WINAPI XInputGetDSoundAudioDeviceGuids(DWORD dwUserIndex, GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid)
{
	return XInputGetDSoundAudioDeviceGuids_orig(dwUserIndex, pDSoundRenderGuid, pDSoundCaptureGuid);
}

PLUGIN_API DWORD WINAPI XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType, void* pBatteryInformation)
{
	return XInputGetBatteryInformation_orig(dwUserIndex, devType, pBatteryInformation);
}

PLUGIN_API DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved, void* pKeystroke)
{
	return XInputGetKeystroke_orig(dwUserIndex, dwReserved, pKeystroke);
}

//dinput8.dll
typedef HRESULT(WINAPI* DirectInput8Create_ptr)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, void* punkOuter);

DirectInput8Create_ptr DirectInput8Create_orig;

PLUGIN_API HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, void* punkOuter)
{
	return DirectInput8Create_orig(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

//dxgi.dll
struct UNKNOWN {
	BYTE unknown[20];
};

typedef HRESULT(WINAPI* DXGIDumpJournal_ptr)(void* unk);
typedef HRESULT(WINAPI* CreateDXGIFactory_ptr)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* CreateDXGIFactory1_ptr)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* CreateDXGIFactory2_ptr)(UINT Flags, REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* DXGID3D10CreateDevice_ptr)(HMODULE hModule, void* pFactory, void* pAdapter, UINT Flags, void* unknown, void* ppDevice);
typedef HRESULT(WINAPI* DXGID3D10CreateLayeredDevice_ptr)(UNKNOWN unk);
typedef size_t(WINAPI* DXGID3D10GetLayeredDeviceSize_ptr)(const void* pLayers, UINT NumLayers);
typedef HRESULT(WINAPI* DXGID3D10RegisterLayers_ptr)(const void* pLayers, UINT NumLayers);
typedef HRESULT(WINAPI* DXGIGetDebugInterface1_ptr)(UINT Flags, REFIID riid, void** pDebug);
typedef HRESULT(WINAPI* DXGIReportAdapterConfiguration_ptr)(DWORD unk);

DXGIDumpJournal_ptr DXGIDumpJournal_orig;
CreateDXGIFactory_ptr CreateDXGIFactory_orig;
CreateDXGIFactory1_ptr CreateDXGIFactory1_orig;
CreateDXGIFactory2_ptr CreateDXGIFactory2_orig;
DXGID3D10CreateDevice_ptr DXGID3D10CreateDevice_orig;
DXGID3D10CreateLayeredDevice_ptr DXGID3D10CreateLayeredDevice_orig;
DXGID3D10GetLayeredDeviceSize_ptr DXGID3D10GetLayeredDeviceSize_orig;
DXGID3D10RegisterLayers_ptr DXGID3D10RegisterLayers_orig;
DXGIGetDebugInterface1_ptr DXGIGetDebugInterface1_orig;
DXGIReportAdapterConfiguration_ptr DXGIReportAdapterConfiguration_orig;

PLUGIN_API HRESULT WINAPI DXGIDumpJournal(void* unk)
{
	return DXGIDumpJournal_orig(unk);
}

PLUGIN_API HRESULT WINAPI CreateDXGIFactory(REFIID riid, _Out_ void** ppFactory)
{
	return CreateDXGIFactory_orig(riid, ppFactory);
}

PLUGIN_API HRESULT WINAPI CreateDXGIFactory1(REFIID riid, _Out_ void** ppFactory)
{
	return CreateDXGIFactory1_orig(riid, ppFactory);
}

PLUGIN_API HRESULT WINAPI CreateDXGIFactory2(UINT Flags, REFIID riid, _Out_ void** ppFactory)
{
	return CreateDXGIFactory2_orig(Flags, riid, ppFactory);
}

PLUGIN_API HRESULT WINAPI DXGID3D10CreateDevice(HMODULE hModule, void* pFactory, void* pAdapter, UINT Flags, void* unknown, void* ppDevice)
{
	return DXGID3D10CreateDevice_orig(hModule, pFactory, pAdapter, Flags, unknown, ppDevice);
}

PLUGIN_API HRESULT WINAPI DXGID3D10CreateLayeredDevice(UNKNOWN unk)
{
	return DXGID3D10CreateLayeredDevice_orig(unk);
}

PLUGIN_API size_t WINAPI DXGID3D10GetLayeredDeviceSize(const void* pLayers, UINT NumLayers)
{
	return DXGID3D10GetLayeredDeviceSize_orig(pLayers, NumLayers);
}

PLUGIN_API HRESULT WINAPI DXGID3D10RegisterLayers(const void* pLayers, UINT NumLayers)
{
	return DXGID3D10RegisterLayers_orig(pLayers, NumLayers);
}

PLUGIN_API HRESULT WINAPI DXGIGetDebugInterface1(UINT Flags, REFIID riid, void** pDebug)
{
	return DXGIGetDebugInterface1_orig(Flags, riid, pDebug);
}

PLUGIN_API HRESULT WINAPI DXGIReportAdapterConfiguration(DWORD unk)
{
	return DXGIReportAdapterConfiguration_orig(unk);
}

HMODULE origModule = NULL;

bool Proxy_Attach()
{
	extern HMODULE ourModule;

	// get the filename of our DLL and try loading the DLL with the same name from system dir
	WCHAR modulePath[MAX_PATH] = { 0 };
	if (!GetSystemDirectoryW(modulePath, _countof(modulePath)))
		return false;

	WCHAR ourModulePath[MAX_PATH] = { 0 };
	GetModuleFileNameW(ourModule, ourModulePath, _countof(ourModulePath));

	WCHAR exeName[MAX_PATH] = { 0 };
	WCHAR extName[MAX_PATH] = { 0 };
	_wsplitpath_s(ourModulePath, NULL, NULL, NULL, NULL, exeName, MAX_PATH, extName, MAX_PATH);

	WCHAR origModulePath[MAX_PATH] = { 0 };
	swprintf_s(origModulePath, MAX_PATH, L"%ws\\%ws%ws", modulePath, exeName, extName);

	origModule = LoadLibraryW(origModulePath);
	if (!origModule)
		return false;

	XInputGetCapabilities_orig = (XInputGetCapabilities_ptr)GetProcAddress(origModule, "XInputGetCapabilities");
	XInputGetDSoundAudioDeviceGuids_orig = (XInputGetDSoundAudioDeviceGuids_ptr)GetProcAddress(origModule, "XInputGetDSoundAudioDeviceGuids");
	XInputGetState_orig = (XInputGetState_ptr)GetProcAddress(origModule, "XInputGetState");
	XInputSetState_orig = (XInputSetState_ptr)GetProcAddress(origModule, "XInputSetState");
	XInputEnable_orig = (XInputEnable_ptr)GetProcAddress(origModule, "XInputEnable");
	XInputGetBatteryInformation_orig = (XInputGetBatteryInformation_ptr)GetProcAddress(origModule, "XInputGetBatteryInformation");
	XInputGetKeystroke_orig = (XInputGetKeystroke_ptr)GetProcAddress(origModule, "XInputGetKeystroke");

	DirectInput8Create_orig = (DirectInput8Create_ptr)GetProcAddress(origModule, "DirectInput8Create");

	DXGIDumpJournal_orig = (DXGIDumpJournal_ptr)GetProcAddress(origModule, "DXGIDumpJournal");
	CreateDXGIFactory_orig = (CreateDXGIFactory_ptr)GetProcAddress(origModule, "CreateDXGIFactory");
	CreateDXGIFactory1_orig = (CreateDXGIFactory1_ptr)GetProcAddress(origModule, "CreateDXGIFactory1");
	CreateDXGIFactory2_orig = (CreateDXGIFactory2_ptr)GetProcAddress(origModule, "CreateDXGIFactory2");
	DXGID3D10CreateDevice_orig = (DXGID3D10CreateDevice_ptr)GetProcAddress(origModule, "DXGID3D10CreateDevice");
	DXGID3D10CreateLayeredDevice_orig = (DXGID3D10CreateLayeredDevice_ptr)GetProcAddress(origModule, "DXGID3D10CreateLayeredDevice");
	DXGID3D10GetLayeredDeviceSize_orig = (DXGID3D10GetLayeredDeviceSize_ptr)GetProcAddress(origModule, "DXGID3D10GetLayeredDeviceSize");
	DXGID3D10RegisterLayers_orig = (DXGID3D10RegisterLayers_ptr)GetProcAddress(origModule, "DXGID3D10RegisterLayers");
	DXGIGetDebugInterface1_orig = (DXGIGetDebugInterface1_ptr)GetProcAddress(origModule, "DXGIGetDebugInterface1");
	DXGIReportAdapterConfiguration_orig = (DXGIReportAdapterConfiguration_ptr)GetProcAddress(origModule, "DXGIReportAdapterConfiguration");

	X3DAudioInitialize_orig = (X3DAudioInitialize_ptr)GetProcAddress(origModule, "X3DAudioInitialize");
	X3DAudioCalculate_orig = (X3DAudioCalculate_ptr)GetProcAddress(origModule, "X3DAudioCalculate");

	CreateFX_orig = (CreateFX_ptr)GetProcAddress(origModule, "CreateFX");

	return true;
}

void Proxy_Detach()
{
	if (!origModule)
		return;

	FreeLibrary(origModule);
	origModule = nullptr;
}
