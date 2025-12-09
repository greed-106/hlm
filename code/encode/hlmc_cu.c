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
#include "hlmc_cu.h"
#include "hlmc_common.h"
#include "hlmc_intra.h"
#include "hlmc_inter.h"
#include "hlmc_rc.h"
#include "hlmc_ecd.h"
#include "hlmc_tq.h"

// 计算CU级qp
HLM_VOID HLMC_CU_cal_qp(HLMC_QPG       *rc_qpg,
                        HLMC_REGS      *regs,
                        HLM_S32        *cu_bits,
                        HLM_S32         patch_type,
                        HLM_U32         cu_x,
                        HLM_U32         cu_y,
                        HLM_U08         mix_flag,
                        HLM_U32         cu_best_intra_satd)
{
    HLM_S32 i              = ((regs->bitdepth - 8) >> 1);  // 8bit:0,  10bit:1,  12bit 2,  14bit 3,  16bit 4
    HLM_S32 remain_cu_nums = (rc_qpg->cu_rows - 1 - cu_y)* rc_qpg->cu_cols + (rc_qpg->cu_cols - cu_x);
    HLM_S32 remaing_bits   = 0;
    HLM_S32 shift          = 7;
    HLM_S32 offset         = 64;
    HLM_S32 qp             = 0;
    HLM_S32 target_bits    = rc_qpg->cur_cu_target_bits;
    HLM_S32 fullness       = 0;
    HLM_S32 tmp_fullness   = 0;
    HLM_F32 info_ratio     = 0.0;
    HLM_S32 qp_min         = 0;
    HLM_S32 qp_max         = 0;
#if QP_RANGE_ADJUST
    HLM_S32 max_qp         = regs->enc_qp_max + 4;
#else
    HLM_S32 max_qp         = regs->enc_qp_max;
#endif
    HLM_S32 max_buffer     = 1 << log2_rc_buffer_size[i];
    HLM_S32 jn_qp          = 0;
    HLM_S32 flat_cu_thres  = 32;

    rc_qpg->total_bits = rc_qpg->pic_target_bits;
    remaing_bits       = rc_qpg->total_bits - rc_qpg->total_bits_encoded;
    target_bits = remaing_bits / remain_cu_nums;
    info_ratio = (HLM_F32)HLM_CLIP((HLM_F32)target_bits / rc_qpg->avg_B, 0.04, 0.7);

    if (cu_x == 0 || cu_y == 0)
    {
        rc_qpg->complex_level         = HLM_MIN(rc_qpg->complex_level,         2);
        rc_qpg->complex_level_comp[0] = HLM_MIN(rc_qpg->complex_level_comp[0], 2);
        rc_qpg->complex_level_comp[1] = HLM_MIN(rc_qpg->complex_level_comp[1], 2);
        rc_qpg->complex_level_comp[2] = HLM_MIN(rc_qpg->complex_level_comp[2], 2);
    }

    //更新当前块比特数
    tmp_fullness = (HLM_S32)HLM_CLIP((rc_qpg->total_diff_bits >> (log2_rc_buffer_size[i] - 7)), 0, 128);
    if (tmp_fullness <= 20)
    {
        fullness = 0;
    }
    else if (tmp_fullness <= 45)
    {
        fullness = 1;
    }
    else if (tmp_fullness <= 76)
    {
        fullness = 2;
    }
    else if (tmp_fullness <= 102)
    {
        fullness = 3;
    }
    else
    {
        fullness = 4;
    }

    rc_qpg->target_bits_cu[rc_qpg->complex_level] = (HLM_S32)(rc_qpg->B_lossless[rc_qpg->complex_level] * info_ratio);
    rc_qpg->target_bits_cu[rc_qpg->complex_level] = HLM_MIN(
        (rc_qpg->target_bits_cu[rc_qpg->complex_level] * scale_target_bits[fullness][rc_qpg->complex_level] + offset) >> shift,
        rc_qpg->B_lossless[rc_qpg->complex_level]);

    qp = ((rc_qpg->B_lossless[rc_qpg->complex_level] - rc_qpg->target_bits_cu[rc_qpg->complex_level]) * 2
        + (1 << (HLM_LOG2_WIDTH_SIZE + HLM_LOG2_HEIGHT_SIZE - 1))) >> (HLM_LOG2_WIDTH_SIZE + HLM_LOG2_HEIGHT_SIZE);
    qp += 4;
    if ((HLM_S32)cu_best_intra_satd < flat_cu_thres * (1 << (regs->bitdepth - 8)) && qp >= (HLM_S32)(16 + 6 * (regs->bitdepth - 8)))
    {
#if ENC_OPT
        qp = HLM_MAX(qp - 6, (HLM_S32)(16 + 6 * (regs->bitdepth - 8)));
#else
        qp = HLM_MAX(qp - 6, 16 * (HLM_S32)(regs->bitdepth - 8));
#endif
    }
    else if (mix_flag != 0)
    {
        qp = qp - 3 * mix_flag;
    }

    // min_qp
    if (rc_qpg->total_diff_bits < 0.75 * max_buffer)
    {
        qp_min = (HLM_S32)((max_qp - 16) * rc_qpg->total_diff_bits / (3 * max_buffer)) + 4;
    }
    else
    {
        qp_min = (HLM_S32)(3 * max_qp * rc_qpg->total_diff_bits / (max_buffer)-2 * max_qp);
    }

    // max_qp
    if (rc_qpg->total_diff_bits < 0.5 * max_buffer)
    {
        qp_max = (HLM_S32)((1.5 * max_qp - 8) * rc_qpg->total_diff_bits / (max_buffer)) + 4;
    }
    else
    {
        qp_max = (HLM_S32)(max_qp * rc_qpg->total_diff_bits / (2 * max_buffer) + 0.5 * max_qp);
    }
    qp = HLM_CLIP(qp, qp_min, qp_max);

    // qp_min需要和jn_qp取max钳位，不同复杂度和比特深度的jn_qp不同。
    jn_qp = 4 + i * 6 + rc_qpg->complex_level * 6;
    qp = HLM_MAX(qp, jn_qp);
#if QP_RANGE_ADJUST
    qp -= 4;  // QP值域调整
#endif
    qp = HLM_CLIP(qp, regs->enc_qp_min, regs->enc_qp_max);

    rc_qpg->cur_cu_qp = qp;
#if WRITE_PARAMETERS
    rc_qpg->fullness_level = tmp_fullness;
#endif
}

