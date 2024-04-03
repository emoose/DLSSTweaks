#pragma once
/*
* Copyright (c) 2021 NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA Corporation and its licensors retain all intellectual property and proprietary
* rights in and to this software, related documentation and any modifications thereto.
* Any use, reproduction, disclosure or distribution of this software and related
* documentation without an express license agreement from NVIDIA Corporation is strictly
* prohibited.
*
* TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
* SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
* BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
* INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGES.
*/


#ifndef NVSDK_NGX_DLSSG_H
#define NVSDK_NGX_DLSSG_H

#include <stdint.h> // for uint64_t

#include "nvsdk_ngx.h" // DX11/12 here
#include "nvsdk_ngx_defs.h"

// Newer drivers (all drivers moving forward) declare these versions of the snippet info params
#define NVSDK_NGX_Parameter_FrameGeneration_Available                "FrameGeneration.Available"
#define NVSDK_NGX_Parameter_FrameGeneration_FeatureInitResult        "FrameGeneration.FeatureInitResult"
#define NVSDK_NGX_Parameter_FrameGeneration_MinDriverVersionMajor    "FrameGeneration.MinDriverVersionMajor"
#define NVSDK_NGX_Parameter_FrameGeneration_MinDriverVersionMinor    "FrameGeneration.MinDriverVersionMinor"
#define NVSDK_NGX_Parameter_FrameGeneration_NeedsUpdatedDriver       "FrameGeneration.NeedsUpdatedDriver"

// Some older drivers declare these versions of the snippet info params instead, so for now we make these
// available for parallel checking
#define NVSDK_NGX_Parameter_FrameInterpolation_Available                "FrameInterpolation.Available"
#define NVSDK_NGX_Parameter_FrameInterpolation_FeatureInitResult        "FrameInterpolation.FeatureInitResult"
#define NVSDK_NGX_Parameter_FrameInterpolation_MinDriverVersionMajor    "FrameInterpolation.MinDriverVersionMajor"
#define NVSDK_NGX_Parameter_FrameInterpolation_MinDriverVersionMinor    "FrameInterpolation.MinDriverVersionMinor"
#define NVSDK_NGX_Parameter_FrameInterpolation_NeedsUpdatedDriver       "FrameInterpolation.NeedsUpdatedDriver"

#define NVSDK_NGX_DLSSG_Parameter_BackbufferFormat           "DLSSG.BackbufferFormat"
#define NVSDK_NGX_DLSSG_Parameter_CreateTimelineSyncObjectsCallback      "DLSSG.CreateTimelineSyncObjectsCallback"
#define NVSDK_NGX_DLSSG_Parameter_CreateTimelineSyncObjectsCallbackData  "DLSSG.CreateTimelineSyncObjectsCallbackData"
#define NVSDK_NGX_DLSSG_Parameter_SyncSignalCallback         "DLSSG.SyncSignalCallback"
#define NVSDK_NGX_DLSSG_Parameter_SyncSignalCallbackData     "DLSSG.SyncSignalCallbackData"
#define NVSDK_NGX_DLSSG_Parameter_SyncWaitCallback           "DLSSG.SyncWaitCallback"
#define NVSDK_NGX_DLSSG_Parameter_SyncWaitCallbackData       "DLSSG.SyncWaitCallbackData"
#define NVSDK_NGX_DLSSG_Parameter_QueueSubmitCallback        "DLSSG.QueueSubmitCallback"
#define NVSDK_NGX_DLSSG_Parameter_QueueSubmitCallbackData    "DLSSG.QueueSubmitCallbackData"
#define NVSDK_NGX_DLSSG_Parameter_SyncSignalOnlyCallback     "DLSSG.SyncSignalOnlyCallback"
#define NVSDK_NGX_DLSSG_Parameter_SyncSignalOnlyCallbackData "DLSSG.SyncSignalOnlyCallbackData"
#define NVSDK_NGX_DLSSG_Parameter_SyncWaitOnlyCallback       "DLSSG.SyncWaitOnlyCallback"
#define NVSDK_NGX_DLSSG_Parameter_SyncWaitOnlyCallbackData   "DLSSG.SyncWaitOnlyCallbackData"
#define NVSDK_NGX_DLSSG_Parameter_SyncFlushCallback          "DLSSG.SyncFlushCallback"
#define NVSDK_NGX_DLSSG_Parameter_SyncFlushCallbackData      "DLSSG.SyncFlushCallbackData"

#define NVSDK_NGX_DLSSG_Parameter_Backbuffer                 "DLSSG.Backbuffer"
#define NVSDK_NGX_DLSSG_Parameter_MVecs                      "DLSSG.MVecs"
#define NVSDK_NGX_DLSSG_Parameter_Depth                      "DLSSG.Depth"
#define NVSDK_NGX_DLSSG_Parameter_HUDLess                    "DLSSG.HUDLess"
#define NVSDK_NGX_DLSSG_Parameter_UI                         "DLSSG.UI"
#define NVSDK_NGX_Parameter_OutputInterpolated              "DLSSG.OutputInterpolated"
#define NVSDK_NGX_Parameter_OutputReal                      "DLSSG.OutputReal"

