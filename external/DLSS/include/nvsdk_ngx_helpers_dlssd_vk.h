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

#ifndef NVSDK_NGX_HELPERS_DLSSD_VK_H
#define NVSDK_NGX_HELPERS_DLSSD_VK_H
#pragma once

#include "nvsdk_ngx_vk.h"
#include "nvsdk_ngx_defs_dlssd.h"
#include "nvsdk_ngx_params_dlssd.h"

typedef struct NVSDK_NGX_VK_DLSSD_Eval_Params
{
    NVSDK_NGX_Resource_VK*              pInDiffuseAlbedo;
    NVSDK_NGX_Resource_VK*              pInSpecularAlbedo;
    NVSDK_NGX_Resource_VK*              pInNormals;
    NVSDK_NGX_Resource_VK*              pInRoughness;

    NVSDK_NGX_Resource_VK*              pInColor;
    NVSDK_NGX_Resource_VK*              pInOutput;
    NVSDK_NGX_Resource_VK *             pInDepth;
    NVSDK_NGX_Resource_VK *             pInMotionVectors;
    float                               InJitterOffsetX;     /* Jitter offset must be in input/render pixel space */
    float                               InJitterOffsetY;
    NVSDK_NGX_Dimensions                InRenderSubrectDimensions;
    /*** OPTIONAL - leave to 0/0.0f if unused ***/
    int                                 InReset;             /* Set to 1 when scene changes completely (new level etc) */
    float                               InMVScaleX;          /* If MVs need custom scaling to convert to pixel space */
    float                               InMVScaleY;
    NVSDK_NGX_Resource_VK *             pInTransparencyMask; /* Unused/Reserved for future use */
    NVSDK_NGX_Resource_VK *             pInExposureTexture;
    NVSDK_NGX_Resource_VK *             pInBiasCurrentColorMask;
    NVSDK_NGX_Coordinates               InDiffuseAlbedoSubrectBase;
    NVSDK_NGX_Coordinates               InSpecularAlbedoSubrectBase;
    NVSDK_NGX_Coordinates               InNormalsSubrectBase;
    NVSDK_NGX_Coordinates               InRoughnessSubrectBase;
    NVSDK_NGX_Coordinates               InColorSubrectBase;
    NVSDK_NGX_Coordinates               InDepthSubrectBase;
    NVSDK_NGX_Coordinates               InMVSubrectBase;
    NVSDK_NGX_Coordinates               InTranslucencySubrectBase;
    NVSDK_NGX_Coordinates               InBiasCurrentColorSubrectBase;
    NVSDK_NGX_Coordinates               InOutputSubrectBase;
    float                               InPreExposure;
    float                               InExposureScale;
    int                                 InIndicatorInvertXAxis;
    int                                 InIndicatorInvertYAxis;
    /*** OPTIONAL - only for research purposes ***/

    NVSDK_NGX_Resource_VK*              pInReflectedAlbedo;
    NVSDK_NGX_Resource_VK*              pInColorBeforeParticles;
    NVSDK_NGX_Resource_VK*              pInColorBeforeTransparency;
    NVSDK_NGX_Resource_VK*              pInColorBeforeFog;
    NVSDK_NGX_Resource_VK*              pInDiffuseHitDistance;
    NVSDK_NGX_Resource_VK*              pInSpecularHitDistance;
    NVSDK_NGX_Resource_VK*              pInDiffuseRayDirection;
    NVSDK_NGX_Resource_VK*              pInSpecularRayDirection;
    NVSDK_NGX_Resource_VK*              pInDiffuseRayDirectionHitDistance;
    NVSDK_NGX_Resource_VK*              pInSpecularRayDirectionHitDistance;
    NVSDK_NGX_Coordinates               InReflectedAlbedoSubrectBase;
    NVSDK_NGX_Coordinates               InColorBeforeParticlesSubrectBase;
    NVSDK_NGX_Coordinates               InColorBeforeTransparencySubrectBase;
    NVSDK_NGX_Coordinates               InColorBeforeFogSubrectBase;
    NVSDK_NGX_Coordinates               InDiffuseHitDistanceSubrectBase;
    NVSDK_NGX_Coordinates               InSpecularHitDistanceSubrectBase;
    NVSDK_NGX_Coordinates               InDiffuseRayDirectionSubrectBase;
    NVSDK_NGX_Coordinates               InSpecularRayDirectionSubrectBase;
    NVSDK_NGX_Coordinates               InDiffuseRayDirectionHitDistanceSubrectBase;
    NVSDK_NGX_Coordinates               InSpecularRayDirectionHitDistanceSubrectBase;
    float*                              pInWorldToViewMatrix;
    float*                              pInViewToClipMatrix;

    NVSDK_NGX_VK_GBuffer                GBufferSurface;
    NVSDK_NGX_ToneMapperType            InToneMapperType;
    NVSDK_NGX_Resource_VK *             pInMotionVectors3D;
    NVSDK_NGX_Resource_VK *             pInIsParticleMask; /* to identify which pixels contains particles, essentially that are not drawn as part of base pass */
    NVSDK_NGX_Resource_VK *             pInAnimatedTextureMask; /* a binary mask covering pixels occupied by animated textures */
    NVSDK_NGX_Resource_VK *             pInDepthHighRes;
    NVSDK_NGX_Resource_VK *             pInPositionViewSpace;
    float                               InFrameTimeDeltaInMsec; /* helps in determining the amount to denoise or anti-alias based on the speed of the object from motion vector magnitudes and fps as determined by this delta */
    NVSDK_NGX_Resource_VK *             pInRayTracingHitDistance; /* for each effect - approximation to the amount of noise in a ray-traced color */
    NVSDK_NGX_Resource_VK *             pInMotionVectorsReflections; /* motion vectors of reflected objects like for mirrored surfaces */
} NVSDK_NGX_VK_DLSSD_Eval_Params;

