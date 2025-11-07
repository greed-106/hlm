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
#include "hlmc_patch.h"
#include "hlmc_ecd.h"
#include "hlmc_rc.h"
#include "hlmc_cu.h"

// 编码patch header
HLM_VOID HLMC_PATCH_encode_header(HLM_PATCH_HEADER  *patch_ctx,
                                  HLMC_BITSTREAM    *bs)
{
    HLM_PARAM_SPS *sps                       = &patch_ctx->sps;
    HLM_PARAM_PPS *pps                       = &patch_ctx->pps;
    HLM_S32 patch_log2_segment_width_minus4  = 0;
    HLM_S32 patch_log2_segment_height_minus3 = 0;
    HLM_S32 patch_ibc_enable_flag            = 0;
    HLM_S32 patch_x                          = 0;
    HLM_S32 patch_y                          = 0;
    HLM_S32 patch_width                      = 0;
    HLM_S32 patch_height                     = 0;

    patch_log2_segment_width_minus4  = patch_ctx->segment_width_in_log2 - HLM_LOG2_WIDTH_SIZE;
    patch_log2_segment_height_minus3 = patch_ctx->segment_height_in_log2 - HLM_LOG2_HEIGHT_SIZE;
    assert(0 <= patch_log2_segment_width_minus4  && patch_log2_segment_width_minus4  <= 255);
    assert(0 <= patch_log2_segment_height_minus3 && patch_log2_segment_height_minus3 <= 255);
    if (pps->pic_type == HLM_FRAME_TYPE_I)
    {
        patch_ibc_enable_flag = sps->i_frame_enable_ibc;
    }
    else
    {
        patch_ibc_enable_flag = sps->p_frame_enable_ibc;
    }
    patch_x      = sps->patch_info.patch_param[patch_ctx->patch_idx].patch_x;
    patch_y      = sps->patch_info.patch_param[patch_ctx->patch_idx].patch_y;
    patch_width  = sps->patch_info.patch_param[patch_ctx->patch_idx].patch_width[0];
    patch_height = sps->patch_info.patch_param[patch_ctx->patch_idx].patch_height[0];

#if !START_CODE_FIX
    HLMC_ECD_PutStartCode4Byte(bs);
#endif
    if (pps->pic_type == HLM_FRAME_TYPE_I)
    {
#if START_CODE_FIX
        HLMC_ECD_PutStartCode4Byte(bs);
#endif
        HLMC_ECD_PutNaluHeader(HLM_IDR_NUT, 1, bs);
    }
    else
    {
#if START_CODE_FIX
        HLMC_ECD_PutStartCode3Byte(bs);
#endif
        HLMC_ECD_PutNaluHeader(HLM_NONIDR_NUT, 1, bs);
    }

    HLMC_ECD_PutUeBits(   bs, patch_ctx->patch_idx,                       1, "patch_id");
    HLMC_ECD_PutLongBits( bs, patch_ctx->patch_bpp,                      12, "patch_bpp");
    HLMC_ECD_PutShortBits(bs, patch_ctx->segment_enable_flag,             1, "patch_segment_enable_flag");
    HLMC_ECD_PutShortBits(bs, patch_log2_segment_width_minus4,            8, "patch_log2_segment_width_minus4");
    HLMC_ECD_PutShortBits(bs, patch_log2_segment_height_minus3,           8, "patch_log2_segment_height_minus3");
    HLMC_ECD_PutShortBits(bs, patch_ctx->patch_extra_params_present_flag, 1, "patch_extra_params_present_flag");
    if (patch_ctx->patch_extra_params_present_flag)
    {
        HLMC_ECD_PutShortBits(bs, sps->bitdepth - 8,            4, "patch_bit_depth_luma_minus8");
        HLMC_ECD_PutShortBits(bs, sps->bitdepth - 8,            4, "patch_bit_depth_chroma_minus8");
        HLMC_ECD_PutShortBits(bs, sps->format,                  3, "patch_chroma_format");
        HLMC_ECD_PutShortBits(bs, patch_ibc_enable_flag,        1, "patch_ibc_enable_flag");
        HLMC_ECD_PutShortBits(bs, sps->intra_8x8_enable_flag,   1, "patch_intra_8x8_enable_flag");
        HLMC_ECD_PutShortBits(bs, sps->cu_delta_qp_enable_flag, 1, "patch_cu_delta_qp_enable_flag");
        HLMC_ECD_PutShortBits(bs, sps->mv_limit_enable_flag,    1, "patch_mv_search_range_limit_enable_flag");
        if (sps->mv_limit_enable_flag)
        {
            HLMC_ECD_PutShortBits(bs, sps->mv_search_width,  8, "patch_mv_search_range_width");
            HLMC_ECD_PutShortBits(bs, sps->mv_search_height, 8, "patch_mv_search_range_height");
        }
        HLMC_ECD_PutLongBits( bs, patch_x,      32, "patch_x");
        HLMC_ECD_PutLongBits( bs, patch_y,      32, "patch_y");
        HLMC_ECD_PutLongBits( bs, patch_width,  32, "patch_width");
        HLMC_ECD_PutLongBits( bs, patch_height, 32, "patch_height");
        HLMC_ECD_PutShortBits(bs, pps->pic_type, 1, "patch_type");
        HLMC_ECD_PutLongBits( bs, pps->poc,     16, "patch_poc");
        if (sps->cu_delta_qp_enable_flag)
        {
            HLMC_ECD_PutUeBits(bs, pps->pic_luma_qp,         1, "patch_luma_qp");
            HLMC_ECD_PutSeBits(bs, pps->pic_chroma_delta_qp, 1, "patch_chroma_delta_qp");
        }
    }
}