#define NVSDK_NGX_DLSSG_Parameter_CmdQueue                   "DLSSG.CmdQueue"
#define NVSDK_NGX_DLSSG_Parameter_CmdAlloc                   "DLSSG.CmdAlloc"
#define NVSDK_NGX_DLSSG_Parameter_Fence_Event                "DLSSG.FenceEvent"
#define NVSDK_NGX_DLSSG_Parameter_IsRecording                "DLSSG.IsRecording"

#define NVSDK_NGX_DLSSG_Parameter_VkOFAModeRequest           "DLSSG.VkOFAModeRequest"

#define NVSDK_NGX_DLSSG_Parameter_EnableInterp               "DLSSG.EnableInterp"

#define NVSDK_NGX_DLSSG_Parameter_LowResMvecPass             "DLSSG.run_lowres_mvec_pass"

#define NVSDK_NGX_DLSSG_Invalid_Constants                    "DLSSG.is_constants_valid"

#define NVSDK_NGX_DLSSG_Parameter_FlushRequired              "DLSSG.FlushRequired"

//! IMPORTANT: All matrices should NOT contain temporal AA jitter offset
//! Specifies matrix transformation from the camera view to the clip space.
//! Float4x4 as void*
#define NVSDK_NGX_DLSSG_Parameter_CameraViewToClip           "DLSSG.CameraViewToClip"
//! Float4x4 as void*
#define NVSDK_NGX_DLSSG_Parameter_ClipToCameraView           "DLSSG.ClipToCameraView"
//! Float4x4 as void*
#define NVSDK_NGX_DLSSG_Parameter_ClipToLensClip             "DLSSG.ClipToLensClip"
//! Float4x4 as void*
#define NVSDK_NGX_DLSSG_Parameter_ClipToPrevClip             "DLSSG.ClipToPrevClip"
//! Float4x4 as void*
#define NVSDK_NGX_DLSSG_Parameter_PrevClipToClip             "DLSSG.PrevClipToClip"

//! Specifies clip space jitter offset
#define NVSDK_NGX_DLSSG_Parameter_JitterOffsetX              "DLSSG.JitterOffsetX"
#define NVSDK_NGX_DLSSG_Parameter_JitterOffsetY              "DLSSG.JitterOffsetY"

//! Specifies scale factors used to normalize motion vectors (so the values are in [-1,1] range)
#define NVSDK_NGX_DLSSG_Parameter_MvecScaleX                 "DLSSG.MvecScaleX"
#define NVSDK_NGX_DLSSG_Parameter_MvecScaleY                 "DLSSG.MvecScaleY"

//! Specifies camera pinhole offset.
#define NVSDK_NGX_DLSSG_Parameter_CameraPinholeOffsetX       "DLSSG.CameraPinholeOffsetX"
#define NVSDK_NGX_DLSSG_Parameter_CameraPinholeOffsetY       "DLSSG.CameraPinholeOffsetY"

//! Specifies camera position in world space.
#define NVSDK_NGX_DLSSG_Parameter_CameraPosX                 "DLSSG.CameraPosX"
#define NVSDK_NGX_DLSSG_Parameter_CameraPosY                 "DLSSG.CameraPosY"
#define NVSDK_NGX_DLSSG_Parameter_CameraPosZ                 "DLSSG.CameraPosZ"

//! Specifies camera up vector in world space.
#define NVSDK_NGX_DLSSG_Parameter_CameraUpX                  "DLSSG.CameraUpX"
#define NVSDK_NGX_DLSSG_Parameter_CameraUpY                  "DLSSG.CameraUpY"
#define NVSDK_NGX_DLSSG_Parameter_CameraUpZ                  "DLSSG.CameraUpZ"

//! Specifies camera right vector in world space.
#define NVSDK_NGX_DLSSG_Parameter_CameraRightX               "DLSSG.CameraRightX"
#define NVSDK_NGX_DLSSG_Parameter_CameraRightY               "DLSSG.CameraRightY"
#define NVSDK_NGX_DLSSG_Parameter_CameraRightZ               "DLSSG.CameraRightZ"

//! Specifies camera forward vector in world space.
#define NVSDK_NGX_DLSSG_Parameter_CameraFwdX                 "DLSSG.CameraFwdX"
#define NVSDK_NGX_DLSSG_Parameter_CameraFwdY                 "DLSSG.CameraFwdY"
#define NVSDK_NGX_DLSSG_Parameter_CameraFwdZ                 "DLSSG.CameraFwdZ"