static inline NVSDK_NGX_Result NGX_VULKAN_CREATE_DLSSD_EXT1(
    VkDevice InDevice,
    VkCommandBuffer InCmdList,
    unsigned int InCreationNodeMask,
    unsigned int InVisibilityNodeMask,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_DLSSD_Create_Params *pInDlssDCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_CreationNodeMask, InCreationNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_VisibilityNodeMask, InVisibilityNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, pInDlssDCreateParams->InWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, pInDlssDCreateParams->InHeight);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutWidth, pInDlssDCreateParams->InTargetWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutHeight, pInDlssDCreateParams->InTargetHeight);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_PerfQualityValue, pInDlssDCreateParams->InPerfQualityValue);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, pInDlssDCreateParams->InFeatureCreateFlags);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Enable_Output_Subrects, pInDlssDCreateParams->InEnableOutputSubrects ? 1 : 0);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Denoise_Mode, NVSDK_NGX_DLSS_Denoise_Mode_DLUnified);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Roughness_Mode, pInDlssDCreateParams->InRoughnessMode);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Use_HW_Depth, pInDlssDCreateParams->InUseHWDepth);

    if (InDevice) return NVSDK_NGX_VULKAN_CreateFeature1(InDevice, InCmdList, NVSDK_NGX_Feature_RayReconstruction, pInParams, ppOutHandle);
    else return NVSDK_NGX_VULKAN_CreateFeature(InCmdList, NVSDK_NGX_Feature_RayReconstruction, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_VULKAN_EVALUATE_DLSSD_EXT(
    VkCommandBuffer InCmdList,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_VK_DLSSD_Eval_Params *pInDlssDEvalParams)
{
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInColor);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInMotionVectors);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInOutput);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInDepth);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInDiffuseAlbedo);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInSpecularAlbedo);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInTransparencyMask);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInExposureTexture);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInBiasCurrentColorMask);
	for (size_t i = 0; i <= 15; i++)
	{
        NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->GBufferSurface.pInAttrib[i]);
	}
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInMotionVectors3D);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInIsParticleMask);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInAnimatedTextureMask);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInDepthHighRes);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInPositionViewSpace);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInRayTracingHitDistance);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInMotionVectorsReflections);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInReflectedAlbedo);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInColorBeforeParticles);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInColorBeforeTransparency);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInColorBeforeFog);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInDiffuseHitDistance);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInSpecularHitDistance);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInDiffuseRayDirection);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInSpecularRayDirection);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInDiffuseRayDirectionHitDistance);
    NVSDK_NGX_ENSURE_VK_IMAGEVIEW(pInDlssDEvalParams->pInSpecularRayDirectionHitDistance);

    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_Color, pInDlssDEvalParams->pInColor);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_Output, pInDlssDEvalParams->pInOutput);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_Depth, pInDlssDEvalParams->pInDepth);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_MotionVectors, pInDlssDEvalParams->pInMotionVectors);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Jitter_Offset_X, pInDlssDEvalParams->InJitterOffsetX);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Jitter_Offset_Y, pInDlssDEvalParams->InJitterOffsetY);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_Reset, pInDlssDEvalParams->InReset);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_MV_Scale_X, pInDlssDEvalParams->InMVScaleX == 0.0f ? 1.0f : pInDlssDEvalParams->InMVScaleX);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_MV_Scale_Y, pInDlssDEvalParams->InMVScaleY == 0.0f ? 1.0f : pInDlssDEvalParams->InMVScaleY);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_TransparencyMask, pInDlssDEvalParams->pInTransparencyMask);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_ExposureTexture, pInDlssDEvalParams->pInExposureTexture);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, pInDlssDEvalParams->pInBiasCurrentColorMask);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Albedo, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_ALBEDO]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Roughness, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_ROUGHNESS]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Metallic, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_METALLIC]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Specular, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SPECULAR]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Subsurface, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SUBSURFACE]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Normals, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_NORMALS]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_ShadingModelId, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SHADINGMODELID]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_MaterialId, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_MATERIALID]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_8, pInDlssDEvalParams->GBufferSurface.pInAttrib[8]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_9, pInDlssDEvalParams->GBufferSurface.pInAttrib[9]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_SpecularMvec, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SPECULAR_MVEC]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_11, pInDlssDEvalParams->GBufferSurface.pInAttrib[11]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_12, pInDlssDEvalParams->GBufferSurface.pInAttrib[12]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_13, pInDlssDEvalParams->GBufferSurface.pInAttrib[13]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_14, pInDlssDEvalParams->GBufferSurface.pInAttrib[14]);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_15, pInDlssDEvalParams->GBufferSurface.pInAttrib[15]);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_TonemapperType, pInDlssDEvalParams->InToneMapperType);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_MotionVectors3D, pInDlssDEvalParams->pInMotionVectors3D);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_IsParticleMask, pInDlssDEvalParams->pInIsParticleMask);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_AnimatedTextureMask, pInDlssDEvalParams->pInAnimatedTextureMask);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DepthHighRes, pInDlssDEvalParams->pInDepthHighRes);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_Position_ViewSpace, pInDlssDEvalParams->pInPositionViewSpace);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, pInDlssDEvalParams->InFrameTimeDeltaInMsec);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_RayTracingHitDistance, pInDlssDEvalParams->pInRayTracingHitDistance);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_MotionVectorsReflection, pInDlssDEvalParams->pInMotionVectorsReflections);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X, pInDlssDEvalParams->InColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y, pInDlssDEvalParams->InColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_X, pInDlssDEvalParams->InDepthSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_Y, pInDlssDEvalParams->InDepthSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_X, pInDlssDEvalParams->InMVSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_Y, pInDlssDEvalParams->InMVSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_X, pInDlssDEvalParams->InTranslucencySubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_Y, pInDlssDEvalParams->InTranslucencySubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_X, pInDlssDEvalParams->InBiasCurrentColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_Y, pInDlssDEvalParams->InBiasCurrentColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_X, pInDlssDEvalParams->InOutputSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_Y, pInDlssDEvalParams->InOutputSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width , pInDlssDEvalParams->InRenderSubrectDimensions.Width);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, pInDlssDEvalParams->InRenderSubrectDimensions.Height);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DLSS_Pre_Exposure, pInDlssDEvalParams->InPreExposure == 0.0f ? 1.0f : pInDlssDEvalParams->InPreExposure);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DLSS_Exposure_Scale, pInDlssDEvalParams->InExposureScale == 0.0f ? 1.0f : pInDlssDEvalParams->InExposureScale);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Indicator_Invert_X_Axis, pInDlssDEvalParams->InIndicatorInvertXAxis);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Indicator_Invert_Y_Axis, pInDlssDEvalParams->InIndicatorInvertYAxis);

    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Emissive, pInDlssDEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_EMISSIVE]);

    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DiffuseAlbedo, pInDlssDEvalParams->pInDiffuseAlbedo);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_SpecularAlbedo, pInDlssDEvalParams->pInSpecularAlbedo);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Normals, pInDlssDEvalParams->pInNormals);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_GBuffer_Roughness, pInDlssDEvalParams->pInRoughness);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_DiffuseAlbedo_Subrect_Base_X, pInDlssDEvalParams->InDiffuseAlbedoSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_DiffuseAlbedo_Subrect_Base_Y, pInDlssDEvalParams->InDiffuseAlbedoSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_SpecularAlbedo_Subrect_Base_X, pInDlssDEvalParams->InSpecularAlbedoSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_SpecularAlbedo_Subrect_Base_Y, pInDlssDEvalParams->InSpecularAlbedoSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Normals_Subrect_Base_X, pInDlssDEvalParams->InNormalsSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Normals_Subrect_Base_Y, pInDlssDEvalParams->InNormalsSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Roughness_Subrect_Base_X, pInDlssDEvalParams->InRoughnessSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Roughness_Subrect_Base_Y, pInDlssDEvalParams->InRoughnessSubrectBase.Y);


    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_ReflectedAlbedo, pInDlssDEvalParams->pInReflectedAlbedo);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_ColorBeforeParticles, pInDlssDEvalParams->pInColorBeforeParticles);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_ColorBeforeTransparency, pInDlssDEvalParams->pInColorBeforeTransparency);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_ColorBeforeFog, pInDlssDEvalParams->pInColorBeforeFog);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_DiffuseHitDistance, pInDlssDEvalParams->pInDiffuseHitDistance);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_SpecularHitDistance, pInDlssDEvalParams->pInSpecularHitDistance);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirection, pInDlssDEvalParams->pInDiffuseRayDirection);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_SpecularRayDirection, pInDlssDEvalParams->pInSpecularRayDirection);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirectionHitDistance, pInDlssDEvalParams->pInDiffuseRayDirectionHitDistance);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSD_SpecularRayDirectionHitDistance, pInDlssDEvalParams->pInSpecularRayDirectionHitDistance);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_ReflectedAlbedo_Subrect_Base_X, pInDlssDEvalParams->InReflectedAlbedoSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_ReflectedAlbedo_Subrect_Base_Y, pInDlssDEvalParams->InReflectedAlbedoSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_ColorBeforeParticles_Subrect_Base_X, pInDlssDEvalParams->InColorBeforeParticlesSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_ColorBeforeParticles_Subrect_Base_Y, pInDlssDEvalParams->InColorBeforeParticlesSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_ColorBeforeTransparency_Subrect_Base_X, pInDlssDEvalParams->InColorBeforeTransparencySubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_ColorBeforeTransparency_Subrect_Base_Y, pInDlssDEvalParams->InColorBeforeTransparencySubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_ColorBeforeFog_Subrect_Base_X, pInDlssDEvalParams->InColorBeforeFogSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_ColorBeforeFog_Subrect_Base_Y, pInDlssDEvalParams->InColorBeforeFogSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_DiffuseHitDistance_Subrect_Base_X, pInDlssDEvalParams->InDiffuseHitDistanceSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_DiffuseHitDistance_Subrect_Base_Y, pInDlssDEvalParams->InDiffuseHitDistanceSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_SpecularHitDistance_Subrect_Base_X, pInDlssDEvalParams->InSpecularHitDistanceSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_SpecularHitDistance_Subrect_Base_Y, pInDlssDEvalParams->InSpecularHitDistanceSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirection_Subrect_Base_X, pInDlssDEvalParams->InDiffuseRayDirectionSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirection_Subrect_Base_Y, pInDlssDEvalParams->InDiffuseRayDirectionSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_SpecularRayDirection_Subrect_Base_X, pInDlssDEvalParams->InSpecularRayDirectionSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_SpecularRayDirection_Subrect_Base_Y, pInDlssDEvalParams->InSpecularRayDirectionSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirectionHitDistance_Subrect_Base_X, pInDlssDEvalParams->InDiffuseRayDirectionHitDistanceSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirectionHitDistance_Subrect_Base_Y, pInDlssDEvalParams->InDiffuseRayDirectionHitDistanceSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_SpecularRayDirectionHitDistance_Subrect_Base_X, pInDlssDEvalParams->InSpecularRayDirectionHitDistanceSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSSD_SpecularRayDirectionHitDistance_Subrect_Base_Y, pInDlssDEvalParams->InSpecularRayDirectionHitDistanceSubrectBase.Y);

    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSS_WORLD_TO_VIEW_MATRIX, pInDlssDEvalParams->pInWorldToViewMatrix);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSS_VIEW_TO_CLIP_MATRIX, pInDlssDEvalParams->pInViewToClipMatrix);

    return NVSDK_NGX_VULKAN_EvaluateFeature_C(InCmdList, pInHandle, pInParams, NULL);
}

#endif // NVSDK_NGX_HELPERS_DLSSD_VK_H