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
#include "hlmd_cu.h"
#include "hlmd_ecd.h"
#include "hlmd_inter.h"
#include "hlmd_intra.h"

/***************************************************************************************************
* 功  能：解码一个宏块
* 参  数：*
*        spec            -IO    硬件解码算法句柄
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_CU_Process(HLMD_HW_SPEC  *spec)
{
    HLMD_REGS         *regs      = spec->regs;
    HLMD_PATCH_CTX    *patch_ctx = regs->patch_ctx;
    HLM_NEIGHBOR_INFO *nbi_info  = spec->nbi_info;
    HLMD_CU_INFO      *cur_cu    = spec->cur_cu;
    HLMD_BITSTREAM    *bs        = &(spec->bs);
    HLM_S32            i         = 0;

    // 解析
    HLMD_ECD_CU(cur_cu, nbi_info, regs->dec_frame_coding_type, bs , regs);

    // 初始化PU坐标
    if (cur_cu->com_cu_info.cu_type == HLM_I_4x4 || cur_cu->com_cu_info.cu_type == HLM_IBC_4x4)
    {
        for (i = 0; i < HLM_TU_4x4_NUMS; i++)
        {
            cur_cu->com_cu_info.cu_pred_info.pu_info[i].pu_x = ((i % 4) << 2);
            cur_cu->com_cu_info.cu_pred_info.pu_info[i].pu_y = ((i / 4) << 2);
        }
    }

    // 解码
    if (cur_cu->com_cu_info.cu_type < HLM_IBC_4x4)
    {
        HLMD_INTRA_Process(regs, nbi_info, cur_cu);
    }
    else if (cur_cu->com_cu_info.cu_type == HLM_IBC_4x4)
    {
        HLMD_SCC_Process(regs, nbi_info, cur_cu);
    }
    else
    {
        HLMD_INTER_Process(regs, nbi_info, cur_cu);
    }

    // 更新邻域信息
    HLM_NBI_UpdateRec(&cur_cu->com_cu_info, nbi_info,
        regs->dec_recon_y_base, regs->dec_recon_cb_base, regs->dec_recon_cr_base,
        regs->dec_output_luma_stride, regs->dec_output_chroma_stride);
    HLM_NBI_Process(&cur_cu->com_cu_info, nbi_info, regs->dec_frame_coding_type,
        &cur_cu->com_cu_info.cu_pred_info.pu_info[0],
        &cur_cu->com_cu_info.cu_pred_info.pu_info[1]);

    return HLM_STS_OK;
}