//! Specifies camera near view plane distance.
#define NVSDK_NGX_DLSSG_Parameter_CameraNear                 "DLSSG.CameraNear"

//! Specifies camera far view plane distance.
#define NVSDK_NGX_DLSSG_Parameter_CameraFar                  "DLSSG.CameraFar"

//! Specifies camera field of view in radians.
#define NVSDK_NGX_DLSSG_Parameter_CameraFOV                  "DLSSG.CameraFOV"

//! Specifies camera aspect ratio defined as view space width divided by height.
#define NVSDK_NGX_DLSSG_Parameter_CameraAspectRatio          "DLSSG.CameraAspectRatio"

//! Specifies if tagged color buffers are full HDR (rendering to an HDR monitor) or not 
#define NVSDK_NGX_DLSSG_Parameter_ColorBuffersHDR            "DLSSG.ColorBuffersHDR"

//! Specifies if depth values are inverted (value closer to the camera is higher) or not.
#define NVSDK_NGX_DLSSG_Parameter_DepthInverted              "DLSSG.DepthInverted"

//! Specifies if camera motion is included in the MVec buffer.
#define NVSDK_NGX_DLSSG_Parameter_CameraMotionIncluded       "DLSSG.CameraMotionIncluded"

//! Specifies if previous frame has no connection to the current one (motion vectors are invalid)
#define NVSDK_NGX_DLSSG_Parameter_Reset                      "DLSSG.Reset"

//! Specifies if application is not currently rendering game frames (paused in menu, playing video cut-scenes)
#define NVSDK_NGX_DLSSG_Parameter_NotRenderingGameFrames     "DLSSG.NotRenderingGameFrames"

//! Specifies if orthographic projection is used or not.
#define NVSDK_NGX_DLSSG_Parameter_OrthoProjection            "DLSSG.OrthoProjection"

#define NVSDK_NGX_Parameter_DLSSGGetCurrentSettingsCallback  "DLSSG.GetCurrentSettingsCallback"
#define NVSDK_NGX_Parameter_DLSSGEstimateVRAMCallback        "DLSSG.EstimateVRAMCallback"

#define NVSDK_NGX_Parameter_DLSSGMustCallEval                "DLSSG.MustCallEval"
#define NVSDK_NGX_Parameter_DLSSGBurstCaptureRunning         "DLSSG.BurstCaptureRunning"

#define NVSDK_NGX_Parameter_DLSSGInvertXAxis          "DLSSG.InvertXAxis"
#define NVSDK_NGX_Parameter_DLSSGInvertYAxis          "DLSSG.InvertYAxis"

#define NVSDK_NGX_Parameter_DLSSGUserDebugText       "DLSSG.UserDebugText"

#define NVSDK_NGX_DLSSG_Parameter_MvecInvalidValue   "DLSSG.MvecInvalidValue"
#define NVSDK_NGX_DLSSG_Parameter_MvecDilated        "DLSSG.MvecDilated"

#define NVSDK_NGX_DLSSG_Parameter_MVecsSubrectBaseX "DLSSG.MVecsSubrectBaseX"
#define NVSDK_NGX_DLSSG_Parameter_MVecsSubrectBaseY "DLSSG.MVecsSubrectBaseY"
#define NVSDK_NGX_DLSSG_Parameter_MVecsSubrectWidth "DLSSG.MVecsSubrectWidth"
#define NVSDK_NGX_DLSSG_Parameter_MVecsSubrectHeight "DLSSG.MVecsSubrectHeight"
#define NVSDK_NGX_DLSSG_Parameter_DepthSubrectBaseX "DLSSG.DepthSubrectBaseX"
#define NVSDK_NGX_DLSSG_Parameter_DepthSubrectBaseY "DLSSG.DepthSubrectBaseY"
#define NVSDK_NGX_DLSSG_Parameter_DepthSubrectWidth "DLSSG.DepthSubrectWidth"
#define NVSDK_NGX_DLSSG_Parameter_DepthSubrectHeight "DLSSG.DepthSubrectHeight"

#define NVSDK_NGX_DLSSG_Parameter_HUDLessSubrectBaseX "DLSSG.HUDLessSubrectBaseX"
#define NVSDK_NGX_DLSSG_Parameter_HUDLessSubrectBaseY "DLSSG.HUDLessSubrectBaseY"
#define NVSDK_NGX_DLSSG_Parameter_HUDLessSubrectWidth "DLSSG.HUDLessSubrectWidth"
#define NVSDK_NGX_DLSSG_Parameter_HUDLessSubrectHeight "DLSSG.HUDLessSubrectHeight"
#define NVSDK_NGX_DLSSG_Parameter_UISubrectBaseX "DLSSG.UISubrectBaseX"
#define NVSDK_NGX_DLSSG_Parameter_UISubrectBaseY "DLSSG.UISubrectBaseY"
#define NVSDK_NGX_DLSSG_Parameter_UISubrectWidth "DLSSG.UISubrectWidth"
#define NVSDK_NGX_DLSSG_Parameter_UISubrectHeight "DLSSG.UISubrectHeight"

