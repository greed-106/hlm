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
#include "hlmd_inter.h"
#include "hlmd_tq.h"

// PU的帧间预测
HLM_VOID HLMD_INTER_pred(HLMD_REGS              *regs,
                         HLM_NEIGHBOR_INFO      *nbi_info,
                         HLMD_CU_INFO           *cur_cu,
                         HLM_MV                 *inter_mv,
                         HLM_S32                 square,
                         HLM_S32                 pu_x_offset)
{
    HLM_S32 pu_x          = (cur_cu->com_cu_info.cu_x << HLM_LOG2_WIDTH_SIZE) + pu_x_offset;  // PU左上角坐标
    HLM_S32 pu_y          = (cur_cu->com_cu_info.cu_y << HLM_LOG2_HEIGHT_SIZE);
    HLM_S32 pu_w          = 0;
    HLM_S32 pu_h          = 0;
    HLM_S32 ref_x         = 0;  // 匹配块的左上角点在padding后的参考帧上的像素位置
    HLM_S32 ref_y         = 0;
    HLM_S32 oft_x[3]      = { 0, cur_cu->com_cu_info.chroma_offset_x, cur_cu->com_cu_info.chroma_offset_x };  // 色度缩放因子
    HLM_S32 oft_y[3]      = { 0, cur_cu->com_cu_info.chroma_offset_y, cur_cu->com_cu_info.chroma_offset_y };
    HLM_U16 *ref_base[3]  = { regs->dec_ref_y_padding_base, regs->dec_ref_cb_padding_base, regs->dec_ref_cr_padding_base };
    HLM_U32 ref_stride[3] = { regs->dec_ref_frame_luma_stride, regs->dec_ref_frame_chroma_stride, regs->dec_ref_frame_chroma_stride };
    HLM_S32 yuv_comp      = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_S32 yuv_idx       = 0;
    HLM_U16 *dist         = HLM_NULL;

    inter_mv->mvx = HLM_CLIP(inter_mv->mvx, -regs->inter_pad_w_left, regs->inter_pad_w_right);
    inter_mv->mvy = HLM_CLIP(inter_mv->mvy, -regs->inter_pad_h_up,   regs->inter_pad_h_down);

    for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
    {
        // 预测第一个8x8
        pu_w  = 1 << cur_cu->com_cu_info.cu_width[yuv_idx] >> 1;
        pu_h  = 1 << cur_cu->com_cu_info.cu_height[yuv_idx];
        ref_x = ((pu_x + inter_mv->mvx) >> oft_x[yuv_idx]) + regs->inter_pad_w_left;
        ref_y = ((pu_y + inter_mv->mvy) >> oft_y[yuv_idx]) + regs->inter_pad_h_up;
        dist  = cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + (pu_x_offset >> oft_x[yuv_idx]);
        HLM_COM_GetBlock(ref_base[yuv_idx], ref_stride[yuv_idx], dist, HLM_WIDTH_SIZE, ref_x, ref_y, pu_w, pu_h);

        // 预测第二个8x8
        if (!square)
        {
            dist += (8 >> oft_x[yuv_idx]);
            ref_x += (8 >> oft_x[yuv_idx]);
            HLM_COM_GetBlock(ref_base[yuv_idx], ref_stride[yuv_idx], dist, HLM_WIDTH_SIZE, ref_x, ref_y, pu_w, pu_h);
        }
    }

    return;
}