// 编码patch data
HLM_STATUS HLMC_PATCH_encode_data(HLM_SPEC *spec)
{
    HLM_STATUS sts              = HLM_STS_ERR;
    HLMC_BITSTREAM *bs          = &(spec->bs);
    HLMC_QPG *rc_qpg            = &(spec->rc_qpg);
    HLMC_REGS *regs             = spec->regs;
    HLMC_RC_SPEC *rc_spec       = (HLMC_RC_SPEC*)spec->rc_handle;
    HLM_PATCH_HEADER *patch_ctx = &(spec->patch_ctx);
    HLM_U32 patch_idx           = patch_ctx->patch_idx;
    HLM_U32 patch_width         = patch_ctx->sps.patch_info.patch_param[patch_idx].patch_width[0];
    HLM_U32 patch_height        = patch_ctx->sps.patch_info.patch_param[patch_idx].patch_height[0];
    HLM_U32 cu_truebits         = 0;
    HLM_U32 cu_cols             = HLM_SIZE_ALIGN_16(patch_width) >> HLM_LOG2_WIDTH_SIZE;
    HLM_U32 cu_rows             = HLM_SIZE_ALIGN_8(patch_height) >> HLM_LOG2_HEIGHT_SIZE;
    HLM_U32 cu_x                = 0;
    HLM_U32 cu_y                = 0;
    HLM_U32 cu_x_in_segment     = 0;
    HLM_U32 cu_y_in_segment     = 0;

    //初始化QPG
    if (regs->enc_frame_coding_type == HLM_FRAME_TYPE_I)
    {
        rc_qpg->bpp = rc_spec->rate_ctrl.rc_cbr_ctrl.bpp_i;
    }
    else
    {
        rc_qpg->bpp = rc_spec->rate_ctrl.rc_cbr_ctrl.bpp_p;
    }
    rc_qpg->patch_target_bits = (rc_qpg->bpp * patch_width * patch_height + 8) >> 4;  // patch的目标比特数
    rc_qpg->rc_buffer_size_log2 = rc_spec->rate_ctrl.rc_cbr_ctrl.rc_buffer_size_log2;
    HLMC_RC_InitQpg(rc_qpg, regs, cu_cols, cu_rows);

    // 处理所有宏块
    for (cu_y = 0; cu_y < cu_rows; cu_y++)
    {
        for (cu_x = 0; cu_x < cu_cols; cu_x++)
        {
            //cu初始化，计算宏块qp
            HLMC_CU_Init(spec, cu_x, cu_y);

            // 编码一个宏块
            sts = HLMC_CU_Encode(spec, &cu_truebits);
            HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

            // 检查码流缓冲区是否溢出
            if (bs->bit_cnt >= bs->bit_size)
            {
                printf("Bitstream buffer space is not enough!\n");
                return HLM_STS_ERR;
            }

#if WRITE_PARAMETERS
            fprintf(spec->fp_param, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %lld\n",
                spec->patch_ctx.patch_idx, spec->cur_cu.com_cu_info.cu_y, spec->cur_cu.com_cu_info.cu_x,
                spec->cur_cu.com_cu_info.cu_type, spec->cur_cu.intra_satd_cost,
                rc_qpg->complex_level, rc_qpg->complex_level_comp[0],
                (rc_qpg->complex_level_comp[1] + rc_qpg->complex_level_comp[2] + 1) >> 1,
                cu_truebits, rc_qpg->fullness_level,
                spec->cur_cu.com_cu_info.qp[0], spec->cur_cu.com_cu_info.qp[1], spec->cur_cu.com_cu_info.qp[2],
                spec->rc_qpg.total_bits_encoded, spec->rc_qpg.total_diff_bits);
#endif
        }
    }

    //检测每个patch码流是否超过目标比特数
    if (rc_spec->rate_ctrl.rate_ctrl_mode == HLMC_RC_CBR &&
        spec->rc_qpg.total_bits_encoded > spec->rc_qpg.patch_target_bits)
    {
        printf("patch_%d real bitstream %d overflow patch target bits %d !\n",
            spec->patch_ctx.patch_idx, spec->rc_qpg.total_bits_encoded, spec->rc_qpg.patch_target_bits);
    }

    // 更新qp信息
    regs->enc_i_ave_row_qp += rc_qpg->i_sum_row_qp;
    regs->enc_p_ave_row_qp += rc_qpg->p_sum_row_qp;
    spec->cu_cnt += cu_rows * cu_cols;

    HLMC_ECD_RbspTrailingBits(bs);

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：初始化编码所需要的patch信息
* 参  数：* 
*         spec           -I         硬件抽象层工作参数
*         patch_ctx      -O         patch信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_PATCH_Init(HLM_SPEC          *spec,
                         HLM_PATCH_HEADER  *patch_ctx)
{
    HLMC_REGS      *regs         = spec->regs;
    HLM_S32         i            = 0;
    HLMC_RATE_CTRL *rate_ctrl    = &((HLMC_RC_SPEC *)spec->rc_handle)->rate_ctrl;

    // 确保patch级重传的语法和高层一致
    memcpy(&patch_ctx->sps, &spec->sps, sizeof(HLM_PARAM_SPS));
    memcpy(&patch_ctx->pps, &spec->pps, sizeof(HLM_PARAM_PPS));

    if (spec->pps.pic_type == HLM_FRAME_TYPE_P)
    {
        patch_ctx->patch_bpp = rate_ctrl->rc_cbr_ctrl.bpp_p;
    }
    else
    {
        patch_ctx->patch_bpp = rate_ctrl->rc_cbr_ctrl.bpp_i;
    }
    patch_ctx->segment_enable_flag             = spec->coding_ctrl.segment_enable_flag;
    patch_ctx->segment_width_in_log2           = spec->coding_ctrl.segment_width_in_log2;
    patch_ctx->segment_height_in_log2          = spec->coding_ctrl.segment_height_in_log2;
    patch_ctx->patch_extra_params_present_flag = 0;
}

/***************************************************************************************************
* 功  能：编码一个patch
* 参  数：* 
*         spec           -IO        硬件抽象层工作参数
*         patch_ctx      -I         patch信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_PATCH_Encode(HLM_SPEC          *spec,
                             HLM_PATCH_HEADER  *patch_ctx)
{
    HLM_STATUS      sts    = HLM_STS_ERR;
    HLMC_BITSTREAM *bs     = &(spec->bs);
    HLMC_BITSTREAM *out_bs = &(spec->out_bs);

    HLMC_PATCH_encode_header(patch_ctx, bs);

    sts = HLMC_PATCH_encode_data(spec);
    HLM_CHECK_ERROR(sts != HLM_STS_OK,sts);
    HLMC_ECD_WriteEmulaPreventBytes(bs, out_bs);

    spec->total_bs_len += (bs->bit_cnt >> 3);

    // 重置码流信息结构体，为下一个patch编码做准备
    bs->bit_size -= (bs->bit_cnt >> 3);
    bs->bit_cnt   = 0;
    bs->ptr_start = bs->ptr;  // 码流首地址指向下一个patch

    return HLM_STS_OK;
}

/****************************************************************************************
* 功  能：对参考帧进行padding,用于运动估计
* 参  数：
*         spec           -IO        硬件抽象层工作参数
*         patch_ctx      -I         patch信息
* 返回值：无
* 备  注：
****************************************************************************************/
HLM_VOID HLMC_PATCH_Padding(HLM_SPEC         *spec,
                            HLM_PATCH_HEADER *patch_ctx)
{
    HLMC_REGS *regs                  = spec->regs;
    HLM_S32 i                        = 0;
    HLM_S32 j                        = 0;
    HLM_S32 patch_idx                = patch_ctx->patch_idx;
    HLM_PATCH_PARAM patch_param      = patch_ctx->sps.patch_info.patch_param[patch_idx];
    HLM_S32 patch_x[3]               = { patch_param.patch_x,patch_param.patch_x,patch_param.patch_x };
    HLM_S32 patch_y[3]               = { patch_param.patch_y,patch_param.patch_y,patch_param.patch_y };
    HLM_S32 patch_width[2]           = { patch_param.patch_width[0],patch_param.patch_width[1] };
    HLM_S32 patch_height[2]          = { patch_param.patch_height[0],patch_param.patch_height[1] };
    HLM_S32 patch_coded_width[2]     = { patch_param.patch_coded_width[0], patch_param.patch_coded_width[1] };
    HLM_S32 patch_coded_height[2]    = { patch_param.patch_coded_height[0],patch_param.patch_coded_height[1] };
    HLM_U32 src_stride               = 0;
    HLM_S32 pos_x[4]                 = { 0 };
    HLM_S32 pos_y[4]                 = { 0 };
    HLM_S32 pad_left_w               = regs->inter_pad_w_left;
    HLM_S32 pad_right_w              = regs->inter_pad_w_right;
    HLM_S32 pad_up_h                 = regs->inter_pad_h_up;
    HLM_S32 pad_down_h               = regs->inter_pad_h_down;
    HLM_S32 pad_stride_w             = 0;
    HLM_S32 pad_stride_h             = 0;
    HLM_U16 *input_base              = HLM_NULL;
    HLM_U16 *pad_base                = HLM_NULL;
    HLM_U16 *recon_base              = HLM_NULL;
    HLM_U16 *pad_left                = HLM_NULL;
    HLM_U16 *pad_src                 = HLM_NULL;
    HLM_U16 *pad_right               = HLM_NULL;
    HLM_U16 *recon_left              = HLM_NULL;
    HLM_U16 *recon_right             = HLM_NULL;
    HLM_S32 k                        = 0;
    HLM_U16 *pad_dst                 = HLM_NULL;
    HLM_S32 yuv_comp                 = spec->sps.format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_S32 real_patch_width         = HLM_MIN(patch_width[0], (HLM_S32)regs->enc_real_pic_width[0] - patch_x[0]);
    HLM_S32 real_patch_height        = HLM_MIN(patch_height[0], (HLM_S32)regs->enc_real_pic_height[0] - patch_y[0]);
    HLM_S32 real_patch_width_yuv[3]  = { real_patch_width ,real_patch_width ,real_patch_width };
    HLM_S32 real_patch_height_yuv[3] = { real_patch_height ,real_patch_height ,real_patch_height };

    // 将当前patch信息存入hw_regs，用于底层判断可得性
    regs->cur_patch_param         = &patch_ctx->sps.patch_info.patch_param[patch_idx];
    regs->enc_pic_width[0]        = patch_coded_width[0];
    regs->enc_pic_height[0]       = patch_coded_height[0];
    regs->enc_pic_width [1]       = patch_coded_width[1];
    regs->enc_pic_height[1]       = patch_coded_height[1];
    regs->enc_input_luma_stride   = patch_coded_width[0];
    regs->enc_input_chroma_stride = patch_coded_width[1];
    pad_right_w                  += patch_coded_width[0] - real_patch_width;
    pad_down_h                   += patch_coded_height[0] - real_patch_height;
    switch (spec->sps.format)
    {
    case HLM_IMG_YUV_420:
        real_patch_width_yuv[1]  = real_patch_width_yuv[0] >> 1;
        real_patch_height_yuv[1] = real_patch_height_yuv[0] >> 1;
        patch_x[1]               = patch_x[0] >> 1;
        patch_y[1]               = patch_y[0] >> 1;
        break;
    case HLM_IMG_YUV_422:
        real_patch_width_yuv[1]  = real_patch_width_yuv[0] >> 1;
        real_patch_height_yuv[1] = real_patch_height_yuv[0];
        patch_x[1]               = patch_x[0] >> 1;
        patch_y[1]               = patch_y[0];
        break;
    }
    patch_x[2] = patch_x[1];
    patch_y[2] = patch_y[1];
    real_patch_width_yuv[2] = real_patch_width_yuv[1];
    real_patch_height_yuv[2] = real_patch_height_yuv[1];

    //原始图像patch padding
    for (i = HLM_LUMA_Y; i < yuv_comp; i++)
    {
        if (i == HLM_LUMA_Y)
        {
            src_stride = regs->enc_real_input_luma_stride;
            pad_base   = regs->enc_input_y_base;
            input_base = regs->enc_real_input_y_base + patch_y[i] * src_stride + patch_x[i];
        }
        else if (i == HLM_CHROMA_U)
        {
            src_stride = regs->enc_real_input_chroma_stride;
            pad_base   = regs->enc_input_cb_base;
            input_base = regs->enc_real_input_cb_base + patch_y[i] * src_stride + patch_x[i];
        }
        else
        {
            src_stride = regs->enc_real_input_chroma_stride;
            pad_base   = regs->enc_input_cr_base;
            input_base = regs->enc_real_input_cr_base + patch_y[i] * src_stride + patch_x[i];
        }

        /*to padding the right*/
        pad_src = input_base;
        pad_dst = pad_base;
        for (j = 0; j < real_patch_height_yuv[i]; j++)
        {
            memcpy(pad_dst, pad_src, real_patch_width_yuv[i] * sizeof(HLM_U16));
            for (k = real_patch_width_yuv[i]; k < patch_coded_width[i!=0]; k++)
            {
                pad_dst[k] = pad_src[real_patch_width_yuv[i] - 1];
            }
            pad_src += src_stride;
            pad_dst += patch_coded_width[i != 0];
        }

        /*to padding the down*/
        for (j = real_patch_height_yuv[i]; j < patch_coded_height[i != 0]; j++)
        {
            pad_dst = pad_base + j * patch_coded_width[i != 0];
            memcpy(pad_dst, pad_dst - patch_coded_width[i != 0], patch_coded_width[i != 0] * sizeof(HLM_U16));
        }
    }

    //参考帧 patch padding
    if (regs->mv_ref_cross_patch)
    {
        /*pad-patch的理论范围：Area0 = [x0,x1) * [y0,y1)
        x0 = patch_x - inter_pad_w_left
        x1 = patch_x + patch_coded_width + inter_pad_w_right
        y0 = patch_y - inter_pad_h_up
        y1 = patch_y + patch_coded_height + inter_pad_h_down
        参考帧中有效数据的范围：Area1 = [0,pic_width) * [0,pic_height)
        二者取交集，得到pad-patch中可以直接从参考帧里拿到的数据范围：Area2 = [x2,x3) * [y2,y3)
        Area0 - Area2就是需要padding的区域，可以算出上、下、左、右需要padding的数据长度*/
        pos_x[0]                 = patch_x[0] - regs->inter_pad_w_left;
        pos_y[0]                 = patch_y[0] - regs->inter_pad_h_up;
        pos_x[1]                 = patch_x[0] + patch_coded_width[0] + regs->inter_pad_w_right;
        pos_y[1]                 = patch_y[0] + patch_coded_height[0] + regs->inter_pad_h_down;
        pos_x[2]                 = HLM_MAX(0, pos_x[0]);
        pos_y[2]                 = HLM_MAX(0, pos_y[0]);
        pos_x[3]                 = HLM_MIN((HLM_S32)regs->enc_real_pic_width[0], pos_x[1]);
        pos_y[3]                 = HLM_MIN((HLM_S32)regs->enc_real_pic_height[0], pos_y[1]);
        pad_up_h                 = pos_y[2] - pos_y[0];
        pad_down_h               = pos_y[1] - pos_y[3];
        pad_left_w               = pos_x[2] - pos_x[0];
        pad_right_w              = pos_x[1] - pos_x[3];
        real_patch_width_yuv[0]  = pos_x[3] - pos_x[2];
        real_patch_height_yuv[0] = pos_y[3] - pos_y[2];
        patch_x[0]               = pos_x[0] + pad_left_w;
        patch_y[0]               = pos_y[0] + pad_up_h;

        switch (spec->sps.format)
        {
        case HLM_IMG_YUV_420:
            real_patch_width_yuv[1]  = real_patch_width_yuv[0] >> 1;
            real_patch_height_yuv[1] = real_patch_height_yuv[0] >> 1;
            patch_x[1]               = patch_x[0] >> 1;
            patch_y[1]               = patch_y[0] >> 1;
            break;
        case HLM_IMG_YUV_422:
            real_patch_width_yuv[1]  = real_patch_width_yuv[0] >> 1;
            real_patch_height_yuv[1] = real_patch_height_yuv[0];
            patch_x[1]               = patch_x[0] >> 1;
            patch_y[1]               = patch_y[0];
            break;
        default:  // YUV444, RGB444, YUV400
            real_patch_width_yuv[1]  = real_patch_width_yuv[0];
            real_patch_height_yuv[1] = real_patch_height_yuv[0];
            patch_x[1]               = patch_x[0];
            patch_y[1]               = patch_y[0];
            break;
        }
        real_patch_width_yuv[2] = real_patch_width_yuv[1];
        real_patch_height_yuv[2] = real_patch_height_yuv[1];
        patch_x[2] = patch_x[1];
        patch_y[2] = patch_y[1];
    }

    for (i = HLM_LUMA_Y; i < yuv_comp; i++)
    {
        pad_stride_w = regs->inter_pad_w_left + regs->inter_pad_w_right + patch_coded_width[i != HLM_LUMA_Y];
        pad_stride_h = regs->inter_pad_h_up + regs->inter_pad_h_down + patch_coded_height[i != HLM_LUMA_Y];
        if (i == HLM_LUMA_Y)
        {
            src_stride = regs->enc_real_input_luma_stride;
            pad_base   = regs->enc_ref_y_padding_base;
            recon_base = regs->enc_ref_y_base + patch_y[i] * src_stride + patch_x[i];
            regs->enc_ref_frame_luma_stride = pad_stride_w;
        }
        else if (i == HLM_CHROMA_U)
        {
            src_stride = regs->enc_real_input_chroma_stride;
            pad_base   = regs->enc_ref_cb_padding_base;
            recon_base = regs->enc_ref_cb_base + patch_y[i] * src_stride + patch_x[i];
            regs->enc_ref_frame_chroma_stride = pad_stride_w;
        }
        else
        {
            src_stride = regs->enc_real_input_chroma_stride;
            pad_base   = regs->enc_ref_cr_padding_base;
            recon_base = regs->enc_ref_cr_base + patch_y[i] * src_stride + patch_x[i];
        }

        /*to padding the left and right first*/
        for (j = pad_up_h; j < (pad_stride_h - pad_down_h); j++)
        {
            pad_left    = pad_base + j * pad_stride_w;
            pad_src     = pad_left + pad_left_w;
            pad_right   = pad_src + real_patch_width_yuv[i];
            recon_left  = recon_base + (j - pad_up_h) * src_stride;
            recon_right = recon_left + real_patch_width_yuv[i] - 1;
            memcpy(pad_src, recon_left, real_patch_width_yuv[i] * sizeof(HLM_U16));
            for (k = 0; k < pad_left_w; k++)
            {
                pad_left[k] = recon_left[0];
            }
            for (k = 0; k < pad_right_w; k++)
            {
                pad_right[k] = recon_right[0];
            }
        }

        /*to padding the up*/
        for (j = (pad_up_h - 1); j >= 0; j--)
        {
            pad_left = pad_base + j * pad_stride_w;
            memcpy(pad_left, pad_left + pad_stride_w, pad_stride_w * sizeof(HLM_U16));
        }

        /*to padding the down*/
        for (j = (pad_stride_h - pad_down_h); j < pad_stride_h; j++)
        {
            pad_left = pad_base + j * pad_stride_w;
            memcpy(pad_left, pad_left - pad_stride_w, pad_stride_w * sizeof(HLM_U16));
        }
    }
}

/****************************************************************************************
* 功  能：对参考帧进行padding,用于运动估计
* 参  数：
*         spec           -IO        硬件抽象层工作参数
*         patch_ctx      -I         patch信息
* 返回值：无
* 备  注：
****************************************************************************************/
HLM_VOID HLMC_PATCH_WriteRec(HLM_SPEC         *spec,
                             HLM_PATCH_HEADER *patch_ctx)
{
    HLMC_REGS *regs                  = spec->regs;
    HLM_S32 i                        = 0;
    HLM_S32 j                        = 0;
    HLM_S32 k                        = 0;
    HLM_S32 patch_idx                = patch_ctx->patch_idx;
    HLM_PATCH_PARAM patch_param      = patch_ctx->sps.patch_info.patch_param[patch_idx];
    HLM_S32 patch_x[3]               = { patch_param.patch_x, patch_param.patch_x, patch_param.patch_x };
    HLM_S32 patch_y[3]               = { patch_param.patch_y, patch_param.patch_y, patch_param.patch_y };
    HLM_S32 patch_width[2]           = { patch_param.patch_width[0], patch_param.patch_width[1] };
    HLM_S32 patch_height[2]          = { patch_param.patch_height[0],patch_param.patch_height[1] };
    HLM_U32 src_stride               = 0;
    HLM_U32 dst_stride               = 0;
    HLM_U16 *src_base                = HLM_NULL;
    HLM_U16 *dst_base                = HLM_NULL;
    HLM_U16 *src                     = HLM_NULL;
    HLM_U16 *dst                     = HLM_NULL;
    HLM_S32 real_patch_width         = HLM_MIN(patch_width[0], (HLM_S32)regs->enc_real_pic_width[0] - patch_x[0]);
    HLM_S32 real_patch_height        = HLM_MIN(patch_height[0], (HLM_S32)regs->enc_real_pic_height[0] - patch_y[0]);
    HLM_S32 real_patch_width_yuv[3]  = { real_patch_width ,real_patch_width ,real_patch_width };
    HLM_S32 real_patch_height_yuv[3] = { real_patch_height ,real_patch_height ,real_patch_height };

    switch (spec->sps.format)
    {
    case HLM_IMG_YUV_420:
        real_patch_width_yuv[1]  = real_patch_width_yuv[0] >> 1;
        real_patch_height_yuv[1] = real_patch_height_yuv[0] >> 1;
        patch_x[1]               = patch_x[0] >> 1;
        patch_y[1]               = patch_y[0] >> 1;
        break;
    case HLM_IMG_YUV_422:
        real_patch_width_yuv[1]  = real_patch_width_yuv[0] >> 1;
        real_patch_height_yuv[1] = real_patch_height_yuv[0];
        patch_x[1]               = patch_x[0] >> 1;
        patch_y[1]               = patch_y[0];
        break;
    }
    real_patch_width_yuv[2] = real_patch_width_yuv[1];
    real_patch_height_yuv[2] = real_patch_height_yuv[1];
    patch_x[2] = patch_x[1];
    patch_y[2] = patch_y[1];

    //patch 的重建写入整帧重建图像
    HLM_S32 yuv_comp = regs->image_format == HLM_IMG_YUV_400 ? 1 : HLM_MAX_COMP_NUM;
    for (i = HLM_LUMA_Y; i < yuv_comp; i++)
    {
        if (i == HLM_LUMA_Y)
        {
            src_stride = regs->enc_input_luma_stride;
            src_base   = regs->enc_recon_y_base;
            dst_stride = regs->enc_real_input_luma_stride;
            dst_base   = regs->enc_real_rec_y_base + patch_y[i] * dst_stride + patch_x[i];
        }
        else if (i == HLM_CHROMA_U)
        {
            src_stride = regs->enc_input_chroma_stride;
            src_base   = regs->enc_recon_cb_base;
            dst_stride = regs->enc_real_input_chroma_stride;
            dst_base   = regs->enc_real_rec_cb_base + patch_y[i] * dst_stride + patch_x[i];
        }
        else
        {
            src_stride = regs->enc_input_chroma_stride;
            src_base   = regs->enc_recon_cr_base;
            dst_stride = regs->enc_real_input_chroma_stride;
            dst_base   = regs->enc_real_rec_cr_base + patch_y[i] * dst_stride + patch_x[i];
        }

        /*to padding the right*/
        src = src_base;
        dst = dst_base;
        for (j = 0; j < real_patch_height_yuv[i]; j++)
        {
            memcpy(dst, src, real_patch_width_yuv[i] * sizeof(HLM_U16));
            src += src_stride;
            dst += dst_stride;
        }
    }
}
