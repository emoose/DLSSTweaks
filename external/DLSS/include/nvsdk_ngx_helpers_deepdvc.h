/*
 * SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#ifndef NVSDK_NGX_HELPERS_DEEPDVC_H
#define NVSDK_NGX_HELPERS_DEEPDVC_H
#pragma once

#include "nvsdk_ngx_helpers.h"
#include "nvsdk_ngx_defs_deepdvc.h"

typedef NVSDK_NGX_Result(NVSDK_CONV *PFN_NVSDK_NGX_DeepDVC_GetStatsCallback)(NVSDK_NGX_Parameter *InParams);

typedef struct NVSDK_NGX_D3D11_DEEPDVC_Eval_Params
{
    ID3D11Resource*                     pInColor;
    NVSDK_NGX_Coordinates               InColorSubrectBase = {0,0};
    NVSDK_NGX_Dimensions                InColorSubrectSize = {0,0};
    float                               Strength;
    float                               SaturationBoost;
} NVSDK_NGX_D3D11_DEEPDVC_Eval_Params;

typedef struct NVSDK_NGX_CUDA_DEEPDVC_Eval_Params
{
    CUtexObject*                        pInColor;
    NVSDK_NGX_Coordinates               InColorSubrectBase = {0,0};
    NVSDK_NGX_Dimensions                InColorSubrectSize = {0,0};
    float                               Strength;
    float                               SaturationBoost;
} NVSDK_NGX_CUDA_DEEPDVC_Eval_Params;

typedef struct NVSDK_NGX_D3D12_DEEPDVC_Eval_Params
{
    ID3D12Resource*                     pInColor;
    NVSDK_NGX_Coordinates               InColorSubrectBase = {0,0};
    NVSDK_NGX_Dimensions                InColorSubrectSize = {0,0};   
    float                               Strength;
    float                               SaturationBoost;
} NVSDK_NGX_D3D12_DEEPDVC_Eval_Params;

static inline NVSDK_NGX_Result NGX_D3D11_CREATE_DEEPDVC_EXT(
    ID3D11DeviceContext *pInCtx,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_Feature_Create_Params *pDeepDVCCreateParams)
{
    return NVSDK_NGX_D3D11_CreateFeature(pInCtx, NVSDK_NGX_Feature_DeepDVC, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_CUDA_CREATE_DEEPDVC_EXT(
    NVSDK_NGX_Handle** ppOutHandle,
    NVSDK_NGX_Parameter* pInParams,
    NVSDK_NGX_Feature_Create_Params* pDeepDVCCreateParams)
{
    return NVSDK_NGX_CUDA_CreateFeature(NVSDK_NGX_Feature_DeepDVC, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_D3D11_EVALUATE_DEEPDVC_EXT(
    ID3D11DeviceContext *pInCtx,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_D3D11_DEEPDVC_Eval_Params *pDeepDVCEvalParams)
{
    if (pDeepDVCEvalParams->Strength < 0.0f || pDeepDVCEvalParams->Strength > 1.0f || 
        pDeepDVCEvalParams->SaturationBoost < 0.0f || pDeepDVCEvalParams->SaturationBoost > 1.0f)
    {
        return NVSDK_NGX_Result_FAIL_InvalidParameter;
    }
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_Color, pDeepDVCEvalParams->pInColor);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X, pDeepDVCEvalParams->InColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y, pDeepDVCEvalParams->InColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, pDeepDVCEvalParams->InColorSubrectSize.Width);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, pDeepDVCEvalParams->InColorSubrectSize.Height);
    // Strength in range [0.0f,1.0f]
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DeepDVC_Strength, pDeepDVCEvalParams->Strength);
    // SaturationBoost in range [0.0f,1.0f]
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DeepDVC_SaturationBoost, pDeepDVCEvalParams->SaturationBoost);
    return NVSDK_NGX_D3D11_EvaluateFeature_C(pInCtx, pInHandle, pInParams, NULL);
}

static inline NVSDK_NGX_Result NGX_CUDA_EVALUATE_DEEPDVC_EXT(
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_CUDA_DEEPDVC_Eval_Params *pDeepDVCEvalParams)
{
    if (pDeepDVCEvalParams->Strength < 0.0f || pDeepDVCEvalParams->Strength > 1.0f ||
        pDeepDVCEvalParams->SaturationBoost < 0.0f || pDeepDVCEvalParams->SaturationBoost > 1.0f)
    {
        return NVSDK_NGX_Result_FAIL_InvalidParameter;
    }
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_Color, pDeepDVCEvalParams->pInColor);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X, pDeepDVCEvalParams->InColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y, pDeepDVCEvalParams->InColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, pDeepDVCEvalParams->InColorSubrectSize.Width);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, pDeepDVCEvalParams->InColorSubrectSize.Height);
    // Strength in range [0.0f,1.0f]
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DeepDVC_Strength, pDeepDVCEvalParams->Strength);
    // SaturationBoost in range [0.0f,1.0f]
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DeepDVC_SaturationBoost, pDeepDVCEvalParams->SaturationBoost);
    return NVSDK_NGX_CUDA_EvaluateFeature_C(pInHandle, pInParams, NULL);
}

static inline NVSDK_NGX_Result NGX_D3D12_CREATE_DEEPDVC_EXT(
    ID3D12GraphicsCommandList *InCmdList,
    unsigned int InCreationNodeMask,
    unsigned int InVisibilityNodeMask,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_Feature_Create_Params *pDeepDVCCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_CreationNodeMask, InCreationNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_VisibilityNodeMask, InVisibilityNodeMask);
    return NVSDK_NGX_D3D12_CreateFeature(InCmdList, NVSDK_NGX_Feature_DeepDVC, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_D3D12_EVALUATE_DEEPDVC_EXT(
    ID3D12GraphicsCommandList *pInCmdList,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_D3D12_DEEPDVC_Eval_Params *pDeepDVCEvalParams)
{
    if (pDeepDVCEvalParams->Strength < 0.0f || pDeepDVCEvalParams->Strength > 1.0f ||
        pDeepDVCEvalParams->SaturationBoost < 0.0f || pDeepDVCEvalParams->SaturationBoost > 1.0f)
    {
        return NVSDK_NGX_Result_FAIL_InvalidParameter;
    }
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_Color, pDeepDVCEvalParams->pInColor);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X, pDeepDVCEvalParams->InColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y, pDeepDVCEvalParams->InColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, pDeepDVCEvalParams->InColorSubrectSize.Width);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, pDeepDVCEvalParams->InColorSubrectSize.Height);;
    // Strength in range [0.0f,1.0f]
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DeepDVC_Strength, pDeepDVCEvalParams->Strength);
    // SaturationBoost in range [0.0f,1.0f]
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DeepDVC_SaturationBoost, pDeepDVCEvalParams->SaturationBoost);
    return NVSDK_NGX_D3D12_EvaluateFeature_C(pInCmdList, pInHandle, pInParams, NULL);
}

#endif // NVSDK_NGX_HELPERS_DEEPDVC_H