// 调整qp之后，重新设置相关qp参数
HLM_VOID HLMC_CU_set_qp(HLM_SPEC       *spec,
                        HLMC_QPG       *rc_qpg)
{
    HLMC_CU_INFO *cur_cu   = &(spec->cur_cu);
    HLM_U08 luma_complex   = rc_qpg->complex_level_comp[0];
    HLM_U08 chroma_complex = (rc_qpg->complex_level_comp[1] + rc_qpg->complex_level_comp[2] + 1) >> 1;
    HLM_S16 bias           = bias_tab[luma_complex][chroma_complex];
    HLM_S32 qp_min         = spec->regs->enc_qp_min;
    HLM_S32 qp_max         = spec->regs->enc_qp_max;

    cur_cu->com_cu_info.qp[0] = HLM_CLIP(rc_qpg->cur_cu_qp - 2 * bias, qp_min, qp_max);
    cur_cu->com_cu_info.qp[1] = HLM_CLIP(rc_qpg->cur_cu_qp + bias,     qp_min, qp_max);
    cur_cu->com_cu_info.qp[2] = HLM_CLIP(rc_qpg->cur_cu_qp + bias,     qp_min, qp_max);
    if (!spec->sps.cu_delta_qp_enable_flag)  // 定qp
    {
#if FIX_3
        cur_cu->com_cu_info.qp[0] = HLM_CLIP(spec->pps.pic_luma_qp, qp_min, qp_max);
        cur_cu->com_cu_info.qp[1] = HLM_CLIP(spec->pps.pic_luma_qp + spec->pps.pic_chroma_delta_qp, qp_min, qp_max);
#else
        cur_cu->com_cu_info.qp[0] = spec->pps.pic_luma_qp;
        cur_cu->com_cu_info.qp[1] = spec->pps.pic_luma_qp + spec->pps.pic_chroma_delta_qp;
#endif
        cur_cu->com_cu_info.qp[2] = cur_cu->com_cu_info.qp[1];
    }

    // 设置量化信息和lambda
    HLMC_TQ_SetQuantInfo(cur_cu, spec->regs);
}

