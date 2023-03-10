#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include "Proxy.hpp"

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

// dinput8.dll
typedef HRESULT(WINAPI* DirectInput8Create_ptr)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, void* punkOuter);

DirectInput8Create_ptr DirectInput8Create_orig;

PLUGIN_API HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, void* punkOuter)
{
    return DirectInput8Create_orig(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

// dxgi.dll
struct UNKNOWN
{
    BYTE unknown[20];
};

typedef HRESULT(WINAPI* DXGIDumpJournal_ptr)(void* unk);
typedef HRESULT(WINAPI* CreateDXGIFactory_ptr)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* CreateDXGIFactory1_ptr)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* CreateDXGIFactory2_ptr)(UINT Flags, REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* DXGID3D10CreateDevice_ptr)(
    HMODULE hModule, void* pFactory, void* pAdapter, UINT Flags, void* unknown, void* ppDevice);
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

// winmm.dll
FARPROC PlaySoundW_orig;
FARPROC timeSetEvent_orig;
FARPROC timeKillEvent_orig;
FARPROC midiOutMessage_orig;
FARPROC timeBeginPeriod_orig;
FARPROC timeGetTime_orig;
FARPROC NotifyCallbackData_orig;
FARPROC WOW32DriverCallback_orig;
FARPROC WOW32ResolveMultiMediaHandle_orig;
FARPROC aux32Message_orig;
FARPROC joy32Message_orig;
FARPROC mid32Message_orig;
FARPROC mod32Message_orig;
FARPROC mxd32Message_orig;
FARPROC tid32Message_orig;
FARPROC wid32Message_orig;
FARPROC wod32Message_orig;
FARPROC mci32Message_orig;
FARPROC CloseDriver_orig;
FARPROC DefDriverProc_orig;
FARPROC DriverCallback_orig;
FARPROC DrvGetModuleHandle_orig;
FARPROC GetDriverModuleHandle_orig;
FARPROC OpenDriver_orig;
FARPROC PlaySound_orig;
FARPROC Ordinal2_orig;
FARPROC SendDriverMessage_orig;
FARPROC auxGetDevCapsA_orig;
FARPROC auxGetDevCapsW_orig;
FARPROC auxGetNumDevs_orig;
FARPROC auxGetVolume_orig;
FARPROC auxOutMessage_orig;
FARPROC auxSetVolume_orig;
FARPROC joyConfigChanged_orig;
FARPROC joyGetDevCapsA_orig;
FARPROC joyGetDevCapsW_orig;
FARPROC joyGetNumDevs_orig;
FARPROC joyGetPosEx_orig;
FARPROC joyGetPos_orig;
FARPROC joyGetThreshold_orig;
FARPROC joyReleaseCapture_orig;
FARPROC joySetCapture_orig;
FARPROC joySetThreshold_orig;
FARPROC midiConnect_orig;
FARPROC midiDisconnect_orig;
FARPROC midiInAddBuffer_orig;
FARPROC midiInClose_orig;
FARPROC midiInGetDevCapsA_orig;
FARPROC midiInGetDevCapsW_orig;
FARPROC midiInGetErrorTextA_orig;
FARPROC midiInGetErrorTextW_orig;
FARPROC midiInGetID_orig;
FARPROC midiInGetNumDevs_orig;
FARPROC midiInMessage_orig;
FARPROC midiInOpen_orig;
FARPROC midiInPrepareHeader_orig;
FARPROC midiInReset_orig;
FARPROC midiInStart_orig;
FARPROC midiInStop_orig;
FARPROC midiInUnprepareHeader_orig;
FARPROC midiOutCacheDrumPatches_orig;
FARPROC midiOutCachePatches_orig;
FARPROC midiOutClose_orig;
FARPROC midiOutGetDevCapsA_orig;
FARPROC midiOutGetDevCapsW_orig;
FARPROC midiOutGetErrorTextA_orig;
FARPROC midiOutGetErrorTextW_orig;
FARPROC midiOutGetID_orig;
FARPROC midiOutGetNumDevs_orig;
FARPROC midiOutGetVolume_orig;
FARPROC midiOutLongMsg_orig;
FARPROC midiOutOpen_orig;
FARPROC midiOutPrepareHeader_orig;
FARPROC midiOutReset_orig;
FARPROC midiOutSetVolume_orig;
FARPROC midiOutShortMsg_orig;
FARPROC midiOutUnprepareHeader_orig;
FARPROC midiStreamClose_orig;
FARPROC midiStreamOpen_orig;
FARPROC midiStreamOut_orig;
FARPROC midiStreamPause_orig;
FARPROC midiStreamPosition_orig;
FARPROC midiStreamProperty_orig;
FARPROC midiStreamRestart_orig;
FARPROC midiStreamStop_orig;
FARPROC mixerClose_orig;
FARPROC mixerGetControlDetailsA_orig;
FARPROC mixerGetControlDetailsW_orig;
FARPROC mixerGetDevCapsA_orig;
FARPROC mixerGetDevCapsW_orig;
FARPROC mixerGetID_orig;
FARPROC mixerGetLineControlsA_orig;
FARPROC mixerGetLineControlsW_orig;
FARPROC mixerGetLineInfoA_orig;
FARPROC mixerGetLineInfoW_orig;
FARPROC mixerGetNumDevs_orig;
FARPROC mixerMessage_orig;
FARPROC mixerOpen_orig;
FARPROC mixerSetControlDetails_orig;
FARPROC mmDrvInstall_orig;
FARPROC mmGetCurrentTask_orig;
FARPROC mmTaskBlock_orig;
FARPROC mmTaskCreate_orig;
FARPROC mmTaskSignal_orig;
FARPROC mmTaskYield_orig;
FARPROC mmioAdvance_orig;
FARPROC mmioAscend_orig;
FARPROC mmioClose_orig;
FARPROC mmioCreateChunk_orig;
FARPROC mmioDescend_orig;
FARPROC mmioFlush_orig;
FARPROC mmioGetInfo_orig;
FARPROC mmioInstallIOProcA_orig;
FARPROC mmioInstallIOProcW_orig;
FARPROC mmioOpenA_orig;
FARPROC mmioOpenW_orig;
FARPROC mmioRead_orig;
FARPROC mmioRenameA_orig;
FARPROC mmioRenameW_orig;
FARPROC mmioSeek_orig;
FARPROC mmioSendMessage_orig;
FARPROC mmioSetBuffer_orig;
FARPROC mmioSetInfo_orig;
FARPROC mmioStringToFOURCCA_orig;
FARPROC mmioStringToFOURCCW_orig;
FARPROC mmioWrite_orig;
FARPROC timeEndPeriod_orig;
FARPROC timeGetDevCaps_orig;
FARPROC timeGetSystemTime_orig;
FARPROC waveInAddBuffer_orig;
FARPROC waveInClose_orig;
FARPROC waveInGetDevCapsA_orig;
FARPROC waveInGetDevCapsW_orig;
FARPROC waveInGetErrorTextA_orig;
FARPROC waveInGetErrorTextW_orig;
FARPROC waveInGetID_orig;
FARPROC waveInGetNumDevs_orig;
FARPROC waveInGetPosition_orig;
FARPROC waveInMessage_orig;
FARPROC waveInOpen_orig;
FARPROC waveInPrepareHeader_orig;
FARPROC waveInReset_orig;
FARPROC waveInStart_orig;
FARPROC waveInStop_orig;
FARPROC waveInUnprepareHeader_orig;
FARPROC waveOutBreakLoop_orig;
FARPROC waveOutClose_orig;
FARPROC waveOutGetDevCapsA_orig;
FARPROC waveOutGetDevCapsW_orig;
FARPROC waveOutGetErrorTextA_orig;
FARPROC waveOutGetErrorTextW_orig;
FARPROC waveOutGetID_orig;
FARPROC waveOutGetNumDevs_orig;
FARPROC waveOutGetPitch_orig;
FARPROC waveOutGetPlaybackRate_orig;
FARPROC waveOutGetPosition_orig;
FARPROC waveOutGetVolume_orig;
FARPROC waveOutMessage_orig;
FARPROC waveOutOpen_orig;
FARPROC waveOutPause_orig;
FARPROC waveOutPrepareHeader_orig;
FARPROC waveOutReset_orig;
FARPROC waveOutRestart_orig;
FARPROC waveOutSetPitch_orig;
FARPROC waveOutSetPlaybackRate_orig;
FARPROC waveOutSetVolume_orig;
FARPROC waveOutUnprepareHeader_orig;
FARPROC waveOutWrite_orig;
FARPROC mciExecute_orig;
FARPROC mciGetErrorStringA_orig;
FARPROC mciGetErrorStringW_orig;
FARPROC mciSendCommandA_orig;
FARPROC mciSendCommandW_orig;
FARPROC mciSendStringA_orig;
FARPROC mciSendStringW_orig;
FARPROC mciFreeCommandResource_orig;
FARPROC mciLoadCommandResource_orig;
FARPROC mciDriverNotify_orig;
FARPROC mciDriverYield_orig;
FARPROC mciGetCreatorTask_orig;
FARPROC mciGetDeviceIDA_orig;
FARPROC mciGetDeviceIDFromElementIDA_orig;
FARPROC mciGetDeviceIDFromElementIDW_orig;
FARPROC mciGetDeviceIDW_orig;
FARPROC mciGetDriverData_orig;
FARPROC mciGetYieldProc_orig;
FARPROC mciSetDriverData_orig;
FARPROC mciSetYieldProc_orig;
FARPROC PlaySoundA_orig;
FARPROC sndPlaySoundA_orig;
FARPROC sndPlaySoundW_orig;
FARPROC WOWAppExit_orig;
FARPROC mmsystemGetVersion_orig;

