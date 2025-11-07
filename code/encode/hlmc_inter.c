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
#include "hlmc_inter.h"
#include "hlmc_common.h"
#include "hlmc_tq.h"
#include "hlmc_ecd.h"

// 隔点搜索后的mv周围十字的某一点搜索
HLM_VOID HLMC_INTER_search_nearby(HLM_INTER_PART_TYPE    inter_part_type,
                                  HLM_MV                *new_mv,
                                  HLM_U16               *cu_src,
                                  HLM_U16               *cu_dst,
                                  HLM_S32                src_stride,
                                  HLM_S32                dst_stride,
                                  HLM_MV                *mvp,
                                  HLM_U64                lambda,
                                  HLM_U64               *best_cost,
                                  HLM_MV                *best_mv)
{
    HLM_U16 *block_dst = HLM_NULL;
    HLM_U64 dist       = 0;
    HLM_U64 rate       = 0;
    HLM_MV  mvd        = { 0 };
    HLM_U64 cost       = 0;

    if ((inter_part_type == PART_16x8) || (inter_part_type == PART_8x8_0))
    {
        block_dst = cu_dst + new_mv->mvy * dst_stride + new_mv->mvx;
        dist += HLMC_COM_ComputeSad(cu_src, block_dst, 8, 8, src_stride, dst_stride);
    }
    if ((inter_part_type == PART_16x8) || (inter_part_type == PART_8x8_1))
    {
        block_dst = cu_dst + new_mv->mvy * dst_stride + new_mv->mvx + 8;
        dist += HLMC_COM_ComputeSad(cu_src + 8, block_dst, 8, 8, src_stride, dst_stride);
    }

    mvd.mvx = new_mv->mvx - mvp->mvx;
    mvd.mvy = new_mv->mvy - mvp->mvy;
    rate = HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvx, 0, HLM_NULL)
         + HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvy, 0, HLM_NULL);
    cost = dist + ((lambda * rate) >> HLMC_LAMBDA_SHIFT);
    if (cost < best_cost[inter_part_type])
    {
        best_cost[inter_part_type] = cost;
        best_mv[inter_part_type].mvx = new_mv->mvx;
        best_mv[inter_part_type].mvy = new_mv->mvy;
    }
}