// 计算重建
HLM_VOID HLMC_CU_recon(HLMC_REGS           *regs,
                       HLMC_CU_INFO        *best_cu)
{
    HLM_U32 index              = 0;
    HLM_U08 yuv_idx            = 0;
    HLM_U32 raster_idx         = 0;  // 光栅扫描顺序
    HLM_U32 zscan_idx          = 0;  // Z字扫描顺序
    HLM_U08 pu_x               = 0;
    HLM_U08 pu_y               = 0;
    HLM_U32 pixel_x            = 0;
    HLM_U32 pixel_y            = 0;
    HLM_U08 i                  = 0;
    HLM_U32 stride_yuv[3]      = { 0 };
    HLM_U16 *ori_pixel[3]      = { 0 };
    HLM_U16 *rec_pixel[3]      = { 0 };
    HLM_U16 *dst               = 0;
    HLM_U16 *src               = 0;
    HLM_U16 **rec_tmp          = HLM_NULL;
    HLM_CU_PRED_INFO *cur_chan = &best_cu->com_cu_info.cu_pred_info;
    HLM_S32 yuv_comp           = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_S08 hor_shift[3]       = { 0 };
    HLM_S08 ver_shift[3]       = { 0 };
    HLM_U32 pu_w               = 0;
    HLM_U32 pu_h               = 0;
    HLM_U32 pu_pos             = 0;

    HLM_COM_GetFormatShift(regs->image_format, hor_shift, ver_shift);
    stride_yuv[0] = regs->enc_input_luma_stride;
    stride_yuv[1] = regs->enc_input_chroma_stride;
    stride_yuv[2] = regs->enc_input_chroma_stride;

    if (best_cu->com_cu_info.cu_type == HLM_I_4x4  || best_cu->com_cu_info.cu_type == HLM_IBC_4x4 ||
        best_cu->com_cu_info.cu_type == HLM_I_LINE || best_cu->com_cu_info.cu_type == HLM_I_ROW)
    {
        rec_pixel[0] = regs->enc_recon_y_base;
        rec_pixel[1] = regs->enc_recon_cb_base;
        rec_pixel[2] = regs->enc_recon_cr_base;
        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            for (pu_x = 0; pu_x < 4; pu_x++)
            {
                for (pu_y = 0; pu_y < 2; pu_y++)
                {
                    pu_w    = 4 >> hor_shift[yuv_idx];
                    pu_h    = 4 >> ver_shift[yuv_idx];
                    pu_pos  = (pu_y * pu_h) * HLM_WIDTH_SIZE + (pu_x * pu_w);
                    pixel_x = (best_cu->com_cu_info.cu_x << 4 >> hor_shift[yuv_idx]) + pu_x * pu_w;
                    pixel_y = (best_cu->com_cu_info.cu_y << 3 >> ver_shift[yuv_idx]) + pu_y * pu_h;
                    src     = best_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos;
                    dst     = rec_pixel[yuv_idx] + pixel_y * stride_yuv[yuv_idx] + pixel_x;
                    for (i = 0; i < pu_h; i++)
                    {
                        memcpy(dst, src, pu_w * sizeof(HLM_U16));
                        src += HLM_WIDTH_SIZE;
                        dst += stride_yuv[yuv_idx];
                    }
                }
            }
        }
    }
    else if (best_cu->com_cu_info.cu_type == HLM_I_16x8)
    {
        rec_pixel[0] = regs->enc_recon_y_base;
        rec_pixel[1] = regs->enc_recon_cb_base;
        rec_pixel[2] = regs->enc_recon_cr_base;
        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            rec_pixel[yuv_idx] += stride_yuv[yuv_idx] * (best_cu->com_cu_info.cu_y << best_cu->com_cu_info.cu_height[yuv_idx])
                               + (best_cu->com_cu_info.cu_x << best_cu->com_cu_info.cu_width[yuv_idx]);
            for (i = 0; i < (1 << best_cu->com_cu_info.cu_height[yuv_idx]); i++)
            {
                memcpy(rec_pixel[yuv_idx] + stride_yuv[yuv_idx] * i,
                    best_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + (i << HLM_LOG2_WIDTH_SIZE),
                    (HLM_U32)(1 << best_cu->com_cu_info.cu_width[yuv_idx]) * sizeof(HLM_U16));
            }
        }
    }
    else
    {
        rec_pixel[0] = regs->enc_recon_y_base;
        rec_pixel[1] = regs->enc_recon_cb_base;
        rec_pixel[2] = regs->enc_recon_cr_base;
        if (best_cu->com_cu_info.cu_type == HLM_P_SKIP)
        {
            rec_tmp = cur_chan->skip_pred;
            memcpy(&best_cu->pu_info_enc[0].inter_pu_info.inter_mv, &cur_chan->skip_mvp, sizeof(HLM_MV));
            memset(best_cu->com_cu_info.coeffs_num, 0, 3 * 2 * 4);
        }
        else
        {
            if (best_cu->com_cu_info.cu_type == HLM_P_16x8)
            {
                memcpy(&best_cu->pu_info_enc[0].inter_pu_info.inter_mv, &best_cu->pu_info_enc[0].mv_16x8, sizeof(HLM_MV));
            }
            else
            {
                memcpy(&best_cu->pu_info_enc[0].inter_pu_info.inter_mv, &best_cu->pu_info_enc[0].mv_8x8, sizeof(HLM_MV));
                memcpy(&best_cu->pu_info_enc[1].inter_pu_info.inter_mv, &best_cu->pu_info_enc[1].mv_8x8, sizeof(HLM_MV));
            }
            rec_tmp = cur_chan->rec;
        }
        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            dst = rec_pixel[yuv_idx];
            dst += (best_cu->com_cu_info.cu_y * stride_yuv[yuv_idx] * (1 << best_cu->com_cu_info.cu_height[yuv_idx])
                + (best_cu->com_cu_info.cu_x << best_cu->com_cu_info.cu_width[yuv_idx]));
            for (i = 0; i < (1 << best_cu->com_cu_info.cu_height[yuv_idx]); i++)
            {
                memcpy(dst + stride_yuv[yuv_idx] * i, rec_tmp[yuv_idx] + (i << HLM_LOG2_WIDTH_SIZE),
                    (HLM_U32)(1 << best_cu->com_cu_info.cu_width[yuv_idx]) * sizeof(HLM_U16));
            }
        }
    }
}

