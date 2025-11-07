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
#ifndef _HLMC_COMMON_H_
#define _HLMC_COMMON_H_

#include "hlmc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* 功  能：将cur_cu_info的信息复制给best_cu_info
* 参  数：*
*         cur_cu           -I    cur_cu的起始地址
*         best_cu          -O    best_cu的起始地址
*         yuv_comp         -I    分量个数
* 返回值：
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_COM_CopyCuInfo(HLMC_CU_INFO       *cur_cu,
                             HLMC_CU_INFO       *best_cu,
                             HLM_U32             yuv_comp);

/***************************************************************************************************
* 功  能：计算两块block的SSE
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         block_width     -I    块宽度
*         block_height    -I    块高度
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
* 返回值：两块block的SSE值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_COM_ComputeSse(HLM_U16 *block0,
                            HLM_U16 *block1,
                            HLM_U32  block_width,
                            HLM_U32  block_height,
                            HLM_U32  block0_stride,
                            HLM_U32  block1_stride);

/***************************************************************************************************
* 功  能：计算两块block的残差
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         res             -O    残差的起始地址
*         block_width     -I    块宽度
*         block_height    -I    块高度
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
*         res_stride      -I    残差的跨度
* 返回值：
* 备  注：res = block0 - block1
***************************************************************************************************/
HLM_VOID HLMC_COM_ComputeRes(HLM_U16 *block0,
                             HLM_U16 *block1,
                             HLM_COEFF*res,
                             HLM_U32  block_width,
                             HLM_U32  block_height,
                             HLM_U32  block0_stride,
                             HLM_U32  block1_stride,
                             HLM_U32  res_stride);

/***************************************************************************************************
* 功  能：计算两块block的SAD
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         block_width     -I    两块block的宽
*         block_height    -I    两块block的高
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
* 返回值：两块block的SAD值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_COM_ComputeSad(HLM_U16 *block0,
                            HLM_U16 *block1,
                            HLM_U32  block_width,
                            HLM_U32  block_height,
                            HLM_U32  block0_stride,
                            HLM_U32  block1_stride);

#if MIX_IBC
/***************************************************************************************************
* 功  能：计算两块ibc4x4块的SAD
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
*         sad             -O    sad
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_COM_ComputeIbc4x4Sad(HLM_U16 *block0,
                                   HLM_U16 *block1,
                                   HLM_U32  block0_stride,
                                   HLM_U32  block1_stride,
                                   HLM_U32  sad[HLM_IBC_PART_NUM][4]);

/***************************************************************************************************
* 功  能：导出mix_ibc_flag和bvy_zero_flag
* 参  数：*
*        com_cu_info            -I         当前CU信息
*        merge_flag             -I         merge方式
*        mix_ibc_flag           -O         是否为混合ibc
*        bvy_zero_flag          -O         bvy是否全零
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_COM_DeriveMixIbcInfo(HLM_CU_INFO  *com_cu_info,
                                   HLM_U08       merge_flag,
                                   HLM_U08      *mix_ibc_flag,
                                   HLM_U08      *bvy_zero_flag);
#endif

#if PK_WITH_SATD
/***************************************************************************************************
* 功  能：计算两块4x4block的satd
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
* 返回值：两块block的satd值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_COM_ComputeSatd4x4(HLM_U16 *block0,
                                HLM_U16 *block1,
                                HLM_U32  block0_stride,
                                HLM_U32  block1_stride);
#endif

/***************************************************************************************************
* 功  能：计算两个块的satd或sad，使用4x4为基本单元
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         satd_record_4x4 -I    记录4x4子块的satd值
*         block_w         -I    两块block的宽度
*         block_h         -I    两块block的高度
*         stride_0        -I    第一块block的跨度
*         stride_1        -I    第二块block的跨度
* 返回值：两块block的satd值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_COM_ComputeDist16x8(HLM_U16  *block_0,
                                 HLM_U16  *block_1,
                                 HLM_U32  *satd_record_4x4,
                                 HLM_S32   block_w,
                                 HLM_S32   block_h,
                                 HLM_U32   stride_0,
                                 HLM_U32   stride_1);

/***************************************************************************************************
* 功  能：计算cbf
* 参  数：*
*         cu_type          -I    当前宏块类型
*         cbf              -O    三分量cbf
*         coeffs_num       -I    当前宏块4x4非零系数个数
* 返回值：
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_COM_GetCbf(HLM_CU_TYPE      cu_type,
                         HLM_U08          cbf[3],
                         HLM_U08          coeffs_num[3][2][4]);

#ifdef __cplusplus
}
#endif

#endif // _HLMC_COMMON_H_