//! Specifies the internal resolution to be used (mainly for dynamic resolution)
#define NVSDK_NGX_DLSSG_Parameter_InternalWidth      "DLSSG.InternalWidth"
#define NVSDK_NGX_DLSSG_Parameter_InternalHeight     "DLSSG.InternalHeight"

//! If nonzero, we expect to use dynamic resolution (MVec and Depth res will change, possibly per frame)
#define NVSDK_NGX_DLSSG_Parameter_DynamicResolution  "DLSSG.DynamicResolution"

// Specifies an external module callback for handling keypress logic.
#define NVSDK_NGX_DLSSG_Parameter_KeyEventHandler    "DLSSG.key_event_handler"
#define NVSDK_NGX_DLSSG_Parameter_KeyEventHandlerCtx "DLSSG.key_event_handler_ctx"
typedef void (NVSDK_CONV* NVSDK_NGX_DLSSG_KeyEventHandlerCallback)(void* app_context, const char* event_str, const char* key_str);

/*** D3D12 ***/

// This must match the typedef in DLSSGCore; better yet, we should move this into a DLSSGCore header and
// include it.

// Mandatory callback implementations to be provided by the SDK client

/* This callback must
    1. Create D3D12 RHI resources for signal and wait fence objects using the corresponding passed-in initial fence values.
    2. Store them along with their initial fence values in the app_context.
    3. Save these resource-pointers in the corresponding passed-in signal and wait fence object pointers.
*/
typedef void (NVSDK_CONV * PFN_NVSDK_NGX_DLSSG_AppCreateTimelineSyncObjectsCallback)(
    void* app_context,
    void** pp_sync_obj_signal,
    uint64_t sync_obj_signal_value,
    void** pp_sync_obj_wait,
    uint64_t sync_obj_wait_value);

/* This callback must
    1. Do error-handling of passed-in pointers.
    2. Track the passed-in CL and sync_obj object to signal on against the ones stored in this callback's app_context.
    3. Close CL if it's in recording state.
    4. Execute CL
    5. Reset CL and confirm it's in recording state again.
    6. Save the passed-in sync_obj-value in the callback's app_context and do necessary signaling.
    7. Save the reset CL pointer or new CL pointer in the passed-in CL pointer for subsequent use by DLSSG.
    8. Note that SDK client may choose to defer execution of steps 4, 5, and 6 and may therefore queue them as necessary.
*/
typedef void (NVSDK_CONV * PFN_NVSDK_NGX_DLSSG_AppSyncSignalCallback)(
    void* app_context,
    void** pp_cmd_list,
    void* sync_obj_signal,
    uint64_t sync_obj_signal_value);

/* This callback must
    1. Do error-handling of passed-in pointers (signal_sync_obj can be NULL and to be ignored if so).
    2. Track the passed-in CL and sync_obj objects to signal/wait on against the ones stored in this callback's app_context.
    3. Close CL if it's in recording state.
    4. Execute CL
    5. Reset CL and confirm it's in recording state again.
    6. Optionally save the passed-in signal sync_obj-value in the callback's app_context and do necessary signaling, if required to track the CL submission.
    7. Save the passed-in wait sync_obj-value in the callback's app_context and do necessary GPU or CPU wait on that value.
    8. Save the reset CL pointer or new CL pointer in the passed-in CL pointer for subsequent use by DLSSG.
    9. Note that SDK client may choose to defer execution of steps 4, 5, 6 and 7 and may therefore queue them as necessary.
*/
typedef void (NVSDK_CONV * PFN_NVSDK_NGX_DLSSG_AppSyncWaitCallback)(
    void* app_context,
    void** pp_cmd_list,
    void* sync_obj_wait,
    uint64_t sync_obj_wait_value,
    int wait_cpu,
    void* sync_obj_signal,
    uint64_t sync_obj_signal_value);

// Following are optional callback implementations to be provided by the SDK client - useful for in-frame debugging.

/* This callback is expected to
    1. Do error-handling of passed-in pointers.
    2. Track the passed-in CL against the one stored in this callback's app_context.
    3. Close CL if it's in recording state.
    4. Execute CL
    5. Reset CL and confirm it's in recording state again.
    6. Save the reset CL pointer or new CL pointer in the passed-in CL pointer for subsequent use by DLSSG.
    7. Note that SDK client may choose to defer execution of steps 4 and 5 and may therefore queue them as necessary.
*/
typedef void (NVSDK_CONV * PFN_NVSDK_NGX_DLSSG_AppQueueSubmitCallback)(
    void* app_context,
    void** pp_cmd_list);