/***************************************************************************************************
* 功  能：帧内帧间CU初始化
* 参  数：*
*        spec                  -IO    算法句柄
*        cu_x                  -I     cu x坐标，单位是cu
*        cu_y                  -I     cu y坐标，单位是cu
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_CU_Init(HLM_SPEC       *spec,
                      HLM_U32         cu_x,
                      HLM_U32         cu_y)
{
    HLM_U32 i            = 0;
    HLMC_CU_INFO *cur_cu = &(spec->cur_cu);

    cur_cu->com_cu_info.intra_8x8_enable_flag   = spec->regs->intra_8x8_enable_flag;
    cur_cu->com_cu_info.cu_delta_qp_enable_flag = spec->regs->cu_delta_qp_enable_flag;
    cur_cu->com_cu_info.cu_x                    = cu_x;
    cur_cu->com_cu_info.cu_y                    = cu_y;
    if (spec->patch_ctx.segment_enable_flag)
    {
        cur_cu->com_cu_info.left_unavail = (cu_x == 0) || (cu_x % (1 << (spec->patch_ctx.segment_width_in_log2 - HLM_LOG2_WIDTH_SIZE)) == 0);
        cur_cu->com_cu_info.up_unavail   = (cu_y == 0) || (cu_y % (1 << (spec->patch_ctx.segment_height_in_log2 - HLM_LOG2_HEIGHT_SIZE)) == 0);
    }
    else
    {
        cur_cu->com_cu_info.left_unavail = (cu_x == 0);
        cur_cu->com_cu_info.up_unavail   = (cu_y == 0);
    }

    cur_cu->com_cu_info.cu_width[0]     = HLM_LOG2_WIDTH_SIZE;
    cur_cu->com_cu_info.cu_height[0]    = HLM_LOG2_HEIGHT_SIZE;
    cur_cu->com_cu_info.chroma_offset_x = 0;
    cur_cu->com_cu_info.chroma_offset_y = 0;
    switch (spec->coding_ctrl.img_format)
    {
    case HLM_IMG_YUV_444:
    case HLM_IMG_RGB:
        cur_cu->com_cu_info.cu_width[1]     = HLM_LOG2_WIDTH_SIZE;
        cur_cu->com_cu_info.cu_height[1]    = HLM_LOG2_HEIGHT_SIZE;
        cur_cu->com_cu_info.cu_width[2]     = HLM_LOG2_WIDTH_SIZE;
        cur_cu->com_cu_info.cu_height[2]    = HLM_LOG2_HEIGHT_SIZE;
        cur_cu->com_cu_info.chroma_offset_x = 0;
        cur_cu->com_cu_info.chroma_offset_y = 0;
        break;
    case HLM_IMG_YUV_422:
        cur_cu->com_cu_info.cu_width[1]     = (HLM_LOG2_WIDTH_SIZE - 1);
        cur_cu->com_cu_info.cu_height[1]    = HLM_LOG2_HEIGHT_SIZE;
        cur_cu->com_cu_info.cu_width[2]     = (HLM_LOG2_WIDTH_SIZE - 1);
        cur_cu->com_cu_info.cu_height[2]    = HLM_LOG2_HEIGHT_SIZE;
        cur_cu->com_cu_info.chroma_offset_x = 1;
        break;
    case HLM_IMG_YUV_420:
        cur_cu->com_cu_info.cu_width[1]     = (HLM_LOG2_WIDTH_SIZE - 1);
        cur_cu->com_cu_info.cu_height[1]    = (HLM_LOG2_HEIGHT_SIZE - 1);
        cur_cu->com_cu_info.cu_width[2]     = (HLM_LOG2_WIDTH_SIZE - 1);
        cur_cu->com_cu_info.cu_height[2]    = (HLM_LOG2_HEIGHT_SIZE - 1);
        cur_cu->com_cu_info.chroma_offset_x = 1;
        cur_cu->com_cu_info.chroma_offset_y = 1;
        break;
    }

    cur_cu->mix_flag          = 0;  // 混合复杂度标记
    cur_cu->intra_rd_cost     = 0;
    cur_cu->com_cu_info.qp[0] = spec->regs->enc_pic_qp;
    cur_cu->com_cu_info.qp[1] = cur_cu->com_cu_info.qp[0];
    cur_cu->com_cu_info.qp[2] = cur_cu->com_cu_info.qp[0];
    if (cu_x == 0 && cu_y == 0)
    {
#if FIX_3
        cur_cu->com_cu_info.last_code_qp = spec->regs->enc_pic_qp;
#else
        if (spec->regs->cu_delta_qp_enable_flag)
        {
#if QP_RANGE_ADJUST
            cur_cu->com_cu_info.last_code_qp = 22;  // 每帧图像第一个CU的基准qp
#else
            cur_cu->com_cu_info.last_code_qp = 26;  // 每帧图像第一个CU的基准qp
#endif
        }
        else  // 定qp，用不到last_code_qp
        {
            cur_cu->com_cu_info.last_code_qp = spec->regs->enc_pic_qp;
        }
#endif
    }
    for (i = 0; i < HLM_TU_4x4_NUMS; i++)
    {
        cur_cu->com_cu_info.cu_pred_info.pu_info[i].pu_x = ((i % 4) << 2);
        cur_cu->com_cu_info.cu_pred_info.pu_info[i].pu_y = ((i / 4) << 2);
    }

    // 设置量化信息和lambda
    HLMC_TQ_SetQuantInfo(cur_cu, spec->regs);
}

/***************************************************************************************************
* 功  能：cu编码
* 参  数：*
*        spec            -IO    硬件编码算法句柄
*        cu_truebits     -IO    宏块真实编码比特数
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_CU_Encode(HLM_SPEC            *spec,
                          HLM_U32             *cu_truebits)
{
    HLMC_REGS          *regs          = spec->regs;
    HLM_NEIGHBOR_INFO *nbi_info       = &(spec->nbi_info);
    HLMC_CU_INFO       *cur_cu        = &(spec->cur_cu);
    HLMC_BITSTREAM     *bs            = &(spec->bs);
    HLMC_CU_INFO       *best_cu       = &(spec->best_cu);
    HLM_U32             raster_idx    = 0;
    HLM_U32             bits_begin    = bs->bit_cnt + 8 - bs->bits_left;
    HLM_U32             yuv_comp      = spec->sps.format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_U08             block_pu_num  = regs->intra_8x8_enable_flag ? 2 : 8;
    HLM_U08             block_size    = regs->intra_8x8_enable_flag ? 8 : 4;

    // 模式粗筛
    HLMC_INTRA_MODE(regs, nbi_info, cur_cu);
    if (spec->sps.intra_16x1_2x8_enable_flag)
    {
        HLMC_LINE_BY_LINE_MODE(regs, nbi_info, cur_cu);
    }
    if ((regs->enc_frame_coding_type == HLM_FRAME_TYPE_I && spec->sps.i_frame_enable_ibc) ||
        (regs->enc_frame_coding_type == HLM_FRAME_TYPE_P && spec->sps.p_frame_enable_ibc))
    {
        HLMC_SCC_MODE(regs, nbi_info, cur_cu);
    }
    if (regs->enc_frame_coding_type == HLM_FRAME_TYPE_P)
    {
        HLMC_INTER_MODE(regs, nbi_info, cur_cu);
    }

    // 推导QP
    if (spec->sps.cu_delta_qp_enable_flag)
    {
        HLMC_RC_CalComplexLevel(&(spec->rc_qpg), cur_cu->intra_satd_cost, cur_cu->satd_comp, regs->bitdepth, yuv_comp);
        HLMC_CU_cal_qp(&(spec->rc_qpg), regs, 0, regs->enc_frame_coding_type,
            cur_cu->com_cu_info.cu_x, cur_cu->com_cu_info.cu_y, cur_cu->mix_flag, cur_cu->intra_satd_cost);
    }
    HLMC_CU_set_qp(spec, &(spec->rc_qpg));
    HLMC_COM_CopyCuInfo(cur_cu, best_cu, yuv_comp);

    // RDO决策
    cur_cu->com_cu_info.ts_flag = 0;
    best_cu->intra_rd_cost = HLM_MAX_U32;
    for (raster_idx = 0; raster_idx < block_pu_num; raster_idx++)
    {
        HLMC_INTRA_PRED(raster_idx, block_size, regs, nbi_info, cur_cu);
        HLMC_INTRA_RDO(raster_idx, block_size, regs, nbi_info, best_cu, cur_cu);
    }
    HLMC_INTRA_PRED(0, 16, regs, nbi_info, cur_cu);
    HLMC_INTRA_RDO(0, 16, regs, nbi_info, best_cu, cur_cu);

    if (spec->sps.intra_16x1_2x8_enable_flag)
    {
        HLMC_LINE_BY_LINE_RDO(regs, nbi_info, best_cu, cur_cu);
    }

    cur_cu->com_cu_info.ts_flag = 0;
    if (((regs->enc_frame_coding_type == HLM_FRAME_TYPE_I && spec->sps.i_frame_enable_ibc) ||
        (regs->enc_frame_coding_type == HLM_FRAME_TYPE_P && spec->sps.p_frame_enable_ibc)))
    {
        HLMC_SCC_RDO(regs, nbi_info, best_cu, cur_cu);
    }

    if (regs->enc_frame_coding_type == HLM_FRAME_TYPE_P)
    {
        cur_cu->com_cu_info.ts_flag = 0;
        HLMC_INTER_RDO(0, 16, regs, nbi_info, best_cu, cur_cu);
    }

    // 重建
    HLMC_CU_recon(regs, best_cu);
    HLMC_COM_CopyCuInfo(best_cu, cur_cu, yuv_comp);

    // 写码流
    HLMC_ECD_CU(cur_cu, nbi_info, regs->enc_frame_coding_type, yuv_comp, bs, regs->segment_enable_flag, regs->segment_width_in_log2,
        regs->intra_16x1_2x8_enable_flag,
        regs->enc_frame_coding_type == HLM_FRAME_TYPE_I ? spec->sps.i_frame_enable_ibc : spec->sps.p_frame_enable_ibc, regs->sub_ibc_enable_flag);

    // 更新邻域信息
    HLM_NBI_UpdateRec(&cur_cu->com_cu_info, nbi_info,
        regs->enc_recon_y_base, regs->enc_recon_cb_base, regs->enc_recon_cr_base,
        regs->enc_input_luma_stride, regs->enc_input_chroma_stride);
    HLM_NBI_Process(&cur_cu->com_cu_info, nbi_info, regs->enc_frame_coding_type,
        &cur_cu->pu_info_enc[0].inter_pu_info,
        &cur_cu->pu_info_enc[1].inter_pu_info);

    // 更新码控信息
    *cu_truebits = bs->bit_cnt + 8 - bs->bits_left - bits_begin;
    HLMC_RC_UpdateQpg(&spec->rc_qpg, cur_cu->mix_flag, *cu_truebits, cur_cu->com_cu_info.qp[0], cur_cu->com_cu_info.qp[1]);
    if ((HLM_FRAME_TYPE_I == regs->enc_frame_coding_type))
    {
        spec->rc_qpg.i_sum_row_qp += cur_cu->com_cu_info.qp[0];
    }
    else if ((HLM_FRAME_TYPE_P == regs->enc_frame_coding_type))
    {
        spec->rc_qpg.p_sum_row_qp += cur_cu->com_cu_info.qp[0];
    }

    return HLM_STS_OK;
}
