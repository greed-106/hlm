/***************************************************************************************************

The copyright in this software is being made available under the License included below.
This software may be subject to other third party and contributor rights, including patent
rights, and no such rights are granted under this license.

Copyright (C) 2025, Hangzhou Hikvision Digital Technology Co., Ltd. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted
only for the purpose of developing standards within Audio and Video Coding Standard Workgroup of
China (AVS) and for testing and promoting such standards. The following conditions are required
to be met:

* Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.
* The name of Hangzhou Hikvision Digital Technology Co., Ltd. may not be used to endorse or
promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

***************************************************************************************************/
#ifndef _HLMC_INTRA_H_
#define _HLMC_INTRA_H_

#include "hlmc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _HLMC_INTRA_MODE_COST
{
    HLM_U32 total_cost;
    HLM_U32 bits;
    HLM_U32 satd;
    HLM_U32 satd_comp[3];  // 单独每个分量的satd
} HLMC_INTRA_MODE_COST;

static const HLM_U32 HLMC_INTRA_MODE_BIT_COST_16x8[HLM_INTRA_PRED_MODE_NUM_16x8] = { 3,3,5,5 };

/***************************************************************************************************
* 功  能：帧内模式选择主函数
* 参  数：*
*        regs                  -I    硬件寄存器
*        nbi_info              -I    当前cu相邻块信息
*        cur_cu                -IO   当前cu信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTRA_MODE(HLMC_REGS            *regs,
                         HLM_NEIGHBOR_INFO    *nbi_info,
                         HLMC_CU_INFO         *cur_cu);

/***************************************************************************************************
* 功  能：逐行逐列模式初筛主函数
* 参  数：*
*        regs               -I        硬件寄存器
*        nbi_info           -I        当前ctu相邻块信息
*        cur_cu             -IO       当前ctu信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_LINE_BY_LINE_MODE(HLMC_REGS            *regs,
                                HLM_NEIGHBOR_INFO    *nbi_info,
                                HLMC_CU_INFO         *cur_cu);

/***************************************************************************************************
* 功  能：获取16x1的参考像素
* 参  数：*
*        regs                    -I       硬件寄存器
*        *cur_mb                 -I       当前ctu信息
*        *ref_pixel               -O       参考像素
*        line_index               -I       预测行
*        org_pixel_flag           -I       原始值做参考标志
*        pred_mode                -I       预测行
* 返回值：
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTRA_MODE_get_ref_pixel_16x1(HLMC_REGS      *regs,
                                            HLMC_CU_INFO   *cur_cu,
                                            HLM_U16        *ref_pixel,
                                            HLM_U08         pu_line_index,
                                            HLM_U08         org_pixel_flag,
#if  HLM_INTRA_SEARCH_REC_L
                                           HLM_NEIGHBOR_INFO *nbi_info,
#endif
                                            HLM_U08        *pred_mode);

/***************************************************************************************************
* 功  能：获取1x8的参考像素
* 参  数：*
*        regs                    -I       硬件寄存器
*        *cur_mb                 -I       当前ctu信息
*        *ref_pixel               -O       参考像素
*        line_index               -I       预测行
*        org_pixel_flag           -I       原始值做参考标志
*        pred_mode                -I       预测模式
* 返回值：
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTRA_MODE_get_ref_pixel_2x8(HLMC_REGS      *regs,
                                           HLMC_CU_INFO   *cur_mb,
                                           HLM_U16        *ref_pixel,
                                           HLM_U08         line_index,
                                           HLM_U08         org_pixel_flag,
#if  HLM_INTRA_SEARCH_REC_R
                                           HLM_NEIGHBOR_INFO *nbi_info,
#endif
                                           HLM_U08        *pred_mode);

/***************************************************************************************************
* 功  能：逐行预测 RDO模块
* 参  数：*
*        regs                 -I    当前数据硬件寄存器
*        nbi_info             -I    相邻块信息
*        best_mb              -IO   最优ctu
*        cur_mb               -IO   当前ctu
* 返回值：无
***************************************************************************************************/
HLM_VOID HLMC_LINE_BY_LINE_RDO(HLMC_REGS              *regs,
                               HLM_NEIGHBOR_INFO      *nbi_info,
                               HLMC_CU_INFO           *best_mb,
                               HLMC_CU_INFO           *cur_mb);

/***************************************************************************************************
* 功  能：逐行预测 RD计算模块
* 参  数：*
*        regs                 -I    当前数据硬件寄存器
*        distortion            -I   当前模式损失
*        best_mb              -IO   最优ctu
*        cur_mb               -IO   当前ctu
* 返回值：无
***************************************************************************************************/
HLM_VOID LINE_BY_LINE_RD_CAL(HLMC_REGS              *regs,
                             HLM_U32                 distortion,
                             HLMC_CU_INFO           *best_cu,
                             HLMC_CU_INFO           *cur_cu);

/***************************************************************************************************
* 功  能：SCC模式初筛主函数
* 参  数：*
*        regs               -I        硬件寄存器
*        nbi_info           -I        当前cu相邻块信息
*        cur_cu             -IO       当前cu信息
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_SCC_MODE(HLMC_REGS            *regs,
                      HLM_NEIGHBOR_INFO    *nbi_info,
                      HLMC_CU_INFO         *cur_cu);

/***************************************************************************************************
* 功  能：帧内预测主函数
* 参  数：*
*        raster_idx               -I    当前PU的光栅扫描索引
*        channel_size             -I    通道大小
*        regs                     -I    硬件寄存器
*        nbi_info                 -I    当前cu相邻块信息
*        cur_cu                   -O    当前cu信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTRA_PRED(HLM_U32                raster_idx,
                         HLM_U32                channel_size,
                         HLMC_REGS             *regs,
                         HLM_NEIGHBOR_INFO     *nbi_info,
                         HLMC_CU_INFO          *cur_cu);

/***************************************************************************************************
* 功  能：帧内RDO模块
* 参  数：*
*        raster_idx               -I    当前PU的光栅扫描索引
*        channel_size             -I    当前通道的大小
*        regs                     -I    当前数据硬件寄存器
*        nbi_info                 -I    相邻块信息
*        best_cu                  -O    最优cu
*        cur_cu                   -I    当前cu
* 返回值：无
***************************************************************************************************/
HLM_VOID HLMC_INTRA_RDO(HLM_U32                raster_idx,
                        HLM_U32                channel_size,
                        HLMC_REGS             *regs,
                        HLM_NEIGHBOR_INFO     *nbi_info,
                        HLMC_CU_INFO          *best_cu,
                        HLMC_CU_INFO          *cur_cu);

/***************************************************************************************************
* 功  能：SCC RDO模块
* 参  数：*
*        regs                 -I    当前数据硬件寄存器
*        nbi_info             -I    相邻块信息
*        best_cu              -O    最优cu
*        cur_cu               -I    当前cu
* 返回值：无
***************************************************************************************************/
HLM_VOID HLMC_SCC_RDO(HLMC_REGS              *regs,
                      HLM_NEIGHBOR_INFO      *nbi_info,
                      HLMC_CU_INFO           *best_cu,
                      HLMC_CU_INFO           *cur_cu);

#ifdef __cplusplus
}
#endif

#endif // _HLMC_INTRA_H_