/* This callback must
    1. Do error-handling of passed-in pointers.
    2. Save the passed-in signal sync_obj-value in the callback's app_context and do necessary signaling.
    3. Note that SDK client may choose to defer its execution and therefore only queue it as necessary.
*/
typedef void (NVSDK_CONV * PFN_NVSDK_NGX_DLSSG_AppSyncSignalOnlyCallback)(
    void* app_context,
    void* sync_obj_signal,
    uint64_t sync_obj_signal_value);

/* This callback must
    1. Do error-handling of passed-in pointers.
    2. Save the passed-in wait sync_obj-value in the callback's app_context and do necessary GPU or CPU wait on that value.
    3. Note that SDK client may choose to defer its execution and therefore only queue it as necessary.
*/
typedef void (NVSDK_CONV * PFN_NVSDK_NGX_DLSSG_AppSyncWaitOnlyCallback)(
    void* app_context,
    void* sync_obj_wait,
    uint64_t sync_obj_wait_value,
    int wait_cpu);

/* This callback must
    1. Do error-handling of passed-in pointers.
    2. Track the passed-in CL and sync_obj object to signal on against the ones stored in this callback's app_context.
    3. Close CL if it's in recording state.
    4. Execute CL
    5. Reset CL and confirm it's in recording state again.
    6. Save the passed-in signal sync_obj-value in the callback's app_context and do necessary signaling.
    7. Do necessary GPU or CPU wait for the same sync_obj-value.
    8. Save the reset CL pointer or new CL pointer in the passed-in CL pointer for subsequent use by DLSSG.
    9. Note that none of these steps can be deferred since the intent of this callback is immediate CL execution.
*/
typedef void (NVSDK_CONV * PFN_NVSDK_NGX_DLSSG_AppSyncFlushCallback)(
    void* app_context,
    void** pp_cmd_list,
    void* sync_obj_signal,
    uint64_t sync_obj_signal_value,
    int wait_cpu);

typedef struct NVSDK_NGX_DLSSG_Create_Params
{
    void* CreateTimelineSyncObjectsCallbackData;
    PFN_NVSDK_NGX_DLSSG_AppCreateTimelineSyncObjectsCallback CreateTimelineSyncObjectsCallback;
    void* SyncSignalCallbackData;
    PFN_NVSDK_NGX_DLSSG_AppSyncSignalCallback SyncSignalCallback;
    void* SyncWaitCallbackData;
    PFN_NVSDK_NGX_DLSSG_AppSyncWaitCallback SyncWaitCallback;
    void* QueueSubmitCallbackData;
    PFN_NVSDK_NGX_DLSSG_AppQueueSubmitCallback QueueSubmitCallback;
    void* SyncSignalOnlyCallbackData;
    PFN_NVSDK_NGX_DLSSG_AppSyncSignalOnlyCallback SyncSignalOnlyCallback;
    void* SyncWaitOnlyCallbackData;
    PFN_NVSDK_NGX_DLSSG_AppSyncWaitOnlyCallback SyncWaitOnlyCallback;
    void* SyncFlushCallbackData;
    PFN_NVSDK_NGX_DLSSG_AppSyncFlushCallback SyncFlushCallback;
    unsigned int Width;
    unsigned int Height;
    unsigned int NativeBackbufferFormat;
} NVSDK_NGX_DLSSG_Create_Params;

typedef struct NVSDK_NGX_D3D12_DLSSG_Eval_Params
{
    ID3D12Resource* pBackbuffer;
    ID3D12Resource* pDepth;
    ID3D12Resource* pMVecs;
    ID3D12Resource* pHudless; // Optional
    ID3D12Resource* pUI; // Optional
    ID3D12Resource* pOutputInterpFrame;
    ID3D12Resource* pOutputRealFrame; // In some cases, the feature may modify this frame (e.g. debugging)
} NVSDK_NGX_D3D12_DLSSG_Eval_Params;

