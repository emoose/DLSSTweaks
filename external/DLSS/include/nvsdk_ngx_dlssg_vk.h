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


#ifndef NVSDK_NGX_DLSSG_VK_H
#define NVSDK_NGX_DLSSG_VK_H

#include <vulkan/vulkan.h>
// TODO TBD: Refactor these before we ship VK in the SDK so that _dlssg.h is shared items, and D3D12 goes in its own header
#include "nvsdk_ngx_dlssg.h"
#include "nvsdk_ngx_vk.h" // VK here

// Native VK OFA needs this for malloc/free calls below
#include <cstdlib>

typedef enum NVSDK_NGX_VK_OFA_MODE_REQUEST
{
	INTEROP = 0,
	NATIVE,
	AUTO,
	MODE_NUM
} NVSDK_NGX_VK_OFA_MODE_REQUEST;

typedef struct NVSDK_NGX_VK_DLSSG_Eval_Params
{
    NVSDK_NGX_Resource_VK* pBackbuffer;
    NVSDK_NGX_Resource_VK* pDepth;
    NVSDK_NGX_Resource_VK* pMVecs;
    NVSDK_NGX_Resource_VK* pHudless; // Optional
    NVSDK_NGX_Resource_VK* pUI; // Optional
    NVSDK_NGX_Resource_VK* pOutputInterpFrame;
    NVSDK_NGX_Resource_VK* pOutputRealFrame; // In some cases, the feature may modify this frame (e.g. debugging)
} NVSDK_NGX_VK_DLSSG_Eval_Params;

static inline NVSDK_NGX_Result NGX_VK_CREATE_DLSSG(
    VkCommandBuffer pInCmdBuf,
    unsigned int InCreationNodeMask,
    unsigned int InVisibilityNodeMask,
    NVSDK_NGX_Handle** ppOutHandle,
    NVSDK_NGX_Parameter* pInParams,
    NVSDK_NGX_DLSSG_Create_Params* pInDlssgCreateParams)
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

    return NVSDK_NGX_VULKAN_CreateFeature(pInCmdBuf, NVSDK_NGX_Feature_FrameGeneration, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_VK_EVALUATE_DLSSG(
    VkCommandBuffer pInCmdBuf,
    NVSDK_NGX_Handle* pInHandle,
    NVSDK_NGX_Parameter* pInParams,
    NVSDK_NGX_VK_DLSSG_Eval_Params* pInDlssgEvalParams,
    NVSDK_NGX_DLSSG_Opt_Eval_Params* pInDlssgOptEvalParams)
{
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_Backbuffer, pInDlssgEvalParams->pBackbuffer);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_MVecs, pInDlssgEvalParams->pMVecs);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_Depth, pInDlssgEvalParams->pDepth);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_HUDLess, pInDlssgEvalParams->pHudless);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_DLSSG_Parameter_UI, pInDlssgEvalParams->pUI);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_OutputInterpolated, pInDlssgEvalParams->pOutputInterpFrame);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_OutputReal, pInDlssgEvalParams->pOutputRealFrame);

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

    // C version or cpp?
    return NVSDK_NGX_VULKAN_EvaluateFeature_C(pInCmdBuf, pInHandle, pInParams, NULL);
}

static inline NVSDK_NGX_Result NGX_VK_GET_CURRENT_SETTINGS_DLSSG(const NVSDK_NGX_Handle* InHandle,
    NVSDK_NGX_Parameter* InParams,
    unsigned int* pMustCallEval
)
{
    void* Callback = NULL;
    NVSDK_NGX_Parameter_GetVoidPointer(InParams, NVSDK_NGX_Parameter_DLSSGGetCurrentSettingsCallback, &Callback);
    if (!Callback)
    {
        // Possible reasons for this:
        // - Installed DLSSG is out of date and does not support the feature we need
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

    return NVSDK_NGX_Parameter_GetUI(InParams, NVSDK_NGX_Parameter_DLSSGMustCallEval, pMustCallEval);
}

static inline NVSDK_NGX_Result NGX_VK_GET_NATIVE_OPTICAL_FLOW_QUEUE_INFO(VkPhysicalDevice physicalDevice, uint32_t* queueFamilyIndex, uint32_t* queueIndex)
{
	NVSDK_NGX_Result Res = NVSDK_NGX_Result_Success;

	if (queueFamilyIndex == nullptr || queueIndex == nullptr)
	{
		return NVSDK_NGX_Result_Fail;
	}

	(*queueFamilyIndex) = 0;
	(*queueIndex) = 0;

	VkPhysicalDeviceOpticalFlowFeaturesNV physicalDeviceOpticalFlowFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV };
	VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &physicalDeviceOpticalFlowFeatures };

	vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeatures2);

	int isNativeOpticalFlowSupported = static_cast<int>(!!physicalDeviceOpticalFlowFeatures.opticalFlow);
	if (!isNativeOpticalFlowSupported)
	{
		return NVSDK_NGX_Result_Fail;
	}

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
	VkQueueFamilyProperties* queueProps = (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
	if (queueProps == NULL)
	{
		return NVSDK_NGX_Result_Fail;
	}
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueProps);

	// Native OFA always runs on the very first queue of the very first optical flow-capable queue family.
	// Native OFA queue family cannot be the same as that of its client
	VkQueueFlags requiredCaps = VK_QUEUE_OPTICAL_FLOW_BIT_NV;
	for ((*queueFamilyIndex) = 0; (*queueFamilyIndex) < queueFamilyCount && (queueProps[*queueFamilyIndex].queueFlags & requiredCaps) != requiredCaps; (*queueFamilyIndex)++);

	free(queueProps);
	queueProps = NULL;

	if ((*queueFamilyIndex) == queueFamilyCount)
	{
		return NVSDK_NGX_Result_Fail;
	}

	return Res;
}
#endif
