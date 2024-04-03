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

#ifndef NVSDK_NGX_DEFS_DEEPDVC_H
#define NVSDK_NGX_DEFS_DEEPDVC_H
#pragma once

#include "nvsdk_ngx_defs.h"

typedef enum NVSDK_NGX_DeepDVC_Mode
{
    NVSDK_NGX_DeepDVC_Mode_Off = 0, // No DeepDVC
    NVSDK_NGX_DeepDVC_Mode_On,  // Minimal DeepDVC Integration for RT Effects
} NVSDK_NGX_DeepDVC_Mode;

#define NVSDK_NGX_Parameter_DeepDVC_Available               "DeepDVC.Available"
#define NVSDK_NGX_Parameter_DeepDVC_NeedsUpdatedDriver      "DeepDVC.NeedsUpdatedDriver"
#define NVSDK_NGX_Parameter_DeepDVC_MinDriverVersionMajor   "DeepDVC.MinDriverVersionMajor"
#define NVSDK_NGX_Parameter_DeepDVC_MinDriverVersionMinor   "DeepDVC.MinDriverVersionMinor"
#define NVSDK_NGX_Parameter_DeepDVC_FeatureInitResult       "DeepDVC.FeatureInitResult"
#define NVSDK_NGX_Parameter_DeepDVC_Strength                "DeepDVC.Strength"
#define NVSDK_NGX_Parameter_DeepDVC_SaturationBoost         "DeepDVC.SaturationBoost"
#define NVSDK_NGX_Parameter_DeepDVC_GetStatsCallback        "DeepDVC.GetStatsCallback"

#endif // NVSDK_NGX_DEFS_DEEPDVC_H
