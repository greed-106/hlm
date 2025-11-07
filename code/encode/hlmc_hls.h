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
#ifndef _HLMC_HLS_H_
#define _HLMC_HLS_H_

#include "hlmc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* 功  能：写寄存器
* 参  数：*
*         spec            -I    算法模型句柄
*         input           -I    算法模型输入参数结构体
*         rec_dpb_idx     -I    重构图像id
*         ref_dpb_idx     -I    参考图像id
*         patch_type      -I    当前帧帧类型
*         regs            -O    寄存器参数结构体
*         rc_regs         -I    码控信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_HLS_WriteReg(HLM_SPEC                     *spec,
                             HLMC_PROCESS_IN              *input,
                             HLM_S32                       rec_dpb_idx,
                             HLM_S32                       ref_dpb_idx,
                             HLMC_PATCH_REF_TYPE           patch_type,
                             HLMC_REGS                    *regs,
                             VENC_RATE_CTRL_OUT_REGS      *rc_regs);

/***************************************************************************************************
* 功  能：读寄存器
* 参  数：*
*         spec            -O    算法模型句柄
*         output          -O    算法模型输出参数结构体
*         regs            -I    寄存器参数结构体
*         sps_pps_bits    -I    高层语法比特数
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_ReadReg(HLM_SPEC         *spec,
                          HLMC_PROCESS_OUT *output,
                          HLMC_REGS        *regs,
                          HLM_U32           sps_pps_bits);

/***************************************************************************************************
* 功  能：初始化编码库spec
* 参  数：*
*         regs         -I    寄存器参数结构体
*         spec         -O    硬件抽象层工作参数集
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_InitSpec(HLMC_REGS *regs,
                           HLM_SPEC  *spec);

/***************************************************************************************************
* 功  能：RAM空间分配
* 参  数：*
*        ram_buf                -I    上层开辟的编码内存
*        cur_cu                 -O    当前cu各层PU信息
*        best_cu                -O    当前cu最优PU信息
*        nbi_info               -O    当前cu上相邻和左相邻参考数据
*        regs                   -O    寄存器
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_HLS_AllocRam(HLM_RAM_BUF         *ram_buf,
                             HLMC_CU_INFO        *cur_cu,
                             HLMC_CU_INFO        *best_cu,
                             HLM_NEIGHBOR_INFO   *nbi_info,
                             HLMC_REGS           *regs);

/***************************************************************************************************
* 功  能：初始化 SPS参数集
* 参  数：*
*         coding_ctrl       -I         编码模型配置参数
*         sps               -O         SPS结构体
*         rate_ctrl         -I         码控参数
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_InitSeqHeader(HLMC_CODING_CTRL *coding_ctrl,
                                HLM_PARAM_SPS    *sps,
                                HLMC_RATE_CTRL   *rate_ctrl);

/***************************************************************************************************
* 功  能：初始化PPS参数集
* 参  数：*
*         coding_ctrl                   -I         编码模型配置参数
*         frame_type                    -I         帧类型
*         poc                           -I         帧号
*         pic_luma_qp                   -I         图像的初始qp
*         pps                           -O         PPS结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_InitPicHeader(HLMC_CODING_CTRL     *coding_ctrl,
                                HLMC_PATCH_REF_TYPE   frame_type,
                                HLM_S32               poc,
                                HLM_S32               pic_luma_qp,
                                HLM_PARAM_PPS        *pps);

/***************************************************************************************************
* 功  能：生成SPS参数集
* 参  数：*
*         sps              -I         SPS结构体
*         bs               -I         码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_GeneratSeqHeader(HLM_PARAM_SPS       *sps,
                                   HLMC_BITSTREAM      *bs);

/***************************************************************************************************
* 功  能：生成图像头
* 参  数：*
*         pps              -I         PPS结构体
*         sps              -I         SPS结构体
*         bs               -IO        码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_GeneratPicHeader(HLM_PARAM_PPS       *pps,
                                   HLM_PARAM_SPS       *sps,
                                   HLMC_BITSTREAM      *bs);

/***************************************************************************************************
* 功  能：生成SEI参数集
* 参  数：*
*         regs             -I         寄存器
*         bs               -IO        码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_GeneratSEI(HLMC_REGS        *regs,
                             HLMC_BITSTREAM   *bs);

#ifdef __cplusplus
}
#endif

#endif // _HLMC_HLS_H_