PLUGIN_API void CloseDriver()
{
    CloseDriver_orig();
}
PLUGIN_API void DefDriverProc()
{
    DefDriverProc_orig();
}
PLUGIN_API void DriverCallback()
{
    DriverCallback_orig();
}
PLUGIN_API void DrvGetModuleHandle()
{
    DrvGetModuleHandle_orig();
}
PLUGIN_API void GetDriverModuleHandle()
{
    GetDriverModuleHandle_orig();
}
PLUGIN_API void NotifyCallbackData()
{
    NotifyCallbackData_orig();
}
PLUGIN_API void OpenDriver()
{
    OpenDriver_orig();
}
PLUGIN_API void PlaySound()
{
    PlaySound_orig();
}
PLUGIN_API void PlaySoundA()
{
    PlaySoundA_orig();
}
PLUGIN_API void PlaySoundW()
{
    PlaySoundW_orig();
}
PLUGIN_API void SendDriverMessage()
{
    SendDriverMessage_orig();
}
PLUGIN_API void WOW32DriverCallback()
{
    WOW32DriverCallback_orig();
}
PLUGIN_API void WOW32ResolveMultiMediaHandle()
{
    WOW32ResolveMultiMediaHandle_orig();
}
PLUGIN_API void WOWAppExit()
{
    WOWAppExit_orig();
}
PLUGIN_API void aux32Message()
{
    aux32Message_orig();
}
PLUGIN_API void auxGetDevCapsA()
{
    auxGetDevCapsA_orig();
}
PLUGIN_API void auxGetDevCapsW()
{
    auxGetDevCapsW_orig();
}
PLUGIN_API void auxGetNumDevs()
{
    auxGetNumDevs_orig();
}
PLUGIN_API void auxGetVolume()
{
    auxGetVolume_orig();
}
PLUGIN_API void auxOutMessage()
{
    auxOutMessage_orig();
}
PLUGIN_API void auxSetVolume()
{
    auxSetVolume_orig();
}
PLUGIN_API void joy32Message()
{
    joy32Message_orig();
}
PLUGIN_API void joyConfigChanged()
{
    joyConfigChanged_orig();
}
PLUGIN_API void joyGetDevCapsA()
{
    joyGetDevCapsA_orig();
}
PLUGIN_API void joyGetDevCapsW()
{
    joyGetDevCapsW_orig();
}
PLUGIN_API void joyGetNumDevs()
{
    joyGetNumDevs_orig();
}
PLUGIN_API void joyGetPos()
{
    joyGetPos_orig();
}
PLUGIN_API void joyGetPosEx()
{
    joyGetPosEx_orig();
}
PLUGIN_API void joyGetThreshold()
{
    joyGetThreshold_orig();
}
PLUGIN_API void joyReleaseCapture()
{
    joyReleaseCapture_orig();
}
PLUGIN_API void joySetCapture()
{
    joySetCapture_orig();
}
PLUGIN_API void joySetThreshold()
{
    joySetThreshold_orig();
}
PLUGIN_API void mci32Message()
{
    mci32Message_orig();
}
PLUGIN_API void mciDriverNotify()
{
    mciDriverNotify_orig();
}
PLUGIN_API void mciDriverYield()
{
    mciDriverYield_orig();
}
PLUGIN_API void mciExecute()
{
    mciExecute_orig();
}
PLUGIN_API void mciFreeCommandResource()
{
    mciFreeCommandResource_orig();
}
PLUGIN_API void mciGetCreatorTask()
{
    mciGetCreatorTask_orig();
}
PLUGIN_API void mciGetDeviceIDA()
{
    mciGetDeviceIDA_orig();
}
PLUGIN_API void mciGetDeviceIDFromElementIDA()
{
    mciGetDeviceIDFromElementIDA_orig();
}
PLUGIN_API void mciGetDeviceIDFromElementIDW()
{
    mciGetDeviceIDFromElementIDW_orig();
}
PLUGIN_API void mciGetDeviceIDW()
{
    mciGetDeviceIDW_orig();
}
PLUGIN_API void mciGetDriverData()
{
    mciGetDriverData_orig();
}
PLUGIN_API void mciGetErrorStringA()
{
    mciGetErrorStringA_orig();
}
PLUGIN_API void mciGetErrorStringW()
{
    mciGetErrorStringW_orig();
}
PLUGIN_API void mciGetYieldProc()
{
    mciGetYieldProc_orig();
}
PLUGIN_API void mciLoadCommandResource()
{
    mciLoadCommandResource_orig();
}
PLUGIN_API void mciSendCommandA()
{
    mciSendCommandA_orig();
}
PLUGIN_API void mciSendCommandW()
{
    mciSendCommandW_orig();
}
PLUGIN_API void mciSendStringA()
{
    mciSendStringA_orig();
}
PLUGIN_API void mciSendStringW()
{
    mciSendStringW_orig();
}
PLUGIN_API void mciSetDriverData()
{
    mciSetDriverData_orig();
}
PLUGIN_API void mciSetYieldProc()
{
    mciSetYieldProc_orig();
}
PLUGIN_API void mid32Message()
{
    mid32Message_orig();
}
PLUGIN_API void midiConnect()
{
    midiConnect_orig();
}
PLUGIN_API void midiDisconnect()
{
    midiDisconnect_orig();
}
PLUGIN_API void midiInAddBuffer()
{
    midiInAddBuffer_orig();
}
PLUGIN_API void midiInClose()
{
    midiInClose_orig();
}
PLUGIN_API void midiInGetDevCapsA()
{
    midiInGetDevCapsA_orig();
}
PLUGIN_API void midiInGetDevCapsW()
{
    midiInGetDevCapsW_orig();
}
PLUGIN_API void midiInGetErrorTextA()
{
    midiInGetErrorTextA_orig();
}
PLUGIN_API void midiInGetErrorTextW()
{
    midiInGetErrorTextW_orig();
}
PLUGIN_API void midiInGetID()
{
    midiInGetID_orig();
}
PLUGIN_API void midiInGetNumDevs()
{
    midiInGetNumDevs_orig();
}
PLUGIN_API void midiInMessage()
{
    midiInMessage_orig();
}
PLUGIN_API void midiInOpen()
{
    midiInOpen_orig();
}
PLUGIN_API void midiInPrepareHeader()
{
    midiInPrepareHeader_orig();
}
PLUGIN_API void midiInReset()
{
    midiInReset_orig();
}
PLUGIN_API void midiInStart()
{
    midiInStart_orig();
}
PLUGIN_API void midiInStop()
{
    midiInStop_orig();
}
PLUGIN_API void midiInUnprepareHeader()
{
    midiInUnprepareHeader_orig();
}
PLUGIN_API void midiOutCacheDrumPatches()
{
    midiOutCacheDrumPatches_orig();
}
PLUGIN_API void midiOutCachePatches()
{
    midiOutCachePatches_orig();
}
PLUGIN_API void midiOutClose()
{
    midiOutClose_orig();
}
PLUGIN_API void midiOutGetDevCapsA()
{
    midiOutGetDevCapsA_orig();
}
PLUGIN_API void midiOutGetDevCapsW()
{
    midiOutGetDevCapsW_orig();
}
PLUGIN_API void midiOutGetErrorTextA()
{
    midiOutGetErrorTextA_orig();
}
PLUGIN_API void midiOutGetErrorTextW()
{
    midiOutGetErrorTextW_orig();
}
PLUGIN_API void midiOutGetID()
{
    midiOutGetID_orig();
}
PLUGIN_API void midiOutGetNumDevs()
{
    midiOutGetNumDevs_orig();
}
PLUGIN_API void midiOutGetVolume()
{
    midiOutGetVolume_orig();
}
PLUGIN_API void midiOutLongMsg()
{
    midiOutLongMsg_orig();
}
PLUGIN_API void midiOutMessage()
{
    midiOutMessage_orig();
}
PLUGIN_API void midiOutOpen()
{
    midiOutOpen_orig();
}
PLUGIN_API void midiOutPrepareHeader()
{
    midiOutPrepareHeader_orig();
}
PLUGIN_API void midiOutReset()
{
    midiOutReset_orig();
}
PLUGIN_API void midiOutSetVolume()
{
    midiOutSetVolume_orig();
}
PLUGIN_API void midiOutShortMsg()
{
    midiOutShortMsg_orig();
}
PLUGIN_API void midiOutUnprepareHeader()
{
    midiOutUnprepareHeader_orig();
}
PLUGIN_API void midiStreamClose()
{
    midiStreamClose_orig();
}
PLUGIN_API void midiStreamOpen()
{
    midiStreamOpen_orig();
}
PLUGIN_API void midiStreamOut()
{
    midiStreamOut_orig();
}
PLUGIN_API void midiStreamPause()
{
    midiStreamPause_orig();
}
PLUGIN_API void midiStreamPosition()
{
    midiStreamPosition_orig();
}
PLUGIN_API void midiStreamProperty()
{
    midiStreamProperty_orig();
}
PLUGIN_API void midiStreamRestart()
{
    midiStreamRestart_orig();
}
PLUGIN_API void midiStreamStop()
{
    midiStreamStop_orig();
}
PLUGIN_API void mixerClose()
{
    mixerClose_orig();
}
PLUGIN_API void mixerGetControlDetailsA()
{
    mixerGetControlDetailsA_orig();
}
PLUGIN_API void mixerGetControlDetailsW()
{
    mixerGetControlDetailsW_orig();
}
PLUGIN_API void mixerGetDevCapsA()
{
    mixerGetDevCapsA_orig();
}
PLUGIN_API void mixerGetDevCapsW()
{
    mixerGetDevCapsW_orig();
}
PLUGIN_API void mixerGetID()
{
    mixerGetID_orig();
}
PLUGIN_API void mixerGetLineControlsA()
{
    mixerGetLineControlsA_orig();
}
PLUGIN_API void mixerGetLineControlsW()
{
    mixerGetLineControlsW_orig();
}
PLUGIN_API void mixerGetLineInfoA()
{
    mixerGetLineInfoA_orig();
}
PLUGIN_API void mixerGetLineInfoW()
{
    mixerGetLineInfoW_orig();
}
PLUGIN_API void mixerGetNumDevs()
{
    mixerGetNumDevs_orig();
}
PLUGIN_API void mixerMessage()
{
    mixerMessage_orig();
}
PLUGIN_API void mixerOpen()
{
    mixerOpen_orig();
}
PLUGIN_API void mixerSetControlDetails()
{
    mixerSetControlDetails_orig();
}
PLUGIN_API void mmDrvInstall()
{
    mmDrvInstall_orig();
}
PLUGIN_API void mmGetCurrentTask()
{
    mmGetCurrentTask_orig();
}
PLUGIN_API void mmTaskBlock()
{
    mmTaskBlock_orig();
}
PLUGIN_API void mmTaskCreate()
{
    mmTaskCreate_orig();
}
PLUGIN_API void mmTaskSignal()
{
    mmTaskSignal_orig();
}
PLUGIN_API void mmTaskYield()
{
    mmTaskYield_orig();
}
PLUGIN_API void mmioAdvance()
{
    mmioAdvance_orig();
}
PLUGIN_API void mmioAscend()
{
    mmioAscend_orig();
}
PLUGIN_API void mmioClose()
{
    mmioClose_orig();
}
PLUGIN_API void mmioCreateChunk()
{
    mmioCreateChunk_orig();
}
PLUGIN_API void mmioDescend()
{
    mmioDescend_orig();
}
PLUGIN_API void mmioFlush()
{
    mmioFlush_orig();
}
PLUGIN_API void mmioGetInfo()
{
    mmioGetInfo_orig();
}
PLUGIN_API void mmioInstallIOProcA()
{
    mmioInstallIOProcA_orig();
}
PLUGIN_API void mmioInstallIOProcW()
{
    mmioInstallIOProcW_orig();
}
PLUGIN_API void mmioOpenA()
{
    mmioOpenA_orig();
}
PLUGIN_API void mmioOpenW()
{
    mmioOpenW_orig();
}
PLUGIN_API void mmioRead()
{
    mmioRead_orig();
}
PLUGIN_API void mmioRenameA()
{
    mmioRenameA_orig();
}
PLUGIN_API void mmioRenameW()
{
    mmioRenameW_orig();
}
PLUGIN_API void mmioSeek()
{
    mmioSeek_orig();
}
PLUGIN_API void mmioSendMessage()
{
    mmioSendMessage_orig();
}
PLUGIN_API void mmioSetBuffer()
{
    mmioSetBuffer_orig();
}
PLUGIN_API void mmioSetInfo()
{
    mmioSetInfo_orig();
}
PLUGIN_API void mmioStringToFOURCCA()
{
    mmioStringToFOURCCA_orig();
}
PLUGIN_API void mmioStringToFOURCCW()
{
    mmioStringToFOURCCW_orig();
}
PLUGIN_API void mmioWrite()
{
    mmioWrite_orig();
}
PLUGIN_API void mmsystemGetVersion()
{
    mmsystemGetVersion_orig();
}
PLUGIN_API void mod32Message()
{
    mod32Message_orig();
}
PLUGIN_API void mxd32Message()
{
    mxd32Message_orig();
}
PLUGIN_API void sndPlaySoundA()
{
    sndPlaySoundA_orig();
}
PLUGIN_API void sndPlaySoundW()
{
    sndPlaySoundW_orig();
}
PLUGIN_API void tid32Message()
{
    tid32Message_orig();
}
PLUGIN_API void timeBeginPeriod()
{
    timeBeginPeriod_orig();
}
PLUGIN_API void timeEndPeriod()
{
    timeEndPeriod_orig();
}
PLUGIN_API void timeGetDevCaps()
{
    timeGetDevCaps_orig();
}
PLUGIN_API void timeGetSystemTime()
{
    timeGetSystemTime_orig();
}
PLUGIN_API void timeGetTime()
{
    timeGetTime_orig();
}
PLUGIN_API void timeKillEvent()
{
    timeKillEvent_orig();
}
PLUGIN_API void timeSetEvent()
{
    timeSetEvent_orig();
}
PLUGIN_API void waveInAddBuffer()
{
    waveInAddBuffer_orig();
}
PLUGIN_API void waveInClose()
{
    waveInClose_orig();
}
PLUGIN_API void waveInGetDevCapsA()
{
    waveInGetDevCapsA_orig();
}
PLUGIN_API void waveInGetDevCapsW()
{
    waveInGetDevCapsW_orig();
}
PLUGIN_API void waveInGetErrorTextA()
{
    waveInGetErrorTextA_orig();
}
PLUGIN_API void waveInGetErrorTextW()
{
    waveInGetErrorTextW_orig();
}
PLUGIN_API void waveInGetID()
{
    waveInGetID_orig();
}
PLUGIN_API void waveInGetNumDevs()
{
    waveInGetNumDevs_orig();
}
PLUGIN_API void waveInGetPosition()
{
    waveInGetPosition_orig();
}
PLUGIN_API void waveInMessage()
{
    waveInMessage_orig();
}
PLUGIN_API void waveInOpen()
{
    waveInOpen_orig();
}
PLUGIN_API void waveInPrepareHeader()
{
    waveInPrepareHeader_orig();
}
PLUGIN_API void waveInReset()
{
    waveInReset_orig();
}
PLUGIN_API void waveInStart()
{
    waveInStart_orig();
}
PLUGIN_API void waveInStop()
{
    waveInStop_orig();
}
PLUGIN_API void waveInUnprepareHeader()
{
    waveInUnprepareHeader_orig();
}
PLUGIN_API void waveOutBreakLoop()
{
    waveOutBreakLoop_orig();
}
PLUGIN_API void waveOutClose()
{
    waveOutClose_orig();
}
PLUGIN_API void waveOutGetDevCapsA()
{
    waveOutGetDevCapsA_orig();
}
PLUGIN_API void waveOutGetDevCapsW()
{
    waveOutGetDevCapsW_orig();
}
PLUGIN_API void waveOutGetErrorTextA()
{
    waveOutGetErrorTextA_orig();
}
PLUGIN_API void waveOutGetErrorTextW()
{
    waveOutGetErrorTextW_orig();
}
PLUGIN_API void waveOutGetID()
{
    waveOutGetID_orig();
}
PLUGIN_API void waveOutGetNumDevs()
{
    waveOutGetNumDevs_orig();
}
PLUGIN_API void waveOutGetPitch()
{
    waveOutGetPitch_orig();
}
PLUGIN_API void waveOutGetPlaybackRate()
{
    waveOutGetPlaybackRate_orig();
}
PLUGIN_API void waveOutGetPosition()
{
    waveOutGetPosition_orig();
}
PLUGIN_API void waveOutGetVolume()
{
    waveOutGetVolume_orig();
}
PLUGIN_API void waveOutMessage()
{
    waveOutMessage_orig();
}
PLUGIN_API void waveOutOpen()
{
    waveOutOpen_orig();
}
PLUGIN_API void waveOutPause()
{
    waveOutPause_orig();
}
PLUGIN_API void waveOutPrepareHeader()
{
    waveOutPrepareHeader_orig();
}
PLUGIN_API void waveOutReset()
{
    waveOutReset_orig();
}
PLUGIN_API void waveOutRestart()
{
    waveOutRestart_orig();
}
PLUGIN_API void waveOutSetPitch()
{
    waveOutSetPitch_orig();
}
PLUGIN_API void waveOutSetPlaybackRate()
{
    waveOutSetPlaybackRate_orig();
}
PLUGIN_API void waveOutSetVolume()
{
    waveOutSetVolume_orig();
}
PLUGIN_API void waveOutUnprepareHeader()
{
    waveOutUnprepareHeader_orig();
}
PLUGIN_API void waveOutWrite()
{
    waveOutWrite_orig();
}
PLUGIN_API void wid32Message()
{
    wid32Message_orig();
}
PLUGIN_API void wod32Message()
{
    wod32Message_orig();
}

