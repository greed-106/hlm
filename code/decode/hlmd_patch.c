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
#include "hlmd_patch.h"
#include "hlmd_cu.h"

/***************************************************************************************************
* 功  能：解码一个patch
* 参  数：*
*         spec           -I         硬件抽象层工作参数
*         patch_ctx      -I         patch信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_PATCH_Process(HLMD_HW_SPEC      *spec,
                              HLMD_PATCH_CTX    *patch_ctx)
{
    HLM_U32 cu_x                 = 0;
    HLM_U32 cu_y                 = 0;
    HLM_STATUS sts               = HLM_STS_ERR;
    HLM_U16 *dst                 = HLM_NULL;
    HLM_U16 *src                 = HLM_NULL;
    HLM_U32  comp                = 0;
    HLM_U32  i                   = 0;
    HLM_PATCH_PARAM *patch_param = spec->regs->cur_patch_param;
    HLMD_CU_INFO    *cur_cu      = spec->cur_cu;
    HLM_PATCH_HEADER *sh         = &(patch_ctx->patch_header);
    HLMD_PIC_DATA  *pic_data     = spec->regs->cur_frame->ref_pic_data;
    HLM_U32 yuv_comp             = spec->regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_U32 patch_pos_x[3]       = { patch_param->patch_x, patch_param->patch_x, patch_param->patch_x };
    HLM_U32 patch_pos_y[3]       = { patch_param->patch_y,patch_param->patch_y,patch_param->patch_y };
    HLM_U32 real_patch_width[3]  = { HLM_MIN(patch_param->patch_width[0],  spec->regs->dec_pic_width  - patch_param->patch_x) ,
                                     HLM_MIN(patch_param->patch_width[0],  spec->regs->dec_pic_width  - patch_param->patch_x) ,
                                     HLM_MIN(patch_param->patch_width[0],  spec->regs->dec_pic_width  - patch_param->patch_x) };
    HLM_U32 real_patch_height[3] = { HLM_MIN(patch_param->patch_height[0], spec->regs->dec_pic_height - patch_param->patch_y),
                                     HLM_MIN(patch_param->patch_height[0], spec->regs->dec_pic_height - patch_param->patch_y),
                                     HLM_MIN(patch_param->patch_height[0], spec->regs->dec_pic_height - patch_param->patch_y) };
    HLM_U32 cu_x_in_segment      = 0;
    HLM_U32 cu_y_in_segment      = 0;

    switch (spec->regs->image_format)
    {
    case HLM_IMG_YUV_420:
        real_patch_width[1]  = real_patch_width [0] >> 1;
        real_patch_height[1] = real_patch_height[0] >> 1;
        patch_pos_x[1]       = patch_pos_x[0] >> 1;
        patch_pos_y[1]       = patch_pos_y[0] >> 1;
        break;
    case HLM_IMG_YUV_422:
        real_patch_width[1]  = real_patch_width [0] >> 1;
        real_patch_height[1] = real_patch_height[0];
        patch_pos_x[1]       = patch_pos_x[0] >> 1;
        patch_pos_y[1]       = patch_pos_y[0];
        break;
    }
    real_patch_width[2]  = real_patch_width[1];
    real_patch_height[2] = real_patch_height[1];
    patch_pos_x[2]       = patch_pos_x[1];
    patch_pos_y[2]       = patch_pos_y[1];

    // 设置patch_qp
    cur_cu->com_cu_info.qp[0]                   = sh->pps.pic_luma_qp;
    cur_cu->com_cu_info.qp[1]                   = cur_cu->com_cu_info.qp[0];
    cur_cu->com_cu_info.qp[2]                   = cur_cu->com_cu_info.qp[1];
    cur_cu->luma_bitdepth                       = spec->bit_depth_luma;
    cur_cu->chroma_bitdepth                     = spec->bit_depth_chroma;
    cur_cu->com_cu_info.intra_8x8_enable_flag   = spec->regs->intra_8x8_enable_flag;
    cur_cu->com_cu_info.cu_delta_qp_enable_flag = spec->regs->cu_delta_qp_enable_flag;

    // 处理所有宏块
    for (cu_y = 0; cu_y < spec->cu_rows; cu_y++)
    {
        for (cu_x = 0; cu_x < spec->cu_cols; cu_x++)
        {
            cur_cu->com_cu_info.cu_x = cu_x;
            cur_cu->com_cu_info.cu_y = cu_y;
            if (spec->regs->segment_enable_flag)
            {
                cur_cu->com_cu_info.left_unavail = (cu_x == 0) || (cu_x % (1 << (spec->regs->segment_width_in_log2 - HLM_LOG2_WIDTH_SIZE)) == 0);
                cur_cu->com_cu_info.up_unavail   = (cu_y == 0) || (cu_y % (1 << (spec->regs->segment_height_in_log2 - HLM_LOG2_HEIGHT_SIZE)) == 0);
            }
            else
            {
                cur_cu->com_cu_info.left_unavail = (cu_x == 0);
                cur_cu->com_cu_info.up_unavail   = (cu_y == 0);
            }

            sts = HLMD_CU_Process(spec);
            HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
        }
    }

    // 将patch数据patch_padding_recon裁剪后放到yuv.data
    for (comp = 0; comp < yuv_comp; comp++)
    {
        src = pic_data->patch_padding_recon[comp];
        dst = pic_data->yuv.data[comp] + patch_pos_y[comp] * pic_data->yuv.step[comp] + patch_pos_x[comp];
        for (i = 0; i < real_patch_height[comp]; i++)
        {
            memcpy(dst, src, real_patch_width[comp] * sizeof(HLM_U16));
            dst += pic_data->yuv.step[comp];
            src += pic_data->yuv.step[comp];
        }
    }

    return sts;
}