/***************************************************************************************************
* 功  能：帧间预测模块
* 参  数：*
*        regs                 -I     当前数据硬件寄存器
*        nbi_info             -I     相邻块信息
*        cur_cu               -I/O   当前CU
* 返回值：无
***************************************************************************************************/
HLM_VOID HLMD_INTER_Process(HLMD_REGS           *regs,
                            HLM_NEIGHBOR_INFO   *nbi_info,
                            HLMD_CU_INFO        *cur_cu)
{
    HLM_S32  i                      = 0;
    HLM_S32  j                      = 0;
    HLM_U32  available[PART_NUM][3] = { 0 };
    HLM_MV   nbi_mv[PART_NUM][3]    = { 0 };
    HLM_S32  ref_idx[PART_NUM][3]   = { 0 };
    HLM_U08  yuv_idx                = 0;
    HLM_U16 *cur_cu_rec             = HLM_NULL;
    HLM_U32  rec_stride             = 0;
    HLM_S32  yuv_comp               = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_S32  cu_width[3]            = { 1 << cur_cu->com_cu_info.cu_width[0],
                                        1 << cur_cu->com_cu_info.cu_width[1],
                                        1 << cur_cu->com_cu_info.cu_width[2] };
    HLM_S32  cu_height[3]           = { 1 << cur_cu->com_cu_info.cu_height[0],
                                        1 << cur_cu->com_cu_info.cu_height[1],
                                        1 << cur_cu->com_cu_info.cu_height[2] };
    HLM_S32 is_right_cu             = ((cur_cu->com_cu_info.cu_x + 1) << 4) >= regs->cur_patch_param->patch_coded_width[0];

    if (HLM_P_SKIP == cur_cu->com_cu_info.cu_type)
    {
        HLM_COM_InterSkipMvp(&cur_cu->com_cu_info, nbi_info, is_right_cu, &cur_cu->com_cu_info.cu_pred_info.skip_mvp);

        cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mv = cur_cu->com_cu_info.cu_pred_info.skip_mvp;
        HLMD_INTER_pred(regs, nbi_info, cur_cu, &cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mv, 0, 0);

        // 拷贝重建
        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            if (yuv_idx == HLM_LUMA_Y)
            {
                cur_cu_rec = regs->dec_recon_y_base;
                rec_stride = regs->dec_output_luma_stride;
            }
            else if (yuv_idx == HLM_CHROMA_U)
            {
                cur_cu_rec = regs->dec_recon_cb_base;
                rec_stride = regs->dec_output_chroma_stride;
            }
            else
            {
                cur_cu_rec = regs->dec_recon_cr_base;
                rec_stride = regs->dec_output_chroma_stride;
            }
            cur_cu_rec += (cur_cu->com_cu_info.cu_y * rec_stride * cu_height[yuv_idx] + cur_cu->com_cu_info.cu_x * cu_width[yuv_idx]);
            for (j = 0; j < cu_height[yuv_idx]; j++)
            {
                memcpy(cur_cu_rec + rec_stride * j,
                    cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + HLM_WIDTH_SIZE * j, cu_width[yuv_idx] * sizeof(HLM_U16));
            }
        }
    }
    else
    {
        if (HLM_P_16x8 == cur_cu->com_cu_info.cu_type)
        {
            HLM_COM_InterMvpPre(&cur_cu->com_cu_info, &cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mv,
                nbi_info, is_right_cu, PROC_BS, available, nbi_mv, ref_idx);
            HLM_COM_GetMvp(available[PART_16x8], 0, 0, nbi_mv[PART_16x8], ref_idx[PART_16x8],
                &cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mvp);

            cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mv.mvx = cur_cu->mvd_l0[0].mvx
                + cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mvp.mvx;
            cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mv.mvy = cur_cu->mvd_l0[0].mvy
                + cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mvp.mvy;
            HLMD_INTER_pred(regs, nbi_info, cur_cu, &cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mv, 0, 0);

            HLMD_TQ_Process(cur_cu, 0, yuv_comp);

            // 拷贝重建
            for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
            {
                if (yuv_idx == HLM_LUMA_Y)
                {
                    cur_cu_rec = regs->dec_recon_y_base;
                    rec_stride = regs->dec_output_luma_stride;
                }
                else if (yuv_idx == HLM_CHROMA_U)
                {
                    cur_cu_rec = regs->dec_recon_cb_base;
                    rec_stride = regs->dec_output_chroma_stride;
                }
                else
                {
                    cur_cu_rec = regs->dec_recon_cr_base;
                    rec_stride = regs->dec_output_chroma_stride;
                }
                cur_cu_rec += (cur_cu->com_cu_info.cu_y * rec_stride * cu_height[yuv_idx] + cur_cu->com_cu_info.cu_x * cu_width[yuv_idx]);
                for (j = 0; j < cu_height[yuv_idx]; j++)
                {
                    memcpy(cur_cu_rec + rec_stride * j,
                        cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + HLM_WIDTH_SIZE * j, cu_width[yuv_idx] * sizeof(HLM_U16));
                }
            }
        }
        else if (HLM_P_8x8 == cur_cu->com_cu_info.cu_type)
        {
            for (i = 0; i < 2; i++)
            {
                HLM_COM_InterMvpPre(&cur_cu->com_cu_info, &cur_cu->com_cu_info.cu_pred_info.pu_info[0].inter_mv,
                    nbi_info, is_right_cu, PROC_BS, available, nbi_mv, ref_idx);
                HLM_COM_GetMvp(available[PART_8x8_0 + i], 8 * i, 0, nbi_mv[PART_8x8_0 + i], ref_idx[PART_8x8_0 + i],
                    &cur_cu->com_cu_info.cu_pred_info.pu_info[i].inter_mvp);

                cur_cu->com_cu_info.cu_pred_info.pu_info[i].inter_mv.mvx = cur_cu->mvd_l0[i].mvx
                    + cur_cu->com_cu_info.cu_pred_info.pu_info[i].inter_mvp.mvx;
                cur_cu->com_cu_info.cu_pred_info.pu_info[i].inter_mv.mvy = cur_cu->mvd_l0[i].mvy
                    + cur_cu->com_cu_info.cu_pred_info.pu_info[i].inter_mvp.mvy;
                HLMD_INTER_pred(regs, nbi_info, cur_cu, &cur_cu->com_cu_info.cu_pred_info.pu_info[i].inter_mv, 1, 8 * i);
            }

            HLMD_TQ_Process(cur_cu, 0, yuv_comp);

            // 拷贝重建
            for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
            {
                if (yuv_idx == HLM_LUMA_Y)
                {
                    cur_cu_rec = regs->dec_recon_y_base;
                    rec_stride = regs->dec_output_luma_stride;
                }
                else if (yuv_idx == HLM_CHROMA_U)
                {
                    cur_cu_rec = regs->dec_recon_cb_base;
                    rec_stride = regs->dec_output_chroma_stride;
                }
                else
                {
                    cur_cu_rec = regs->dec_recon_cr_base;
                    rec_stride = regs->dec_output_chroma_stride;
                }
                cur_cu_rec += (cur_cu->com_cu_info.cu_y * rec_stride * cu_height[yuv_idx] + cur_cu->com_cu_info.cu_x * cu_width[yuv_idx]);
                for (j = 0; j < cu_height[yuv_idx]; j++)
                {
                    memcpy(cur_cu_rec + rec_stride * j,
                        cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + HLM_WIDTH_SIZE * j, cu_width[yuv_idx] * sizeof(HLM_U16));
                }
            }
        }
    }

    return;
}