namespace proxy
{
HMODULE origModule = NULL;
bool is_wrapping_nvngx = false;

bool on_attach(HMODULE ourModule)
{
    // get the filename of our DLL and try loading the DLL with the same name from system dir
    WCHAR modulePath[MAX_PATH] = {0};
    if (!GetSystemDirectoryW(modulePath, _countof(modulePath)))
        return false;

    WCHAR ourModulePath[MAX_PATH] = {0};
    GetModuleFileNameW(ourModule, ourModulePath, _countof(ourModulePath));

    WCHAR exeName[MAX_PATH] = {0};
    WCHAR extName[MAX_PATH] = {0};
    _wsplitpath_s(ourModulePath, NULL, NULL, NULL, NULL, exeName, MAX_PATH, extName, MAX_PATH);

    if (!_wcsicmp(exeName, L"nvngx"))
    {
        is_wrapping_nvngx = true;
        return proxy_nvngx::on_attach(ourModule);
    }

    WCHAR origModulePath[MAX_PATH] = {0};
    swprintf_s(origModulePath, MAX_PATH, L"%ws\\%ws%ws", modulePath, exeName, extName);

    origModule = LoadLibraryW(origModulePath);
    if (!origModule)
        return false;

    XInputGetCapabilities_orig = (XInputGetCapabilities_ptr)GetProcAddress(origModule, "XInputGetCapabilities");
    XInputGetDSoundAudioDeviceGuids_orig =
        (XInputGetDSoundAudioDeviceGuids_ptr)GetProcAddress(origModule, "XInputGetDSoundAudioDeviceGuids");
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

    PlaySoundW_orig = GetProcAddress(origModule, "PlaySoundW");
    timeSetEvent_orig = GetProcAddress(origModule, "timeSetEvent");
    timeKillEvent_orig = GetProcAddress(origModule, "timeKillEvent");
    midiOutMessage_orig = GetProcAddress(origModule, "midiOutMessage");
    timeBeginPeriod_orig = GetProcAddress(origModule, "timeBeginPeriod");
    timeGetTime_orig = GetProcAddress(origModule, "timeGetTime");
    NotifyCallbackData_orig = GetProcAddress(origModule, "NotifyCallbackData");
    WOW32DriverCallback_orig = GetProcAddress(origModule, "WOW32DriverCallback");
    WOW32ResolveMultiMediaHandle_orig = GetProcAddress(origModule, "WOW32ResolveMultiMediaHandle");
    aux32Message_orig = GetProcAddress(origModule, "aux32Message");
    joy32Message_orig = GetProcAddress(origModule, "joy32Message");
    mid32Message_orig = GetProcAddress(origModule, "mid32Message");
    mod32Message_orig = GetProcAddress(origModule, "mod32Message");
    mxd32Message_orig = GetProcAddress(origModule, "mxd32Message");
    tid32Message_orig = GetProcAddress(origModule, "tid32Message");
    wid32Message_orig = GetProcAddress(origModule, "wid32Message");
    wod32Message_orig = GetProcAddress(origModule, "wod32Message");
    mci32Message_orig = GetProcAddress(origModule, "mci32Message");
    CloseDriver_orig = GetProcAddress(origModule, "CloseDriver");
    DefDriverProc_orig = GetProcAddress(origModule, "DefDriverProc");
    DriverCallback_orig = GetProcAddress(origModule, "DriverCallback");
    DrvGetModuleHandle_orig = GetProcAddress(origModule, "DrvGetModuleHandle");
    GetDriverModuleHandle_orig = GetProcAddress(origModule, "GetDriverModuleHandle");
    OpenDriver_orig = GetProcAddress(origModule, "OpenDriver");
    PlaySound_orig = GetProcAddress(origModule, "PlaySound");
    Ordinal2_orig = GetProcAddress(origModule, "Ordinal2");
    SendDriverMessage_orig = GetProcAddress(origModule, "SendDriverMessage");
    auxGetDevCapsA_orig = GetProcAddress(origModule, "auxGetDevCapsA");
    auxGetDevCapsW_orig = GetProcAddress(origModule, "auxGetDevCapsW");
    auxGetNumDevs_orig = GetProcAddress(origModule, "auxGetNumDevs");
    auxGetVolume_orig = GetProcAddress(origModule, "auxGetVolume");
    auxOutMessage_orig = GetProcAddress(origModule, "auxOutMessage");
    auxSetVolume_orig = GetProcAddress(origModule, "auxSetVolume");
    joyConfigChanged_orig = GetProcAddress(origModule, "joyConfigChanged");
    joyGetDevCapsA_orig = GetProcAddress(origModule, "joyGetDevCapsA");
    joyGetDevCapsW_orig = GetProcAddress(origModule, "joyGetDevCapsW");
    joyGetNumDevs_orig = GetProcAddress(origModule, "joyGetNumDevs");
    joyGetPosEx_orig = GetProcAddress(origModule, "joyGetPosEx");
    joyGetPos_orig = GetProcAddress(origModule, "joyGetPos");
    joyGetThreshold_orig = GetProcAddress(origModule, "joyGetThreshold");
    joyReleaseCapture_orig = GetProcAddress(origModule, "joyReleaseCapture");
    joySetCapture_orig = GetProcAddress(origModule, "joySetCapture");
    joySetThreshold_orig = GetProcAddress(origModule, "joySetThreshold");
    midiConnect_orig = GetProcAddress(origModule, "midiConnect");
    midiDisconnect_orig = GetProcAddress(origModule, "midiDisconnect");
    midiInAddBuffer_orig = GetProcAddress(origModule, "midiInAddBuffer");
    midiInClose_orig = GetProcAddress(origModule, "midiInClose");
    midiInGetDevCapsA_orig = GetProcAddress(origModule, "midiInGetDevCapsA");
    midiInGetDevCapsW_orig = GetProcAddress(origModule, "midiInGetDevCapsW");
    midiInGetErrorTextA_orig = GetProcAddress(origModule, "midiInGetErrorTextA");
    midiInGetErrorTextW_orig = GetProcAddress(origModule, "midiInGetErrorTextW");
    midiInGetID_orig = GetProcAddress(origModule, "midiInGetID");
    midiInGetNumDevs_orig = GetProcAddress(origModule, "midiInGetNumDevs");
    midiInMessage_orig = GetProcAddress(origModule, "midiInMessage");
    midiInOpen_orig = GetProcAddress(origModule, "midiInOpen");
    midiInPrepareHeader_orig = GetProcAddress(origModule, "midiInPrepareHeader");
    midiInReset_orig = GetProcAddress(origModule, "midiInReset");
    midiInStart_orig = GetProcAddress(origModule, "midiInStart");
    midiInStop_orig = GetProcAddress(origModule, "midiInStop");
    midiInUnprepareHeader_orig = GetProcAddress(origModule, "midiInUnprepareHeader");
    midiOutCacheDrumPatches_orig = GetProcAddress(origModule, "midiOutCacheDrumPatches");
    midiOutCachePatches_orig = GetProcAddress(origModule, "midiOutCachePatches");
    midiOutClose_orig = GetProcAddress(origModule, "midiOutClose");
    midiOutGetDevCapsA_orig = GetProcAddress(origModule, "midiOutGetDevCapsA");
    midiOutGetDevCapsW_orig = GetProcAddress(origModule, "midiOutGetDevCapsW");
    midiOutGetErrorTextA_orig = GetProcAddress(origModule, "midiOutGetErrorTextA");
    midiOutGetErrorTextW_orig = GetProcAddress(origModule, "midiOutGetErrorTextW");
    midiOutGetID_orig = GetProcAddress(origModule, "midiOutGetID");
    midiOutGetNumDevs_orig = GetProcAddress(origModule, "midiOutGetNumDevs");
    midiOutGetVolume_orig = GetProcAddress(origModule, "midiOutGetVolume");
    midiOutLongMsg_orig = GetProcAddress(origModule, "midiOutLongMsg");
    midiOutOpen_orig = GetProcAddress(origModule, "midiOutOpen");
    midiOutPrepareHeader_orig = GetProcAddress(origModule, "midiOutPrepareHeader");
    midiOutReset_orig = GetProcAddress(origModule, "midiOutReset");
    midiOutSetVolume_orig = GetProcAddress(origModule, "midiOutSetVolume");
    midiOutShortMsg_orig = GetProcAddress(origModule, "midiOutShortMsg");
    midiOutUnprepareHeader_orig = GetProcAddress(origModule, "midiOutUnprepareHeader");
    midiStreamClose_orig = GetProcAddress(origModule, "midiStreamClose");
    midiStreamOpen_orig = GetProcAddress(origModule, "midiStreamOpen");
    midiStreamOut_orig = GetProcAddress(origModule, "midiStreamOut");
    midiStreamPause_orig = GetProcAddress(origModule, "midiStreamPause");
    midiStreamPosition_orig = GetProcAddress(origModule, "midiStreamPosition");
    midiStreamProperty_orig = GetProcAddress(origModule, "midiStreamProperty");
    midiStreamRestart_orig = GetProcAddress(origModule, "midiStreamRestart");
    midiStreamStop_orig = GetProcAddress(origModule, "midiStreamStop");
    mixerClose_orig = GetProcAddress(origModule, "mixerClose");
    mixerGetControlDetailsA_orig = GetProcAddress(origModule, "mixerGetControlDetailsA");
    mixerGetControlDetailsW_orig = GetProcAddress(origModule, "mixerGetControlDetailsW");
    mixerGetDevCapsA_orig = GetProcAddress(origModule, "mixerGetDevCapsA");
    mixerGetDevCapsW_orig = GetProcAddress(origModule, "mixerGetDevCapsW");
    mixerGetID_orig = GetProcAddress(origModule, "mixerGetID");
    mixerGetLineControlsA_orig = GetProcAddress(origModule, "mixerGetLineControlsA");
    mixerGetLineControlsW_orig = GetProcAddress(origModule, "mixerGetLineControlsW");
    mixerGetLineInfoA_orig = GetProcAddress(origModule, "mixerGetLineInfoA");
    mixerGetLineInfoW_orig = GetProcAddress(origModule, "mixerGetLineInfoW");
    mixerGetNumDevs_orig = GetProcAddress(origModule, "mixerGetNumDevs");
    mixerMessage_orig = GetProcAddress(origModule, "mixerMessage");
    mixerOpen_orig = GetProcAddress(origModule, "mixerOpen");
    mixerSetControlDetails_orig = GetProcAddress(origModule, "mixerSetControlDetails");
    mmDrvInstall_orig = GetProcAddress(origModule, "mmDrvInstall");
    mmGetCurrentTask_orig = GetProcAddress(origModule, "mmGetCurrentTask");
    mmTaskBlock_orig = GetProcAddress(origModule, "mmTaskBlock");
    mmTaskCreate_orig = GetProcAddress(origModule, "mmTaskCreate");
    mmTaskSignal_orig = GetProcAddress(origModule, "mmTaskSignal");
    mmTaskYield_orig = GetProcAddress(origModule, "mmTaskYield");
    mmioAdvance_orig = GetProcAddress(origModule, "mmioAdvance");
    mmioAscend_orig = GetProcAddress(origModule, "mmioAscend");
    mmioClose_orig = GetProcAddress(origModule, "mmioClose");
    mmioCreateChunk_orig = GetProcAddress(origModule, "mmioCreateChunk");
    mmioDescend_orig = GetProcAddress(origModule, "mmioDescend");
    mmioFlush_orig = GetProcAddress(origModule, "mmioFlush");
    mmioGetInfo_orig = GetProcAddress(origModule, "mmioGetInfo");
    mmioInstallIOProcA_orig = GetProcAddress(origModule, "mmioInstallIOProcA");
    mmioInstallIOProcW_orig = GetProcAddress(origModule, "mmioInstallIOProcW");
    mmioOpenA_orig = GetProcAddress(origModule, "mmioOpenA");
    mmioOpenW_orig = GetProcAddress(origModule, "mmioOpenW");
    mmioRead_orig = GetProcAddress(origModule, "mmioRead");
    mmioRenameA_orig = GetProcAddress(origModule, "mmioRenameA");
    mmioRenameW_orig = GetProcAddress(origModule, "mmioRenameW");
    mmioSeek_orig = GetProcAddress(origModule, "mmioSeek");
    mmioSendMessage_orig = GetProcAddress(origModule, "mmioSendMessage");
    mmioSetBuffer_orig = GetProcAddress(origModule, "mmioSetBuffer");
    mmioSetInfo_orig = GetProcAddress(origModule, "mmioSetInfo");
    mmioStringToFOURCCA_orig = GetProcAddress(origModule, "mmioStringToFOURCCA");
    mmioStringToFOURCCW_orig = GetProcAddress(origModule, "mmioStringToFOURCCW");
    mmioWrite_orig = GetProcAddress(origModule, "mmioWrite");
    timeEndPeriod_orig = GetProcAddress(origModule, "timeEndPeriod");
    timeGetDevCaps_orig = GetProcAddress(origModule, "timeGetDevCaps");
    timeGetSystemTime_orig = GetProcAddress(origModule, "timeGetSystemTime");
    waveInAddBuffer_orig = GetProcAddress(origModule, "waveInAddBuffer");
    waveInClose_orig = GetProcAddress(origModule, "waveInClose");
    waveInGetDevCapsA_orig = GetProcAddress(origModule, "waveInGetDevCapsA");
    waveInGetDevCapsW_orig = GetProcAddress(origModule, "waveInGetDevCapsW");
    waveInGetErrorTextA_orig = GetProcAddress(origModule, "waveInGetErrorTextA");
    waveInGetErrorTextW_orig = GetProcAddress(origModule, "waveInGetErrorTextW");
    waveInGetID_orig = GetProcAddress(origModule, "waveInGetID");
    waveInGetNumDevs_orig = GetProcAddress(origModule, "waveInGetNumDevs");
    waveInGetPosition_orig = GetProcAddress(origModule, "waveInGetPosition");
    waveInMessage_orig = GetProcAddress(origModule, "waveInMessage");
    waveInOpen_orig = GetProcAddress(origModule, "waveInOpen");
    waveInPrepareHeader_orig = GetProcAddress(origModule, "waveInPrepareHeader");
    waveInReset_orig = GetProcAddress(origModule, "waveInReset");
    waveInStart_orig = GetProcAddress(origModule, "waveInStart");
    waveInStop_orig = GetProcAddress(origModule, "waveInStop");
    waveInUnprepareHeader_orig = GetProcAddress(origModule, "waveInUnprepareHeader");
    waveOutBreakLoop_orig = GetProcAddress(origModule, "waveOutBreakLoop");
    waveOutClose_orig = GetProcAddress(origModule, "waveOutClose");
    waveOutGetDevCapsA_orig = GetProcAddress(origModule, "waveOutGetDevCapsA");
    waveOutGetDevCapsW_orig = GetProcAddress(origModule, "waveOutGetDevCapsW");
    waveOutGetErrorTextA_orig = GetProcAddress(origModule, "waveOutGetErrorTextA");
    waveOutGetErrorTextW_orig = GetProcAddress(origModule, "waveOutGetErrorTextW");
    waveOutGetID_orig = GetProcAddress(origModule, "waveOutGetID");
    waveOutGetNumDevs_orig = GetProcAddress(origModule, "waveOutGetNumDevs");
    waveOutGetPitch_orig = GetProcAddress(origModule, "waveOutGetPitch");
    waveOutGetPlaybackRate_orig = GetProcAddress(origModule, "waveOutGetPlaybackRate");
    waveOutGetPosition_orig = GetProcAddress(origModule, "waveOutGetPosition");
    waveOutGetVolume_orig = GetProcAddress(origModule, "waveOutGetVolume");
    waveOutMessage_orig = GetProcAddress(origModule, "waveOutMessage");
    waveOutOpen_orig = GetProcAddress(origModule, "waveOutOpen");
    waveOutPause_orig = GetProcAddress(origModule, "waveOutPause");
    waveOutPrepareHeader_orig = GetProcAddress(origModule, "waveOutPrepareHeader");
    waveOutReset_orig = GetProcAddress(origModule, "waveOutReset");
    waveOutRestart_orig = GetProcAddress(origModule, "waveOutRestart");
    waveOutSetPitch_orig = GetProcAddress(origModule, "waveOutSetPitch");
    waveOutSetPlaybackRate_orig = GetProcAddress(origModule, "waveOutSetPlaybackRate");
    waveOutSetVolume_orig = GetProcAddress(origModule, "waveOutSetVolume");
    waveOutUnprepareHeader_orig = GetProcAddress(origModule, "waveOutUnprepareHeader");
    waveOutWrite_orig = GetProcAddress(origModule, "waveOutWrite");
    mciExecute_orig = GetProcAddress(origModule, "mciExecute");
    mciGetErrorStringA_orig = GetProcAddress(origModule, "mciGetErrorStringA");
    mciGetErrorStringW_orig = GetProcAddress(origModule, "mciGetErrorStringW");
    mciSendCommandA_orig = GetProcAddress(origModule, "mciSendCommandA");
    mciSendCommandW_orig = GetProcAddress(origModule, "mciSendCommandW");
    mciSendStringA_orig = GetProcAddress(origModule, "mciSendStringA");
    mciSendStringW_orig = GetProcAddress(origModule, "mciSendStringW");
    mciFreeCommandResource_orig = GetProcAddress(origModule, "mciFreeCommandResource");
    mciLoadCommandResource_orig = GetProcAddress(origModule, "mciLoadCommandResource");
    mciDriverNotify_orig = GetProcAddress(origModule, "mciDriverNotify");
    mciDriverYield_orig = GetProcAddress(origModule, "mciDriverYield");
    mciGetCreatorTask_orig = GetProcAddress(origModule, "mciGetCreatorTask");
    mciGetDeviceIDA_orig = GetProcAddress(origModule, "mciGetDeviceIDA");
    mciGetDeviceIDFromElementIDA_orig = GetProcAddress(origModule, "mciGetDeviceIDFromElementIDA");
    mciGetDeviceIDFromElementIDW_orig = GetProcAddress(origModule, "mciGetDeviceIDFromElementIDW");
    mciGetDeviceIDW_orig = GetProcAddress(origModule, "mciGetDeviceIDW");
    mciGetDriverData_orig = GetProcAddress(origModule, "mciGetDriverData");
    mciGetYieldProc_orig = GetProcAddress(origModule, "mciGetYieldProc");
    mciSetDriverData_orig = GetProcAddress(origModule, "mciSetDriverData");
    mciSetYieldProc_orig = GetProcAddress(origModule, "mciSetYieldProc");
    PlaySoundA_orig = GetProcAddress(origModule, "PlaySoundA");
    sndPlaySoundA_orig = GetProcAddress(origModule, "sndPlaySoundA");
    sndPlaySoundW_orig = GetProcAddress(origModule, "sndPlaySoundW");
    WOWAppExit_orig = GetProcAddress(origModule, "WOWAppExit");
    mmsystemGetVersion_orig = GetProcAddress(origModule, "mmsystemGetVersion");

    return true;
}

void on_detach()
{
    if (is_wrapping_nvngx)
        proxy_nvngx::on_detach();

    if (!origModule)
        return;

    FreeLibrary(origModule);
    origModule = nullptr;
}

};
