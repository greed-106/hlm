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
#ifndef _HLMC_TQ_H_
#define _HLMC_TQ_H_

#include "hlmc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HLMC_Q_BITS                 15
#define HLMC_DEFAULT_I_BIAS         682
#define HLMC_DEFAULT_P_BIAS         342
#define HLMC_BIAS_SHIFT_BITS        11

static const HLM_S32 HLMC_QUANT_MF[6][3] =
{
    { 13107, 8066, 5243 },
    { 11916, 7490, 4660 },
    { 10082, 6554, 4194 },
    { 9362, 5825, 3647 },
    { 8192, 5243, 3355 },
    { 7282, 4559, 2893 },
};

/***************************************************************************************************
* 功  能：根据qp，设置量化信息和lambda
* 参  数：*
*        regs                 -I    寄存器
*        cur_cu               -IO   当前CU信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_TQ_SetQuantInfo(HLMC_CU_INFO *cur_cu,
                              HLMC_REGS    *regs);

/***************************************************************************************************
* 功  能：变换量化
* 参  数：*
*        raster_idx           -I    当前TU的索引
*        channel_size         -I    当前通道的大小
*        bitdepth             -I    比特深度
*        yuv_comp             -I    yuv分量个数
*        cur_cu               -IO   当前CU信息
* 备  注：调用前对cu_type先赋值，i块赋值i_16x8，p块赋值p_16x8
***************************************************************************************************/
HLM_VOID HLMC_TQ_Process(HLM_U32         raster_idx,
                         HLM_U32         channel_size,
                         HLM_U08         bitdepth,
                         HLM_U32         yuv_comp,
                         HLMC_CU_INFO   *cur_cu);
#if LINE_BY_LINE
/***************************************************************************************************
* 功  能：16x1的量化
* 参  数：*
*        regs                 -I    寄存器
*        cur_cu               -IO   当前CU信息
*        line_index           -I    当前TU的索引
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_INTRA_16x1_TQ_Process(HLMC_REGS              *regs, 
                                   HLMC_CU_INFO           *cur_mb, 
                                   HLM_U08 line_index);
/***************************************************************************************************
* 功  能：1x8的量化
* 参  数：*
*        regs                 -I    寄存器
*        cur_cu               -IO   当前CU信息
*        line_index           -I    当前TU的索引
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_INTRA_1x8_TQ_Process(HLMC_REGS              *regs,
                                  HLMC_CU_INFO           *cur_mb,
                                  HLM_U08 line_index);
#endif
#ifdef __cplusplus
}
#endif

#endif // _HLMC_TQ_H_
