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
#ifndef _HLMC_RC_H_
#define _HLMC_RC_H_

#include "hlmc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// 复杂度阈值
#if SAD_SATD_TH_SCALE
#define RC_COMP_TH_VF                  (1.2  * 4) 
#define RC_COMP_TH_F                   (3.2  * 4) 
#define RC_COMP_TH_C                   (6  * 4) 
#define RC_COMP_TH_VC                  (12 * 4) 
#else
#define RC_COMP_TH_VF                  (0.3)
#define RC_COMP_TH_F                   (0.8)
#define RC_COMP_TH_C                   (1.3)
#define RC_COMP_TH_VC                  (3.8)
#endif
// 不同水位下的目标比特数缩放因子
static const HLM_U08 scale_target_bits[5][5] =
{
    { 95, 130, 175, 200, 255 },        // 水位特别宽裕（放大128倍）
    { 95, 120, 134, 166, 230 },        // 水位较宽裕
    { 95, 110, 128, 153, 221 },        // 水位中等
    { 80, 110, 121, 140, 153 },        // 水位紧张
    { 80, 110, 110, 115, 128 }         // 水位非常紧张
};

static const HLM_U16 log2_rc_buffer_size[5] =
{
    15, 15, 16, 16, 16
};

static const HLM_U16 B_avg_init[5] =
{
    1842, 2034, 3840, 4352, 4864       // 8bit和10比特暂保持不变
};

static const HLM_U16 B_lossless_init[5][5] =
{
    { 921,  1227, 1842, 2304, 2763 },  // 8比特，不同复杂度下初始的无损比特数
    { 1152, 1536, 2304, 2880, 3456 },  // 10比特，不同复杂度下初始的无损比特数
    { 2720, 3640, 4692, 5204, 5716 },  // 12比特，不同复杂度下初始的无损比特数
    { 3080, 3904, 5068, 5580, 6092 },  // 14比特，不同复杂度下初始的无损比特数
    { 3400, 4152, 5264, 5776, 6288 }   // 16比特，不同复杂度下初始的无损比特数
};

static const HLM_S16 bias_tab[5][5] =
{
    { 1,  1,  1,  1,  2 },
    { 1,  1,  1,  1,  1 },
    { 0,  1,  1,  1,  1 },
    { -1,  0,  1,  1,  1 },
    { -2, -1,  0,  1,  1 },
};

typedef struct _HLMC_RC_SPEC
{
    HLMC_RATE_CTRL       rate_ctrl;
    HLMC_PATCH_REF_TYPE  patchtype;
    HLM_S32              m_targetRate_i;   // I帧目标比特数
    HLM_S32              m_targetRate_p;   // P帧目标比特数
} HLMC_RC_SPEC;

/***************************************************************************************************
* 功  能：模块所需内存计算
* 参  数：*
*         status_size        -O         状态内存大小
*         work_size          -O         工作内存大小
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_RC_GetMemSize(HLM_SZT *status_size,
                              HLM_SZT *work_size);

/***************************************************************************************************
* 功  能：模块创建（状态内存初始化，工作内存分配）
* 参  数：*
*         status_buf         -I        状态内存
*         work_buf           -I        工作内存
*         handle             -O        模块实例句柄
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_RC_Create(HLM_U08     *status_buf,
                          HLM_U08     *work_buf,
                          HLM_VOID   **handle);

/***************************************************************************************************
* 功  能：模块获取帧级QP
* 参  数：*
*         handle                -IO        模块实例句柄
*         patch_type            -I         当前待编码patch类型，等同于帧类型
*         rc_regs               -O         码控信息
*         bitdepth              -I         比特位宽
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_RC_Process(HLM_VOID                 *handle,
                           HLMC_PATCH_REF_TYPE       patch_type,
                           VENC_RATE_CTRL_OUT_REGS  *rc_regs,
                           HLM_U32                   bitdepth);

/***************************************************************************************************
* 功  能：初始化QPG
* 参  数：*
*        rc_qpg                  -IO   QPG码控结构体
*        regs                    -I    寄存器参数结构体
*        cu_cols                 -I    当前帧cu列数
*        cu_rows                 -I    当前帧cu行数
* 返回值：void
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_RC_InitQpg(HLMC_QPG      *rc_qpg,
                         HLMC_REGS     *regs,
                         HLM_S32        cu_cols,
                         HLM_S32        cu_rows);

/***************************************************************************************************
* 功  能：更新码控参数QPG
* 参  数：*
*        rc_qpg                  -IO   QPG码控结构体
*        mix_flag                -I    混合复杂度标记
*        actual_bits             -I    实际的比特
*        luma_qp                 -I    亮度qp
*        chroma_qp               -I    色度qp
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_RC_UpdateQpg(HLMC_QPG      *rc_qpg,
                           HLM_U08        mix_flag,
                           HLM_S32        actual_bits,
                           HLM_S32        luma_qp,
                           HLM_S32        chroma_qp);

/***************************************************************************************************
* 功  能：计算复杂度等级
* 参  数：*
*        rc_qpg                  -IO   QPG码控结构体
*        cur_best_satd           -I    最优的三分量satd
*        satd_comp_cur           -I    当前块各分量satd
*        bitdepth                -I    比特深度
*        yuv_comp                -I    分量个数
* 返回值：void
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_RC_CalComplexLevel(HLMC_QPG *rc_qpg,
                                 HLM_U32   cur_best_satd,
                                 HLM_U32   satd_comp_cur[3],
                                 HLM_U08   bitdepth,
#if  IBC_SCALE
                                 HLM_CU_TYPE   cu_type,
#endif
                                 HLM_U32   yuv_comp);

/***************************************************************************************************
* 功  能：计算混合复杂度块的混合等级
* 参  数：*
*        satd                 -I         亮度的8个4x4子块的satd
*        bitdepth             -I         比特深度
* 返回值：混合等级，0(非混合)、1(简单和复杂的混合)、2(非常简单和非常复杂的混合)
* 备  注：
***************************************************************************************************/
HLM_U08 HLMC_RC_CalMixFlag(HLM_U32  satd[8],
                           HLM_U08  bitdepth);

#ifdef __cplusplus
}
#endif

#endif // _HLMC_RC_H_