typedef struct NVSDK_NGX_DLSSG_Opt_Eval_Params
{
    //! IMPORTANT: All matrices should NOT contain temporal AA jitter offset
    //! Specifies matrix transformation from the camera view to the clip space.
    float cameraViewToClip[4][4];
    //! Specifies matrix transformation from the clip space to the camera view space.
    float clipToCameraView[4][4];
    //! Specifies matrix transformation describing lens distortion in clip space.
    float clipToLensClip[4][4];
    //! Specifies matrix transformation from the current clip to the previous clip space.
    float clipToPrevClip[4][4];
    //! Specifies matrix transformation from the previous clip to the current clip space.
    float prevClipToClip[4][4];

    //! Specifies clip space jitter offset
    float jitterOffset[2];
    //! Specifies scale factors used to normalize motion vectors (so the values are in [-1,1] range)
    float mvecScale[2];
    //! Specifies camera pinhole offset.
    float cameraPinholeOffset[2];
    //! Specifies camera position in world space.
    float cameraPos[3];
    //! Specifies camera up vector in world space.
    float cameraUp[3];
    //! Specifies camera right vector in world space.
    float cameraRight[3];
    //! Specifies camera forward vector in world space.
    float cameraFwd[3];

    //! Specifies camera near view plane distance.
    float cameraNear;
    //! Specifies camera far view plane distance.
    float cameraFar;
    //! Specifies camera field of view in radians.
    float cameraFOV;
    //! Specifies camera aspect ratio defined as view space width divided by height.
    float cameraAspectRatio;

    //! Specifies if tagged color buffers are full HDR (rendering to an HDR monitor) or not 
    bool colorBuffersHDR;
    //! Specifies if depth values are inverted (value closer to the camera is higher) or not.
    bool depthInverted;
    //! Specifies if camera motion is included in the MVec buffer.
    bool cameraMotionIncluded;
    //! Specifies if previous frame has no connection to the current one (motion vectors are invalid)
    bool reset;
    //! Specifies if application is not currently rendering game frames (paused in menu, playing video cut-scenes)
    bool notRenderingGameFrames;
    //! Specifies if orthographic projection is used or not.
    bool orthoProjection;

    //! Specifies which value represents an invalid (un-initialized) value in the motion vectors buffer
    float motionVectorsInvalidValue;
    //! Specifies if motion vectors are already dilated or not.
    bool motionVectorsDilated;

    NVSDK_NGX_Coordinates mvecsSubrectBase = { 0, 0 };
    NVSDK_NGX_Dimensions mvecsSubrectSize = { 0, 0 };
    NVSDK_NGX_Coordinates depthSubrectBase = { 0, 0 };
    NVSDK_NGX_Dimensions depthSubrectSize = { 0, 0 };
    NVSDK_NGX_Coordinates hudLessSubrectBase = { 0, 0 };
    NVSDK_NGX_Dimensions hudLessSubrectSize = { 0, 0 };
    NVSDK_NGX_Coordinates uiSubrectBase = { 0, 0 };
    NVSDK_NGX_Dimensions uiSubrectSize = { 0, 0 };

} NVSDK_NGX_DLSSG_Opt_Eval_Params;

