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

#ifndef NVSDK_NGX_DEFS_DLSSD_H
#define NVSDK_NGX_DEFS_DLSSD_H
#pragma once

typedef enum NVSDK_NGX_DLSS_Denoise_Mode
{
    NVSDK_NGX_DLSS_Denoise_Mode_Off = 0,
    NVSDK_NGX_DLSS_Denoise_Mode_DLUnified = 1, // DL based unified upscaler
} NVSDK_NGX_DLSS_Denoise_Mode;

typedef enum NVSDK_NGX_DLSS_Roughness_Mode
{
    NVSDK_NGX_DLSS_Roughness_Mode_Unpacked = 0, // Read roughness separately 
    NVSDK_NGX_DLSS_Roughness_Mode_Packed = 1, // Read roughness from normals.w
} NVSDK_NGX_DLSS_Roughness_Mode;

typedef enum NVSDK_NGX_DLSS_Depth_Type
{
    NVSDK_NGX_DLSS_Depth_Type_Linear = 0, // Linear Depth
    NVSDK_NGX_DLSS_Depth_Type_HW = 1,     // HW Depth
} NVSDK_NGX_DLSS_Depth_Type;

#define NVSDK_NGX_Parameter_DLSS_Denoise_Mode "DLSS.Denoise.Mode"
#define NVSDK_NGX_Parameter_DLSS_Roughness_Mode "DLSS.Roughness.Mode"
#define NVSDK_NGX_Parameter_DiffuseAlbedo  "DLSS.Input.DiffuseAlbedo"
#define NVSDK_NGX_Parameter_SpecularAlbedo "DLSS.Input.SpecularAlbedo"
#define NVSDK_NGX_Parameter_DLSS_Input_DiffuseAlbedo_Subrect_Base_X "DLSS.Input.DiffuseAlbedo.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSS_Input_DiffuseAlbedo_Subrect_Base_Y "DLSS.Input.DiffuseAlbedo.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSS_Input_SpecularAlbedo_Subrect_Base_X "DLSS.Input.SpecularAlbedo.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSS_Input_SpecularAlbedo_Subrect_Base_Y "DLSS.Input.SpecularAlbedo.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSS_Input_Normals_Subrect_Base_X "DLSS.Input.Normals.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSS_Input_Normals_Subrect_Base_Y "DLSS.Input.Normals.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSS_Input_Roughness_Subrect_Base_X "DLSS.Input.Roughness.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSS_Input_Roughness_Subrect_Base_Y "DLSS.Input.Roughness.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_ViewToClipMatrix "ViewToClipMatrix"
#define NVSDK_NGX_Parameter_GBuffer_Emissive "GBuffer.Emissive"
#define NVSDK_NGX_Parameter_Use_Folded_Network "DLSS.Use.Folded.Network"
#define NVSDK_NGX_Parameter_Diffuse_Ray_Direction "Diffuse.Ray.Direction"
#define NVSDK_NGX_Parameter_DLSS_WORLD_TO_VIEW_MATRIX "WorldToViewMatrix"
#define NVSDK_NGX_Parameter_DLSS_VIEW_TO_CLIP_MATRIX "ViewToClipMatrix"
#define NVSDK_NGX_Parameter_Use_HW_Depth "DLSS.Use.HW.Depth"
#define NVSDK_NGX_Parameter_DLSSD_ReflectedAlbedo "DLSSD.ReflectedAlbedo"
#define NVSDK_NGX_Parameter_DLSSD_ColorBeforeParticles "DLSSD.ColorBeforeParticles"
#define NVSDK_NGX_Parameter_DLSSD_ColorBeforeTransparency "DLSSD.ColorBeforeTransparency"
#define NVSDK_NGX_Parameter_DLSSD_ColorBeforeFog "DLSSD.ColorBeforeFog"
#define NVSDK_NGX_Parameter_DLSSD_DiffuseHitDistance "DLSSD.DiffuseHitDistance"
#define NVSDK_NGX_Parameter_DLSSD_SpecularHitDistance "DLSSD.SpecularHitDistance"
#define NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirection "DLSSD.DiffuseRayDirection"
#define NVSDK_NGX_Parameter_DLSSD_SpecularRayDirection "DLSSD.SpecularRayDirection"
#define NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirectionHitDistance "DLSSD.DiffuseRayDirectionHitDistance"
#define NVSDK_NGX_Parameter_DLSSD_SpecularRayDirectionHitDistance "DLSSD.SpecularRayDirectionHitDistance"
#define NVSDK_NGX_Parameter_DLSSD_ReflectedAlbedo_Subrect_Base_X "DLSSD.ReflectedAlbedo.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_ReflectedAlbedo_Subrect_Base_Y "DLSSD.ReflectedAlbedo.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSSD_ColorBeforeParticles_Subrect_Base_X "DLSSD.ColorBeforeParticles.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_ColorBeforeParticles_Subrect_Base_Y "DLSSD.ColorBeforeParticles.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSSD_ColorBeforeTransparency_Subrect_Base_X "DLSSD.ColorBeforeTransparency.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_ColorBeforeTransparency_Subrect_Base_Y "DLSSD.ColorBeforeTransparency.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSSD_ColorBeforeFog_Subrect_Base_X "DLSSD.ColorBeforeFog.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_ColorBeforeFog_Subrect_Base_Y "DLSSD.ColorBeforeFog.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSSD_DiffuseHitDistance_Subrect_Base_X "DLSSD.DiffuseHitDistance.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_DiffuseHitDistance_Subrect_Base_Y "DLSSD.DiffuseHitDistance.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSSD_SpecularHitDistance_Subrect_Base_X "DLSSD.SpecularHitDistance.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_SpecularHitDistance_Subrect_Base_Y "DLSSD.SpecularHitDistance.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirection_Subrect_Base_X "DLSSD.DiffuseRayDirection.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirection_Subrect_Base_Y "DLSSD.DiffuseRayDirection.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSSD_SpecularRayDirection_Subrect_Base_X "DLSSD.SpecularRayDirection.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_SpecularRayDirection_Subrect_Base_Y "DLSSD.SpecularRayDirection.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirectionHitDistance_Subrect_Base_X "DLSSD.DiffuseRayDirectionHitDistance.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_DiffuseRayDirectionHitDistance_Subrect_Base_Y "DLSSD.DiffuseRayDirectionHitDistance.Subrect.Base.Y"
#define NVSDK_NGX_Parameter_DLSSD_SpecularRayDirectionHitDistance_Subrect_Base_X "DLSSD.SpecularRayDirectionHitDistance.Subrect.Base.X"
#define NVSDK_NGX_Parameter_DLSSD_SpecularRayDirectionHitDistance_Subrect_Base_Y "DLSSD.SpecularRayDirectionHitDistance.Subrect.Base.Y"

#define NVSDK_NGX_Parameter_SuperSamplingDenoising_Available             "SuperSamplingDenoising.Available"
#define NVSDK_NGX_Parameter_SuperSamplingDenoising_NeedsUpdatedDriver    "SuperSamplingDenoising.NeedsUpdatedDriver"
#define NVSDK_NGX_Parameter_SuperSamplingDenoising_MinDriverVersionMajor "SuperSamplingDenoising.MinDriverVersionMajor"
#define NVSDK_NGX_Parameter_SuperSamplingDenoising_MinDriverVersionMinor "SuperSamplingDenoising.MinDriverVersionMinor"
#define NVSDK_NGX_Parameter_SuperSamplingDenoising_FeatureInitResult     "SuperSamplingDenoising.FeatureInitResult"
#define NVSDK_NGX_Parameter_DLSSDOptimalSettingsCallback "DLSSDOptimalSettingsCallback"
#define NVSDK_NGX_Parameter_DLSSDGetStatsCallback        "DLSSDGetStatsCallback"


#endif // NVSDK_NGX_DEFS_DLSSD_H