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


#ifndef NVSDK_NGX_DEEPDVC_VK_H
#define NVSDK_NGX_DEEPDVC_VK_H

#include "nvsdk_ngx_defs_deepdvc.h"
#include "nvsdk_ngx_helpers_vk.h"

typedef struct NVSDK_NGX_VK_DEEPDVC_Eval_Params
{
    NVSDK_NGX_VK_Feature_Eval_Params    Feature;
    NVSDK_NGX_Coordinates               InColorSubrectBase = {0,0};
    NVSDK_NGX_Dimensions                InColorSubrectSize = {0,0};
    float                               Strength;
    float                               SaturationBoost;
} NVSDK_NGX_VK_DEEPDVC_Eval_Params;

static inline NVSDK_NGX_Result NGX_VULKAN_CREATE_DEEPDVC_EXT1(
    VkDevice InDevice,
    VkCommandBuffer InCmdList,
    unsigned int InCreationNodeMask,
    unsigned int InVisibilityNodeMask,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_Feature_Create_Params *pDeepDVCCreateParams)
{

    if (InDevice) return NVSDK_NGX_VULKAN_CreateFeature1(InDevice, InCmdList, NVSDK_NGX_Feature_DeepDVC, pInParams, ppOutHandle);
    else return NVSDK_NGX_VULKAN_CreateFeature(InCmdList, NVSDK_NGX_Feature_DeepDVC, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_VULKAN_CREATE_DEEPDVC_EXT(
    VkCommandBuffer InCmdList,
    unsigned int InCreationNodeMask,
    unsigned int InVisibilityNodeMask,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_Feature_Create_Params *pDeepDVCCreateParams)
{
    return NGX_VULKAN_CREATE_DEEPDVC_EXT1(NULL, InCmdList, InCreationNodeMask, InVisibilityNodeMask, ppOutHandle, pInParams, pDeepDVCCreateParams);
}


static inline NVSDK_NGX_Result NGX_VULKAN_EVALUATE_DEEPDVC_EXT(
    VkCommandBuffer InCmdList,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_VK_DEEPDVC_Eval_Params *pInDeepDVCEvalParams)
{
	NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDeepDVCEvalParams->Feature.pInOutput);
    
	NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_Color, pInDeepDVCEvalParams->Feature.pInColor);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X, pInDeepDVCEvalParams->InColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y, pInDeepDVCEvalParams->InColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, pInDeepDVCEvalParams->InColorSubrectSize.Width);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, pInDeepDVCEvalParams->InColorSubrectSize.Height);    
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DeepDVC_Strength, pInDeepDVCEvalParams->Strength);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DeepDVC_SaturationBoost, pInDeepDVCEvalParams->SaturationBoost);

    return NVSDK_NGX_VULKAN_EvaluateFeature_C(InCmdList, pInHandle, pInParams, NULL);
}


#endif // NVSDK_NGX_DEEPDVC_VK_H