static inline NVSDK_NGX_Result NGX_D3D12_CREATE_DLSSG(
    ID3D12GraphicsCommandList *pInCmdList,
    unsigned int InCreationNodeMask,
    unsigned int InVisibilityNodeMask,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_DLSSG_Create_Params *pInDlssgCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_CreationNodeMask, InCreationNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_VisibilityNodeMask, InVisibilityNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, pInDlssgCreateParams->Width);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, pInDlssgCreateParams->Height);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_BackbufferFormat, pInDlssgCreateParams->NativeBackbufferFormat);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_CreateTimelineSyncObjectsCallbackData, pInDlssgCreateParams->CreateTimelineSyncObjectsCallbackData);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_CreateTimelineSyncObjectsCallback, pInDlssgCreateParams->CreateTimelineSyncObjectsCallback);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncSignalCallbackData, pInDlssgCreateParams->SyncSignalCallbackData);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncSignalCallback, pInDlssgCreateParams->SyncSignalCallback);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncWaitCallbackData, pInDlssgCreateParams->SyncWaitCallbackData);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncWaitCallback, pInDlssgCreateParams->SyncWaitCallback);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_QueueSubmitCallbackData, pInDlssgCreateParams->QueueSubmitCallbackData);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_QueueSubmitCallback, pInDlssgCreateParams->QueueSubmitCallback);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncSignalOnlyCallbackData, pInDlssgCreateParams->SyncSignalOnlyCallbackData);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncSignalOnlyCallback, pInDlssgCreateParams->SyncSignalOnlyCallback);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncWaitOnlyCallbackData, pInDlssgCreateParams->SyncWaitOnlyCallbackData);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncWaitOnlyCallback, pInDlssgCreateParams->SyncWaitOnlyCallback);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncFlushCallbackData, pInDlssgCreateParams->SyncFlushCallbackData);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_SyncFlushCallback, pInDlssgCreateParams->SyncFlushCallback);

    return NVSDK_NGX_D3D12_CreateFeature(pInCmdList, NVSDK_NGX_Feature_FrameGeneration, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_D3D12_EVALUATE_DLSSG(
    ID3D12GraphicsCommandList *pInCmdList,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_D3D12_DLSSG_Eval_Params *pInDlssgEvalParams,
    NVSDK_NGX_DLSSG_Opt_Eval_Params * pInDlssgOptEvalParams)
{
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_DLSSG_Parameter_Backbuffer, pInDlssgEvalParams->pBackbuffer);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_DLSSG_Parameter_MVecs, pInDlssgEvalParams->pMVecs);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_DLSSG_Parameter_Depth, pInDlssgEvalParams->pDepth);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_DLSSG_Parameter_HUDLess, pInDlssgEvalParams->pHudless);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_DLSSG_Parameter_UI, pInDlssgEvalParams->pUI);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_OutputInterpolated, pInDlssgEvalParams->pOutputInterpFrame);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_OutputReal, pInDlssgEvalParams->pOutputRealFrame);

    if (pInDlssgOptEvalParams)
    {
        NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraViewToClip, pInDlssgOptEvalParams->cameraViewToClip);
        NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_ClipToCameraView, pInDlssgOptEvalParams->clipToCameraView);
        NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_ClipToLensClip, pInDlssgOptEvalParams->clipToLensClip);
        NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_ClipToPrevClip, pInDlssgOptEvalParams->clipToPrevClip);
        NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_PrevClipToClip, pInDlssgOptEvalParams->prevClipToClip);

        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_JitterOffsetX, pInDlssgOptEvalParams->jitterOffset[0]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_JitterOffsetY, pInDlssgOptEvalParams->jitterOffset[1]);

        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_MvecScaleX, pInDlssgOptEvalParams->mvecScale[0]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_MvecScaleY, pInDlssgOptEvalParams->mvecScale[1]);

        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraPinholeOffsetX, pInDlssgOptEvalParams->cameraPinholeOffset[0]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraPinholeOffsetY, pInDlssgOptEvalParams->cameraPinholeOffset[1]);

        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraPosX, pInDlssgOptEvalParams->cameraPos[0]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraPosY, pInDlssgOptEvalParams->cameraPos[1]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraPosZ, pInDlssgOptEvalParams->cameraPos[2]);

        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraUpX, pInDlssgOptEvalParams->cameraUp[0]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraUpY, pInDlssgOptEvalParams->cameraUp[1]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraUpZ, pInDlssgOptEvalParams->cameraUp[2]);

        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraRightX, pInDlssgOptEvalParams->cameraRight[0]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraRightY, pInDlssgOptEvalParams->cameraRight[1]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraRightZ, pInDlssgOptEvalParams->cameraRight[2]);

        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraFwdX, pInDlssgOptEvalParams->cameraFwd[0]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraFwdY, pInDlssgOptEvalParams->cameraFwd[1]);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraFwdZ, pInDlssgOptEvalParams->cameraFwd[2]);

        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraNear, pInDlssgOptEvalParams->cameraNear);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraFar, pInDlssgOptEvalParams->cameraFar);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraFOV, pInDlssgOptEvalParams->cameraFOV);
        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraAspectRatio, pInDlssgOptEvalParams->cameraAspectRatio);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_ColorBuffersHDR, pInDlssgOptEvalParams->colorBuffersHDR);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_DepthInverted, pInDlssgOptEvalParams->depthInverted);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_CameraMotionIncluded, pInDlssgOptEvalParams->cameraMotionIncluded);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_Reset, pInDlssgOptEvalParams->reset);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_NotRenderingGameFrames, pInDlssgOptEvalParams->notRenderingGameFrames);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_OrthoProjection, pInDlssgOptEvalParams->orthoProjection);

        NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_DLSSG_Parameter_MvecInvalidValue, pInDlssgOptEvalParams->motionVectorsInvalidValue);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_MvecDilated, pInDlssgOptEvalParams->motionVectorsDilated);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_MVecsSubrectBaseX, pInDlssgOptEvalParams->mvecsSubrectBase.X);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_MVecsSubrectBaseY, pInDlssgOptEvalParams->mvecsSubrectBase.Y);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_MVecsSubrectWidth, pInDlssgOptEvalParams->mvecsSubrectSize.Width);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_MVecsSubrectHeight, pInDlssgOptEvalParams->mvecsSubrectSize.Height);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_DepthSubrectBaseX, pInDlssgOptEvalParams->depthSubrectBase.X);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_DepthSubrectBaseY, pInDlssgOptEvalParams->depthSubrectBase.Y);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_DepthSubrectWidth, pInDlssgOptEvalParams->depthSubrectSize.Width);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_DepthSubrectHeight, pInDlssgOptEvalParams->depthSubrectSize.Height);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_HUDLessSubrectBaseX, pInDlssgOptEvalParams->hudLessSubrectBase.X);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_HUDLessSubrectBaseY, pInDlssgOptEvalParams->hudLessSubrectBase.Y);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_HUDLessSubrectWidth, pInDlssgOptEvalParams->hudLessSubrectSize.Width);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_HUDLessSubrectHeight, pInDlssgOptEvalParams->hudLessSubrectSize.Height);

        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_UISubrectBaseX, pInDlssgOptEvalParams->uiSubrectBase.X);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_UISubrectBaseY, pInDlssgOptEvalParams->uiSubrectBase.Y);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_UISubrectWidth, pInDlssgOptEvalParams->uiSubrectSize.Width);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_DLSSG_Parameter_UISubrectHeight, pInDlssgOptEvalParams->uiSubrectSize.Height);
    }

    return NVSDK_NGX_D3D12_EvaluateFeature_C(pInCmdList, pInHandle, pInParams, NULL);
}