// 获取mvp并进行运动估计，选择最优mv并保存
HLM_VOID HLMC_INTER_search(HLMC_CU_INFO        *cur_cu,
                           HLM_NEIGHBOR_INFO   *nbi_info,
                           HLMC_REGS           *regs)
{
    HLM_S32 i                      = 0;
    HLM_S32 j                      = 0;
    HLM_S32 k                      = 0;
    HLM_S32 start                  = 0;
    HLM_S32 step                   = 0;
    HLM_U16 *cu_src                = HLM_NULL;
    HLM_U16 *cu_dst                = HLM_NULL;
    HLM_U16 *block_dst             = HLM_NULL;
    HLM_U32 sad[4]                 = { 0 };
    HLM_U64 dist[PART_NUM]         = { 0 };
    HLM_U64 rate                   = 0;
    HLM_U64 cost                   = 0;
    HLM_U64 best_cost[PART_NUM]    = { HLM_MAX_S64, HLM_MAX_S64, HLM_MAX_S64 };
    HLM_S32 src_stride             = regs->enc_input_luma_stride;
    HLM_S32 dst_stride             = regs->enc_ref_frame_luma_stride;
    HLM_U64 lambda                 = regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]];
    HLM_MV  mvp[PART_NUM]          = { 0 };
    HLM_MV  best_mv[PART_NUM]      = { 0 };
    HLM_MV  new_mv                 = { 0 };
    HLM_MV  mvd                    = { 0 };
    HLM_S32 is_right_cu            = 0;
    HLM_U32 available[PART_NUM][3] = { 0 };
    HLM_MV  nbi_mv[PART_NUM][3]    = { 0 };
    HLM_S32 ref_idx[PART_NUM][3]   = { 0 };
    HLM_U32 part_w                 = 16;
    HLM_U32 part_h                 = 16;
    HLM_U32 part_start_x           = 0;
    HLM_U32 part_start_y           = 0;
    HLM_U08 inter_interval_search  = 0;  // 0:全搜索，1:隔点搜索

    cu_src = regs->enc_input_y_base + cur_cu->com_cu_info.cu_y * src_stride* HLM_HEIGHT_SIZE + cur_cu->com_cu_info.cu_x * HLM_WIDTH_SIZE;
    cu_dst = regs->enc_ref_y_padding_base + (cur_cu->com_cu_info.cu_y * HLM_HEIGHT_SIZE + regs->inter_pad_h_up) * dst_stride
           + (cur_cu->com_cu_info.cu_x * HLM_WIDTH_SIZE + regs->inter_pad_w_left);
    is_right_cu = (((cur_cu->com_cu_info.cu_x + 1) << 4) >= regs->cur_patch_param->patch_coded_width[0]);
    HLM_COM_InterMvpPre(&cur_cu->com_cu_info, &cur_cu->pu_info_enc[0].mv_8x8,
        nbi_info, is_right_cu, PROC_PRE, available, nbi_mv, ref_idx);

    step = inter_interval_search ? 2 : 1;
    for (j = -regs->inter_pad_h_up; j <= regs->inter_pad_h_down; j++)
    {
        start = (inter_interval_search && (j % 2)) ? 1 : 0;
        for (i = start - regs->inter_pad_w_left; i <= regs->inter_pad_w_right; i += step)
        {
            block_dst = cu_dst + j * dst_stride + i;
            sad[0] = HLMC_COM_ComputeSad(cu_src, block_dst, 8, 8, src_stride, dst_stride);

            block_dst = cu_dst + j * dst_stride + i + 8;
            sad[1] = HLMC_COM_ComputeSad(cu_src + 8, block_dst, 8, 8, src_stride, dst_stride);

            dist[PART_16x8]  = sad[0] + sad[1];
            dist[PART_8x8_0] = sad[0];
            dist[PART_8x8_1] = sad[1];
            for (k = PART_16x8; k < PART_NUM; k++)
            {
                part_w = ((k == PART_8x8_0) || (k == PART_8x8_1)) ? 8 : 16;
                part_h = 8;
                part_start_x = (k == PART_8x8_1) ? 8 : 0;
                part_start_y = 0;
                HLM_COM_GetMvp(available[k], part_start_x, part_start_y, nbi_mv[k], ref_idx[k], &mvp[k]);

                mvd.mvx = i - mvp[k].mvx;
                mvd.mvy = j - mvp[k].mvy;
                rate = HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvx, 0, HLM_NULL)
                     + HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvy, 0, HLM_NULL);
                cost = dist[k] + ((lambda * rate) >> HLMC_LAMBDA_SHIFT);
                if (cost < best_cost[k])
                {
                    best_cost[k] = cost;
                    best_mv[k].mvx = i;
                    best_mv[k].mvy = j;
                }
            }
        }
    }

    if (inter_interval_search)  // 周围微调
    {
        for (i = PART_16x8; i < PART_NUM; i++)
        {
            if (best_mv[i].mvx > -regs->inter_pad_w_left)
            {
                new_mv.mvx = best_mv[i].mvx - 1;
                new_mv.mvy = best_mv[i].mvy;
                HLMC_INTER_search_nearby(i, &new_mv, cu_src, cu_dst, src_stride, dst_stride, &mvp[i], lambda, best_cost, best_mv);
            }
            if (best_mv[i].mvx < regs->inter_pad_w_right)
            {
                new_mv.mvx = best_mv[i].mvx + 1;
                new_mv.mvy = best_mv[i].mvy;
                HLMC_INTER_search_nearby(i, &new_mv, cu_src, cu_dst, src_stride, dst_stride, &mvp[i], lambda, best_cost, best_mv);
            }
            if (best_mv[i].mvy > -regs->inter_pad_h_up)
            {
                new_mv.mvx = best_mv[i].mvx;
                new_mv.mvy = best_mv[i].mvy - 1;
                HLMC_INTER_search_nearby(i, &new_mv, cu_src, cu_dst, src_stride, dst_stride, &mvp[i], lambda, best_cost, best_mv);
            }
            if (best_mv[i].mvy < regs->inter_pad_h_down)
            {
                new_mv.mvx = best_mv[i].mvx;
                new_mv.mvy = best_mv[i].mvy + 1;
                HLMC_INTER_search_nearby(i, &new_mv, cu_src, cu_dst, src_stride, dst_stride, &mvp[i], lambda, best_cost, best_mv);
            }
        }
    }

    memcpy(&cur_cu->pu_info_enc[0].mv_16x8, &best_mv[PART_16x8], sizeof(HLM_MV));
    memcpy(&cur_cu->pu_info_enc[0].mv_8x8, &best_mv[PART_8x8_0], sizeof(HLM_MV));
    memcpy(&cur_cu->pu_info_enc[1].mv_8x8, &best_mv[PART_8x8_1], sizeof(HLM_MV));
    memcpy(cur_cu->com_cu_info.cu_pred_info.inter_mvp, mvp, PART_NUM * sizeof(HLM_MV));

    HLM_COM_InterSkipMvp(&cur_cu->com_cu_info, nbi_info, is_right_cu, &cur_cu->com_cu_info.cu_pred_info.skip_mvp);
}

/***************************************************************************************************
* 功  能：整像素运动搜索
* 参  数：*
*        regs                 -I    当前数据硬件寄存器
*        nbi_info             -I    相邻块信息
*        cur_cu               -IO   当前cu信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTER_MODE(HLMC_REGS           *regs,
                         HLM_NEIGHBOR_INFO   *nbi_info,
                         HLMC_CU_INFO        *cur_cu)
{
    HLM_U64 lambda              = regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]];
    HLM_S32 cu_pixel_x[3]       = { cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0],
                                    cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1],
                                    cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2] };
    HLM_S32 cu_pixel_y[3]       = { cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0],
                                    cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1],
                                    cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2] };
    HLM_S32 cu_pixel_w[3]       = { 1 << cur_cu->com_cu_info.cu_width[0],
                                    1 << cur_cu->com_cu_info.cu_width[1],
                                    1 << cur_cu->com_cu_info.cu_width[2] };
    HLM_S32 cu_pixel_h[3]       = { 1 << cur_cu->com_cu_info.cu_height[0],
                                    1 << cur_cu->com_cu_info.cu_height[1],
                                    1 << cur_cu->com_cu_info.cu_height[2] };
    HLM_S32 cu_offset_x[3]      = { 0, cur_cu->com_cu_info.chroma_offset_x, cur_cu->com_cu_info.chroma_offset_x };
    HLM_S32 cu_offset_y[3]      = { 0, cur_cu->com_cu_info.chroma_offset_y, cur_cu->com_cu_info.chroma_offset_y };
    HLM_S32 src_stride          = regs->enc_input_luma_stride;
    HLM_S32 ref_stride          = regs->enc_ref_frame_luma_stride;
    HLM_U16 blk_pred[2][3][128] = { { { 0 } } };  // 0:16x8, 1:8x8
    HLM_S32 satd                = 0;
    HLM_S32 rate                = 0;
    HLM_S64 cost                = 0;
    HLM_S64 best_cost           = HLM_MAX_S64;
    HLM_CU_PRED_INFO *chan_info = &cur_cu->com_cu_info.cu_pred_info;
    HLM_S32 yuv_idx             = 0;
    HLM_S32 satd_comp[2][3]     = { 0 };
    HLM_U16 *ptr_src[3]         = { HLM_NULL };   // 指向当前帧中的当前块的左上角
    HLM_U16 *ptr_ref[3]         = { HLM_NULL };   // 指向参考帧中的同位块的左上角
    HLM_U16 *src                = HLM_NULL;
    HLM_U16 *dst                = HLM_NULL;
    HLM_MV  mvd                 = { 0 };
    HLM_S32 part_type           = 0;              // 0:16x8, 1:8x8
    HLM_S32 best_part           = 0;
    HLM_S32 i                   = 0;
    HLM_S32 j                   = 0;
    HLM_S32 yuv_comp            = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;

    HLMC_INTER_search(cur_cu, nbi_info, regs);

    ptr_src[0] = regs->enc_input_y_base + cu_pixel_y[0] * regs->enc_input_luma_stride + cu_pixel_x[0];
    ptr_src[1] = regs->enc_input_cb_base + cu_pixel_y[1] * regs->enc_input_chroma_stride + cu_pixel_x[1];
    ptr_src[2] = regs->enc_input_cr_base + cu_pixel_y[2] * regs->enc_input_chroma_stride + cu_pixel_x[2];
    ptr_ref[0] = regs->enc_ref_y_padding_base + (cu_pixel_y[0] + regs->inter_pad_h_up) * regs->enc_ref_frame_luma_stride
               + (cu_pixel_x[0] + regs->inter_pad_w_left);
    ptr_ref[1] = regs->enc_ref_cb_padding_base + (cu_pixel_y[1] + regs->inter_pad_h_up) * regs->enc_ref_frame_chroma_stride
               + (cu_pixel_x[1] + regs->inter_pad_w_left);
    ptr_ref[2] = regs->enc_ref_cr_padding_base + (cu_pixel_y[2] + regs->inter_pad_h_up) * regs->enc_ref_frame_chroma_stride
               + (cu_pixel_x[2] + regs->inter_pad_w_left);

    // 获取预测像素
    for (yuv_idx = HLM_LUMA_Y; yuv_idx < yuv_comp; yuv_idx++)
    {
        ref_stride = (yuv_idx == HLM_LUMA_Y) ? regs->enc_ref_frame_luma_stride : regs->enc_ref_frame_chroma_stride;
        dst = blk_pred[0][yuv_idx];
        src = ptr_ref[yuv_idx] + (cur_cu->pu_info_enc[0].mv_16x8.mvy >> cu_offset_y[yuv_idx]) * ref_stride
            + (cur_cu->pu_info_enc[0].mv_16x8.mvx >> cu_offset_x[yuv_idx]);
        for (i = 0; i < cu_pixel_h[yuv_idx]; i++)
        {
            memcpy(dst, src, cu_pixel_w[yuv_idx] * sizeof(HLM_U16));
            dst += HLM_WIDTH_SIZE;
            src += ref_stride;
        }
    }
    for (yuv_idx = HLM_LUMA_Y; yuv_idx < yuv_comp; yuv_idx++)
    {
        ref_stride = (yuv_idx == HLM_LUMA_Y) ? regs->enc_ref_frame_luma_stride : regs->enc_ref_frame_chroma_stride;
        for (j = 0; j < 2; j++)
        {
            dst = blk_pred[1][yuv_idx] + j * (cu_pixel_w[yuv_idx] >> 1);
            src = ptr_ref[yuv_idx] + j * (cu_pixel_w[yuv_idx] >> 1) + (cur_cu->pu_info_enc[j].mv_8x8.mvy >> cu_offset_y[yuv_idx]) * ref_stride
                + (cur_cu->pu_info_enc[j].mv_8x8.mvx >> cu_offset_x[yuv_idx]);
            for (i = 0; i < cu_pixel_h[yuv_idx]; i++)
            {
                memcpy(dst, src, (cu_pixel_w[yuv_idx] >> 1) * sizeof(HLM_U16));
                dst += HLM_WIDTH_SIZE;
                src += ref_stride;
            }
        }
    }

    // 根据SATD决策PU划分
    for (part_type = 0; part_type <= 1; part_type++)
    {
        for (yuv_idx = HLM_LUMA_Y; yuv_idx < yuv_comp; yuv_idx++)
        {
            src_stride = (yuv_idx == HLM_LUMA_Y) ? regs->enc_input_luma_stride : regs->enc_input_chroma_stride;
            satd = 0;
            for (i = 0; i < cu_pixel_h[yuv_idx]; i += 4)
            {
                for (j = 0; j < (cu_pixel_w[yuv_idx]); j += 4)
                {
#if PK_WITH_SATD
                    satd += HLMC_COM_ComputeSatd4x4(ptr_src[yuv_idx] + i * src_stride + j,
                        blk_pred[part_type][yuv_idx] + i * HLM_WIDTH_SIZE + j, src_stride, HLM_WIDTH_SIZE);
#else
                    satd += HLMC_COM_ComputeSad(ptr_src[yuv_idx] + i * src_stride + j,
                        blk_pred[part_type][yuv_idx] + i * HLM_WIDTH_SIZE + j,
                        4, 4, src_stride, HLM_WIDTH_SIZE);
#endif
                }
            }
            satd_comp[part_type][yuv_idx] = satd;
        }
        if (part_type == 0)
        {
            rate = 1;  // cu_type, HLM_P_16x8
            mvd.mvx = cur_cu->pu_info_enc[0].mv_16x8.mvx - chan_info->inter_mvp[PART_16x8].mvx;
            mvd.mvy = cur_cu->pu_info_enc[0].mv_16x8.mvy - chan_info->inter_mvp[PART_16x8].mvy;
            rate += HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvx, 0, HLM_NULL);
            rate += HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvy, 0, HLM_NULL);
        }
        else
        {
            rate = 3;  // cu_type, HLM_P_8x8
            mvd.mvx = cur_cu->pu_info_enc[0].mv_8x8.mvx - chan_info->inter_mvp[PART_8x8_0].mvx;
            mvd.mvy = cur_cu->pu_info_enc[0].mv_8x8.mvy - chan_info->inter_mvp[PART_8x8_0].mvy;
            rate += HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvx, 0, HLM_NULL);
            rate += HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvy, 0, HLM_NULL);

            mvd.mvx = cur_cu->pu_info_enc[1].mv_8x8.mvx - chan_info->inter_mvp[PART_8x8_1].mvx;
            mvd.mvy = cur_cu->pu_info_enc[1].mv_8x8.mvy - chan_info->inter_mvp[PART_8x8_1].mvy;
            rate += HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvx, 0, HLM_NULL);
            rate += HLMC_ECD_PutSeBits(HLM_NULL, mvd.mvy, 0, HLM_NULL);
        }
        satd = satd_comp[part_type][0] + satd_comp[part_type][1] + satd_comp[part_type][2];
        cost = satd + ((lambda * rate) >> HLMC_LAMBDA_SHIFT);
        if (cost <= best_cost)
        {
            best_cost = cost;
            best_part = part_type;
        }
    }

    // 获取inter模式的预测值
    chan_info->part_type = (best_part == 0 ? HLM_P_16x8 : HLM_P_8x8);
    for (yuv_idx = HLM_LUMA_Y; yuv_idx < yuv_comp; yuv_idx++)
    {
        memcpy(chan_info->inter_pred[yuv_idx], blk_pred[best_part][yuv_idx], sizeof(HLM_U16) *  HLM_CU_SIZE);
    }

    // 获取skip模式的预测值（skip模式不划分）
    for (yuv_idx = HLM_LUMA_Y; yuv_idx < yuv_comp; yuv_idx++)
    {
        ref_stride = yuv_idx == HLM_LUMA_Y ? regs->enc_ref_frame_luma_stride : regs->enc_ref_frame_chroma_stride;
        dst = chan_info->skip_pred[yuv_idx];
        src = ptr_ref[yuv_idx] + (chan_info->skip_mvp.mvy >> cu_offset_y[yuv_idx]) * ref_stride + (chan_info->skip_mvp.mvx >> cu_offset_x[yuv_idx]);
        for (i = 0; i < cu_pixel_h[yuv_idx]; i++)
        {
            memcpy(dst, src, cu_pixel_w[yuv_idx] * sizeof(HLM_U16));
            dst += HLM_WIDTH_SIZE;
            src += ref_stride;
        }
    }

    // 输出satd给码控，用于计算复杂度
    cur_cu->inter_satd_cost = satd_comp[best_part][0] + satd_comp[best_part][1] + satd_comp[best_part][2];
    if (cur_cu->intra_satd_cost > cur_cu->inter_satd_cost)
    {
        cur_cu->intra_satd_cost = (HLM_U32)cur_cu->inter_satd_cost;
        cur_cu->satd_comp[0] = satd_comp[best_part][0];
        cur_cu->satd_comp[1] = satd_comp[best_part][1];
        cur_cu->satd_comp[2] = satd_comp[best_part][2];
#if  IBC_SCALE
        cur_cu->best_cu_type = chan_info->part_type;
#endif
    }
}

// 对运动估计产生的最优划分及mv计算Rdcost
HLM_VOID HLMC_INTER_me_rdcost(HLMC_CU_INFO        *cur_cu,
                              HLM_NEIGHBOR_INFO   *nbi_info,
                              HLMC_REGS           *regs,
                              HLM_U32             *me_rdcost)
{
    HLM_S32 i                      = 0;
    HLM_S32 idx_4                  = 0;
    HLM_U32 bits                   = 0;
    HLM_MV  mvd_l0[2]              = {0};
    HLM_MV  mvp[PART_NUM]          = {0};
    HLM_U16 *cur_cu_src[3]         = { HLM_NULL };
    HLM_U16 *cur_cu_pred[3]        = { HLM_NULL };
    HLM_U16 *cur_cu_rec[3]         = { HLM_NULL };
    HLM_COEFF *cur_cu_res[3]       = { HLM_NULL };
    HLM_U16 *rec_tmp               = HLM_NULL;
    HLM_U32 src_stride[3]          = {0};
    HLM_U32 distortion             = 0;
    HLM_S32 is_right_cu            = 0;
    HLM_U32 channel_size           = 16;
    HLM_CU_PRED_INFO *cur_chan     = &cur_cu->com_cu_info.cu_pred_info;
    HLM_U32 lambda                 = regs->enc_sse_lamda[cur_cu->com_cu_info.qp[0]];
    HLM_U32 available[PART_NUM][3] = { 0 };
    HLM_MV  nbi_mv[PART_NUM][3]    = { 0 };
    HLM_S32 ref_idx[PART_NUM][3]   = { 0 };
    HLM_U08 part_4x4_num           = (1 << (cur_cu->com_cu_info.cu_width[0] + cur_cu->com_cu_info.cu_height[0] - 4));
    HLM_U08 x                      = 0;
    HLM_U08 y                      = 0;
    HLM_U32 cu_width_w[3]          = { 1 << cur_cu->com_cu_info.cu_width[0],  1 << cur_cu->com_cu_info.cu_width[1],  1 << cur_cu->com_cu_info.cu_width[2] };
    HLM_U32 cu_width_h[3]          = { 1 << cur_cu->com_cu_info.cu_height[0], 1 << cur_cu->com_cu_info.cu_height[1], 1 << cur_cu->com_cu_info.cu_height[2] };
    HLM_S32 yuv_comp               = regs->image_format == HLM_IMG_YUV_400 ? 1 : HLM_MAX_COMP_NUM;

    // 计算残差
    for (i = 0; i < yuv_comp; i++)
    {
        if (i == HLM_LUMA_Y)
        {
            cur_cu_src[i] = regs->enc_input_y_base;
            src_stride[i] = regs->enc_input_luma_stride;
        }
        else if (i == HLM_CHROMA_U)
        {
            cur_cu_src[i] = regs->enc_input_cb_base;
            src_stride[i] = regs->enc_input_chroma_stride;
        }
        else
        {
            cur_cu_src[i] = regs->enc_input_cr_base;
            src_stride[i] = regs->enc_input_chroma_stride;
        }
        cur_cu_src[i] += (cur_cu->com_cu_info.cu_y * src_stride[i] * cu_width_h[i] + cur_cu->com_cu_info.cu_x * cu_width_w[i]);
        cur_cu_pred[i] = cur_chan->inter_pred[i];
        cur_cu_res[i]  = cur_chan->res[i];
        cur_cu_rec[i]  = cur_chan->rec[i];
        HLMC_COM_ComputeRes(cur_cu_src[i], cur_cu_pred[i], cur_cu_res[i],
            cu_width_w[i], cu_width_h[i], src_stride[i], HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
    }

    // 重建和失真
    cur_cu->com_cu_info.cu_type = HLM_P_16x8;
    HLMC_TQ_Process(0, 16, regs->bitdepth, yuv_comp, cur_cu);
    for (i = 0; i < yuv_comp; i++)
    {
        part_4x4_num = (1 << (cur_cu->com_cu_info.cu_width[i] + cur_cu->com_cu_info.cu_height[i] - 4));
        for (idx_4 = 0; idx_4 < part_4x4_num; idx_4++)
        {
            HLM_COM_4x4_Block_To_Coeff_Pos(part_4x4_num, idx_4, &x, &y, &cur_cu->com_cu_info, i);
        }
        rec_tmp = cur_chan->rec[i];
        HLM_COM_AddRes(rec_tmp, cur_cu_pred[i], cur_chan->res[i], regs->bitdepth,
            cu_width_w[i], cu_width_h[i], HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
        distortion += HLMC_COM_ComputeSse(cur_cu_src[i], rec_tmp, cu_width_w[i], cu_width_h[i], src_stride[i], HLM_WIDTH_SIZE);
    }

    // 比特估计
    is_right_cu = (((cur_cu->com_cu_info.cu_x + 1) << 4) >= regs->cur_patch_param->patch_coded_width[0]);
    HLM_COM_InterMvpPre(&cur_cu->com_cu_info, &cur_cu->pu_info_enc[0].mv_8x8,
        nbi_info, is_right_cu, PROC_RDO, available, nbi_mv, ref_idx);
    if (cur_chan->part_type == HLM_P_16x8)
    {
        HLM_COM_GetMvp(available[PART_16x8],  0, 0, nbi_mv[PART_16x8], ref_idx[PART_16x8], &cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp);
    }
    else
    {
        HLM_COM_GetMvp(available[PART_8x8_0],  0, 0, nbi_mv[PART_8x8_0], ref_idx[PART_8x8_0], &cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp);
        HLM_COM_GetMvp(available[PART_8x8_1], 8, 0, nbi_mv[PART_8x8_1], ref_idx[PART_8x8_1], &cur_cu->pu_info_enc[1].inter_pu_info.inter_mvp);
    }
    HLMC_COM_GetCbf(cur_cu->com_cu_info.cu_type, cur_cu->com_cu_info.cbf, cur_cu->com_cu_info.coeffs_num);

    bits = HLMC_ECD_EncodeCuType(cur_chan->part_type, HLM_FRAME_TYPE_P, 0, HLM_NULL, regs->p_frame_enable_ibc);
    if (cur_chan->part_type == HLM_P_16x8)
    {
        mvd_l0[0].mvx = cur_cu->pu_info_enc[0].mv_16x8.mvx - cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp.mvx;
        mvd_l0[0].mvy = cur_cu->pu_info_enc[0].mv_16x8.mvy - cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp.mvy;
        bits += HLMC_ECD_PutSeBits(HLM_NULL, mvd_l0[0].mvx, 0, HLM_NULL);
        bits += HLMC_ECD_PutSeBits(HLM_NULL, mvd_l0[0].mvy, 0, HLM_NULL);
    }
    else
    {
        mvd_l0[0].mvx = cur_cu->pu_info_enc[0].mv_8x8.mvx - cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp.mvx;
        mvd_l0[0].mvy = cur_cu->pu_info_enc[0].mv_8x8.mvy - cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp.mvy;
        bits += HLMC_ECD_PutSeBits(HLM_NULL, mvd_l0[0].mvx, 0, HLM_NULL);
        bits += HLMC_ECD_PutSeBits(HLM_NULL, mvd_l0[0].mvy, 0, HLM_NULL);

        mvd_l0[1].mvx = cur_cu->pu_info_enc[1].mv_8x8.mvx - cur_cu->pu_info_enc[1].inter_pu_info.inter_mvp.mvx;
        mvd_l0[1].mvy = cur_cu->pu_info_enc[1].mv_8x8.mvy - cur_cu->pu_info_enc[1].inter_pu_info.inter_mvp.mvy;
        bits += HLMC_ECD_PutSeBits(HLM_NULL, mvd_l0[1].mvx, 0, HLM_NULL);
        bits += HLMC_ECD_PutSeBits(HLM_NULL, mvd_l0[1].mvy, 0, HLM_NULL);
    }

    if (cur_cu->com_cu_info.cbf[0] && cur_cu->com_cu_info.cbf[1] && cur_cu->com_cu_info.cbf[2])
    {
        bits += 1;
    }
    else
    {
        bits += (cur_cu->com_cu_info.cbf[1] && cur_cu->com_cu_info.cbf[2]) ? 3 : 4;
    }
    if (cur_cu->com_cu_info.cbf[0] || cur_cu->com_cu_info.cbf[1] || cur_cu->com_cu_info.cbf[2])
    {
        bits += HLMC_ECD_PutSeBits(HLM_NULL, cur_cu->com_cu_info.qp[0] - cur_cu->com_cu_info.last_code_qp, 0, HLM_NULL);
        if (yuv_comp > 1 && (cur_cu->com_cu_info.cbf[1] || cur_cu->com_cu_info.cbf[2]))
        {
            bits += HLMC_ECD_PutSeBits(HLM_NULL, cur_cu->com_cu_info.qp[1] - cur_cu->com_cu_info.qp[0], 0, HLM_NULL);
        }
        bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 0);
        if (regs->image_format != HLM_IMG_YUV_400)
        {
            bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 1);
            bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 2);
        }
    }

    *me_rdcost = distortion + ((lambda * bits) >> HLMC_LAMBDA_SHIFT);
}

// 计算16x8Skip模式下的SkipRdcost
HLM_VOID HLMC_INTER_skip_rdcost(HLMC_CU_INFO      *cur_cu,
                                HLMC_REGS         *regs,
                                HLM_U32           *skip_rdcost)
{
    HLM_S32 i              = 0;
    HLM_U32 bits           = 1;  // only for skip
    HLM_U16 *cur_cu_src[3] = { HLM_NULL };
    HLM_U16 *rec_tmp       = HLM_NULL;
    HLM_U32 src_stride[3]  = {0};
    HLM_U32 distortion     = 0;
    HLM_U32 channel_size   = 16;
    HLM_U32 lambda         = regs->enc_sse_lamda[cur_cu->com_cu_info.qp[0]];
    HLM_S32 yuv_comp       = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;

    for (i = 0; i < yuv_comp; i++)
    {
        if (i == HLM_LUMA_Y)
        {
            cur_cu_src[i] = regs->enc_input_y_base;
            src_stride[i] = regs->enc_input_luma_stride;
        }
        else if (i == HLM_CHROMA_U)
        {
            cur_cu_src[i] = regs->enc_input_cb_base;
            src_stride[i] = regs->enc_input_chroma_stride;
        }
        else
        {
            cur_cu_src[i] = regs->enc_input_cr_base;
            src_stride[i] = regs->enc_input_chroma_stride;
        }
        cur_cu_src[i] += (cur_cu->com_cu_info.cu_y * src_stride[i] * HLM_HEIGHT_SIZE + cur_cu->com_cu_info.cu_x * HLM_WIDTH_SIZE) ;
        rec_tmp        = cur_cu->com_cu_info.cu_pred_info.skip_pred[i];
        distortion    += HLMC_COM_ComputeSse(cur_cu_src[i], rec_tmp, channel_size, channel_size>>1, src_stride[i], channel_size);
    }

    *skip_rdcost = distortion + ((lambda * bits) >> HLMC_LAMBDA_SHIFT);
}

// 通过PK选出最优的模式并对相应信息进行存储输出
HLM_VOID HLMC_INTER_rdo_pk(HLMC_CU_INFO           *cur_cu,
                           HLMC_CU_INFO           *best_cu,
                           HLMC_REGS              *regs,
                           HLM_NEIGHBOR_INFO      *nbi_info,
                           HLM_U32                 me_rdcost,
                           HLM_U32                 skip_rdcost)
{
    HLM_S32 i                  = 0;
    HLM_U32 j                  = 0;
    HLM_U32 best_rdcost        = 0;
    HLM_CU_PRED_INFO *cur_chan = &cur_cu->com_cu_info.cu_pred_info;
    HLM_U16 *cur_cu_rec        = HLM_NULL;
    HLM_U16 **rec_tmp          = HLM_NULL;
    HLM_U32 rec_stride         = 0;
    HLM_U32 channel_size       = 16;

    if (me_rdcost <= skip_rdcost)
    {
        cur_cu->com_cu_info.cu_type = cur_chan->part_type;
        best_rdcost = me_rdcost;
    }
    else
    {
        cur_cu->com_cu_info.cu_type = HLM_P_SKIP;
        best_rdcost = skip_rdcost;
    }
    if (best_rdcost < best_cu->intra_rd_cost)
    {
        HLMC_COM_CopyCuInfo(cur_cu, best_cu, regs->image_format == HLM_IMG_YUV_400 ? 1 : 3);
    }
}

/***************************************************************************************************
* 功  能：帧间RDO模块
* 参  数：*
*        channel_index        -I    当前通道的索引
*        channel_size         -I    当前通道的大小
*        regs                 -I    当前数据硬件寄存器
*        nbi_info             -I    相邻块信息
*        best_cu              -O    最优cu信息
*        cur_cu               -I    当前cu信息
* 返回值：无
***************************************************************************************************/
HLM_VOID HLMC_INTER_RDO(HLM_U32                 channel_index,
                        HLM_U32                 channel_size,
                        HLMC_REGS              *regs,
                        HLM_NEIGHBOR_INFO      *nbi_info,
                        HLMC_CU_INFO           *best_cu,
                        HLMC_CU_INFO           *cur_cu)
{
    HLM_U32 me_rdcost   = 0;
    HLM_U32 skip_rdcost = 0;

    HLMC_INTER_me_rdcost(cur_cu, nbi_info, regs, &me_rdcost);

    HLMC_INTER_skip_rdcost(cur_cu, regs, &skip_rdcost);

    HLMC_INTER_rdo_pk(cur_cu, best_cu, regs, nbi_info, me_rdcost, skip_rdcost);
}