typedef NVSDK_NGX_Result(NVSDK_CONV* PFN_NVSDK_NGX_DLSSG_GetCurrentSettingsCallback)(const NVSDK_NGX_Handle* InHandle, NVSDK_NGX_Parameter* InParams);

static inline NVSDK_NGX_Result NGX_D3D12_GET_CURRENT_SETTINGS_DLSSG(const NVSDK_NGX_Handle* InHandle,
    NVSDK_NGX_Parameter* InParams,
    unsigned int* pMustCallEval,
    unsigned int* pBurstCaptureRunning = nullptr
    )
{
    void* Callback = NULL;
    NVSDK_NGX_Parameter_GetVoidPointer(InParams, NVSDK_NGX_Parameter_DLSSGGetCurrentSettingsCallback, &Callback);
    if (!Callback)
    {
        // Possible reasons for this:
        // - Installed feature is out of date and does not support the feature we need
        // - You used NVSDK_NGX_AllocateParameters() for creating InParams. Try using NVSDK_NGX_GetCapabilityParameters() instead
        return NVSDK_NGX_Result_FAIL_OutOfDate;
    }

    NVSDK_NGX_Result Res = NVSDK_NGX_Result_Success;
    PFN_NVSDK_NGX_DLSSG_GetCurrentSettingsCallback PFNCallback = (PFN_NVSDK_NGX_DLSSG_GetCurrentSettingsCallback)Callback;
    Res = PFNCallback(InHandle, InParams);
    if (NVSDK_NGX_FAILED(Res))
    {
        return Res;
    }

    if (pBurstCaptureRunning)
    {
        if (NVSDK_NGX_FAILED(NVSDK_NGX_Parameter_GetUI(InParams, NVSDK_NGX_Parameter_DLSSGBurstCaptureRunning, pBurstCaptureRunning)))
            *pBurstCaptureRunning = 0;
    }

    return NVSDK_NGX_Parameter_GetUI(InParams, NVSDK_NGX_Parameter_DLSSGMustCallEval, pMustCallEval);
}

typedef NVSDK_NGX_Result(NVSDK_CONV* PFN_NVSDK_NGX_DLSSG_EstimateVRAMCallback)(uint32_t mvecDepthWidth, uint32_t mvecDepthHeight,
    uint32_t colorWidth, uint32_t colorHeight,
    uint32_t colorBufferFormat,
    uint32_t mvecBufferFormat, uint32_t depthBufferFormat,
    uint32_t hudLessBufferFormat, uint32_t uiBufferFormat, size_t* estimatedVRAMInBytes);

static inline NVSDK_NGX_Result NGX_D3D12_ESTIMATE_VRAM_DLSSG(
    NVSDK_NGX_Parameter* InParams,
    uint32_t mvecDepthWidth, uint32_t mvecDepthHeight,
    uint32_t colorWidth, uint32_t colorHeight,
    uint32_t colorBufferFormat, 
    uint32_t mvecBufferFormat, uint32_t depthBufferFormat,
    uint32_t hudLessBufferFormat, uint32_t uiBufferFormat,
    size_t* estimatedVRAMInBytes
)
{
    void* Callback = NULL;
    NVSDK_NGX_Parameter_GetVoidPointer(InParams, NVSDK_NGX_Parameter_DLSSGEstimateVRAMCallback, &Callback);
    if (!Callback)
    {
        // Possible reasons for this:
        // - Installed feature is out of date and does not support the feature we need
        // - You used NVSDK_NGX_AllocateParameters() for creating InParams. Try using NVSDK_NGX_GetCapabilityParameters() instead
        return NVSDK_NGX_Result_FAIL_OutOfDate;
    }

    NVSDK_NGX_Result Res = NVSDK_NGX_Result_Success;
    PFN_NVSDK_NGX_DLSSG_EstimateVRAMCallback PFNCallback = (PFN_NVSDK_NGX_DLSSG_EstimateVRAMCallback)Callback;
    Res = PFNCallback(mvecDepthWidth, mvecDepthHeight, colorWidth, colorHeight,
        colorBufferFormat, mvecBufferFormat, depthBufferFormat, hudLessBufferFormat, uiBufferFormat, estimatedVRAMInBytes);
    if (NVSDK_NGX_FAILED(Res))
    {
        return Res;
    }

    return NVSDK_NGX_Result_Success;
}

#endif
