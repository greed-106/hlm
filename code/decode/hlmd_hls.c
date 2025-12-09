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
#include "hlmd_hls.h"
#include "hlmd_dpb.h"
#include "hlmd_ecd.h"
#include "hlmd_patch.h"

// 写寄存器
HLM_VOID HLMD_HLS_write_reg(HLMD_SW_SPEC         *spec,
                            HLMD_BITSTREAM       *bs,
                            HLM_FRAME_TYPE        patch_type,
                            HLMD_REGS            *regs)
{
    HLMD_PATCH_CTX  *patch_ctx = &(spec->patch_ctx);
    HLMD_FRAME      *cur_frame = spec->cur_frame;

    regs->patch_ctx                  = patch_ctx;
    regs->cur_frame                  = spec->cur_frame;
    regs->segment_enable_flag        = patch_ctx->patch_header.segment_enable_flag;
    regs->segment_width_in_log2      = patch_ctx->patch_header.segment_width_in_log2;
    regs->segment_height_in_log2     = patch_ctx->patch_header.segment_height_in_log2;
    regs->dec_frame_coding_type      = patch_type;
    regs->bs                         = bs;
    regs->dec_pic_width              = spec->ability.code_width[0];
    regs->dec_pic_height             = spec->ability.code_height[0];
    regs->dec_bitdepth               = spec->sps.bit_depth_luma_minus8 + 8;
    regs->dec_pic_luma_bitdepth      = spec->ability.bit_depth_luma;
    regs->dec_pic_chroma_bitdepth    = spec->ability.bit_depth_chroma;
    regs->mv_ref_cross_patch         = spec->sps.mv_ref_cross_patch;
    regs->inter_pad_w_left           = spec->sps.mv_search_width >> 1;
    regs->inter_pad_w_right          = spec->sps.mv_search_width - 1 - regs->inter_pad_w_left;
    regs->inter_pad_h_up             = spec->sps.mv_search_height >> 1;
    regs->inter_pad_h_down           = spec->sps.mv_search_height - 1 - regs->inter_pad_h_up;
    regs->dec_output_luma_stride     = cur_frame->ref_pic_data->yuv.step[0];
    regs->dec_output_chroma_stride   = cur_frame->ref_pic_data->yuv.step[1];
    regs->nbi_info                   = &spec->nbi_info;
    regs->cur_patch_param            = &spec->sps.patch_info.patch_param[patch_ctx->patch_header.patch_idx];
    regs->image_format               = spec->sps.format;
    regs->dec_recon_y_base           = cur_frame->ref_pic_data->patch_padding_recon[0];
    regs->dec_recon_cb_base          = cur_frame->ref_pic_data->patch_padding_recon[1];
    regs->dec_recon_cr_base          = cur_frame->ref_pic_data->patch_padding_recon[2];
    regs->intra_8x8_enable_flag      = spec->sps.intra_8x8_enable_flag;
    regs->intra_16x1_2x8_enable_flag = spec->sps.intra_16x1_2x8_enable_flag;
    regs->cu_delta_qp_enable_flag    = spec->sps.cu_delta_qp_enable_flag;
    regs->cur_cu                     = &spec->cur_cu;
    regs->sub_ibc_enable_flag        = spec->sps.sub_ibc_enable_flag;
#if INTRA_CHROMA_MODE_SEPARATE
    regs->intra_chroma_mode_enable_flag = spec->sps.intra_chroma_mode_enable_flag;
    regs->intra_sub_chroma_mode_pm_enable_flag = spec->sps.intra_sub_chroma_mode_enable_flag;
#endif
    // 初始化regs->cur_cu->com_cu_info
    regs->cur_cu->com_cu_info.cu_width[0]     = HLM_LOG2_WIDTH_SIZE;
    regs->cur_cu->com_cu_info.cu_height[0]    = HLM_LOG2_HEIGHT_SIZE;
    regs->cur_cu->com_cu_info.chroma_offset_x = 0;
    regs->cur_cu->com_cu_info.chroma_offset_y = 0;
    switch (regs->image_format)
    {
    case HLM_IMG_YUV_444:
    case HLM_IMG_RGB:
        regs->cur_cu->com_cu_info.cu_width[1]     = HLM_LOG2_WIDTH_SIZE;
        regs->cur_cu->com_cu_info.cu_height[1]    = HLM_LOG2_HEIGHT_SIZE;
        regs->cur_cu->com_cu_info.cu_width[2]     = HLM_LOG2_WIDTH_SIZE;
        regs->cur_cu->com_cu_info.cu_height[2]    = HLM_LOG2_HEIGHT_SIZE;
        regs->cur_cu->com_cu_info.chroma_offset_x = 0;
        regs->cur_cu->com_cu_info.chroma_offset_y = 0;
        break;
    case HLM_IMG_YUV_422:
        regs->cur_cu->com_cu_info.cu_width[1]     = (HLM_LOG2_WIDTH_SIZE - 1);
        regs->cur_cu->com_cu_info.cu_height[1]    = HLM_LOG2_HEIGHT_SIZE;
        regs->cur_cu->com_cu_info.cu_width[2]     = (HLM_LOG2_WIDTH_SIZE - 1);
        regs->cur_cu->com_cu_info.cu_height[2]    = HLM_LOG2_HEIGHT_SIZE;
        regs->cur_cu->com_cu_info.chroma_offset_x = 1;
        break;
    case HLM_IMG_YUV_420:
        regs->cur_cu->com_cu_info.cu_width[1]     = (HLM_LOG2_WIDTH_SIZE - 1);
        regs->cur_cu->com_cu_info.cu_height[1]    = (HLM_LOG2_HEIGHT_SIZE - 1);
        regs->cur_cu->com_cu_info.cu_width[2]     = (HLM_LOG2_WIDTH_SIZE - 1);
        regs->cur_cu->com_cu_info.cu_height[2]    = (HLM_LOG2_HEIGHT_SIZE - 1);
        regs->cur_cu->com_cu_info.chroma_offset_x = 1;
        regs->cur_cu->com_cu_info.chroma_offset_y = 1;
        break;
    }
}

// 读寄存器
HLM_VOID HLMD_HLS_read_reg(HLMD_PROCESS_OUT *output,
                           HLMD_REGS        *regs)
{
    HLM_S32 width_out       = HLM_SIZE_ALIGN_16(regs->dec_pic_width);
    HLM_S32 height_out      = HLM_SIZE_ALIGN_8(regs->dec_pic_height);
    HLMD_YUV_FRAME *yuv_dec = &regs->cur_frame->ref_pic_data->yuv;
    HLM_S32 poc             = regs->patch_ctx->patch_header.pps.poc;
    HLM_U16 *src            = HLM_NULL;
    HLM_U16 *dst            = HLM_NULL;
    HLM_U08 comp            = 0;
    HLM_S32 yuv_comp        = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;

    output->image_poc           = poc;
    output->image_out.width[0]  = width_out;
    output->image_out.height[0] = height_out;
    output->image_out.step[0]   = width_out;
    output->image_out.step[1]   = width_out;
    output->image_out.step[2]   = width_out;
    switch (regs->image_format)
    {
    case HLM_IMG_YUV_444:
    case HLM_IMG_RGB:
        output->image_out.width[1]  = output->image_out.width[0];
        output->image_out.height[1] = output->image_out.height[0];
        break;
    case HLM_IMG_YUV_420:
        output->image_out.width[1]  = output->image_out.width[0] >> 1;
        output->image_out.height[1] = output->image_out.height[0] >> 1;
        break;
    case HLM_IMG_YUV_422:
        output->image_out.width[1]  = output->image_out.width[0] >> 1;
        output->image_out.height[1] = output->image_out.height[0];
        break;
    }
    for (comp = 0; comp < yuv_comp; comp++)
    {
        src = yuv_dec->data[comp];
        dst = (HLM_U16 *)(output->image_out.data[comp]);
        memcpy(dst, src, output->image_out.width[comp != 0] * output->image_out.height[comp != 0] * sizeof(HLM_U16));
    }
}

// 解析序列头
HLM_STATUS HLMD_HLS_parse_seq_header(HLMD_BITSTREAM    *bs,
                                     HLM_PARAM_SPS     *sps)
{
    HLM_S32 i                   = 0;
    HLM_S32 uniform_patch_split = 1;  // patch划分是否均匀
    HLM_S32 seq_horizontal_size = 0;
    HLM_S32 seq_vertical_size   = 0;
    HLM_S32 padded_pic_width    = 0;
    HLM_S32 padded_pic_height   = 0;

    sps->profile                 = HLMD_ECD_ReadBits(bs,  8);  // profile_id
    seq_horizontal_size          = HLMD_ECD_ReadBits(bs, 32);  // seq_horizontal_size
    seq_vertical_size            = HLMD_ECD_ReadBits(bs, 32);  // seq_vertical_size
    sps->bit_depth_luma_minus8   = HLMD_ECD_ReadBits(bs,  4);  // seq_bit_depth_luma_minus8
    sps->bit_depth_chroma_minus8 = HLMD_ECD_ReadBits(bs,  4);  // seq_bit_depth_chroma_minus8
    sps->bpp_i                   = HLMD_ECD_ReadBits(bs, 12);  // seq_bpp_i_picture
    sps->bpp_p                   = HLMD_ECD_ReadBits(bs, 12);  // seq_bpp_p_picture
    sps->format                  = HLMD_ECD_ReadBits(bs,  3);  // seq_chroma_format
    sps->i_frame_enable_ibc      = HLMD_ECD_ReadBits(bs,  1);  // seq_i_picture_ibc_enable_flag
    sps->p_frame_enable_ibc      = HLMD_ECD_ReadBits(bs,  1);  // seq_p_picture_ibc_enable_flag
    if (sps->i_frame_enable_ibc || sps->p_frame_enable_ibc)
    {
        sps->sub_ibc_enable_flag = HLMD_ECD_ReadBits(bs,  1);  // seq_sub_ibc_enable_flag
    }
#if FIX_2
    else
    {
        sps->sub_ibc_enable_flag = 0;
    }
#endif
#if FIX_3
    sps->bitdepth = sps->bit_depth_luma_minus8 + 8;
#endif

    sps->mv_ref_cross_patch         = HLMD_ECD_ReadBits(bs,  1);  // seq_mv_cross_patch_enable_flag
    sps->intra_8x8_enable_flag      = HLMD_ECD_ReadBits(bs,  1);  // seq_intra_8x8_enable_flag
    sps->intra_16x1_2x8_enable_flag = HLMD_ECD_ReadBits(bs, 1);  // seq_intra_16x1_2x8_enable_flag
#if INTRA_CHROMA_MODE_SEPARATE
    if (sps->format != HLM_IMG_YUV_400)
    {
        sps->intra_chroma_mode_enable_flag = HLMD_ECD_ReadBits(bs, 1);  // seq_intra_chroma_pm_enable_flag
        sps->intra_sub_chroma_mode_enable_flag = HLMD_ECD_ReadBits(bs, 1);  // seq_intra_chroma_pm_enable_flag
    }
    else
    {
        sps->intra_chroma_mode_enable_flag = 0;  
        sps->intra_sub_chroma_mode_enable_flag = 0;
    }
#endif
    sps->cu_delta_qp_enable_flag    = HLMD_ECD_ReadBits(bs,  1);  // seq_cu_delta_qp_enable_flag
    sps->mv_limit_enable_flag       = HLMD_ECD_ReadBits(bs,  1);  // seq_mv_search_range_limit_enable_flag
    if (sps->mv_limit_enable_flag)
    {
        sps->mv_search_width     = HLMD_ECD_ReadBits(bs,  8);  // seq_mv_search_range_width
        sps->mv_search_height    = HLMD_ECD_ReadBits(bs,  8);  // seq_mv_search_range_height
    }
    else
    {
        sps->mv_search_width     = 1 << 16;
        sps->mv_search_height    = 1 << 16;
    }
    sps->patch_info.patch_num = HLMD_ECD_ReadBits(bs, 8) + 1;  // seq_patch_num_minus1
    for (i = 0; i < (HLM_S32)sps->patch_info.patch_num; i++)
    {
        sps->patch_info.patch_param[i].patch_x               = HLMD_ECD_ReadBits(bs, 32);  // seq_patch_x
        sps->patch_info.patch_param[i].patch_y               = HLMD_ECD_ReadBits(bs, 32);  // seq_patch_y
        sps->patch_info.patch_param[i].patch_width[0]        = HLMD_ECD_ReadBits(bs, 32);  // seq_patch_width
        sps->patch_info.patch_param[i].patch_height[0]       = HLMD_ECD_ReadBits(bs, 32);  // seq_patch_height
        sps->patch_info.patch_param[i].patch_coded_width[0]  = HLM_SIZE_ALIGN_16(sps->patch_info.patch_param[i].patch_width[0]);
        sps->patch_info.patch_param[i].patch_coded_height[0] = HLM_SIZE_ALIGN_8(sps->patch_info.patch_param[i].patch_height[0]);
        switch (sps->format)
        {
        case HLM_IMG_YUV_444:
        case HLM_IMG_RGB:
            sps->patch_info.patch_param[i].patch_width[1]        = sps->patch_info.patch_param[i].patch_width[0];
            sps->patch_info.patch_param[i].patch_height[1]       = sps->patch_info.patch_param[i].patch_height[0];
            sps->patch_info.patch_param[i].patch_coded_width[1]  = sps->patch_info.patch_param[i].patch_coded_width[0];
            sps->patch_info.patch_param[i].patch_coded_height[1] = sps->patch_info.patch_param[i].patch_coded_height[0];
            break;
        case HLM_IMG_YUV_420:
            sps->patch_info.patch_param[i].patch_width[1]        = sps->patch_info.patch_param[i].patch_width[0] >> 1;
            sps->patch_info.patch_param[i].patch_height[1]       = sps->patch_info.patch_param[i].patch_height[0] >> 1;
            sps->patch_info.patch_param[i].patch_coded_width[1]  = sps->patch_info.patch_param[i].patch_coded_width[0] >> 1;
            sps->patch_info.patch_param[i].patch_coded_height[1] = sps->patch_info.patch_param[i].patch_coded_height[0] >> 1;
            break;
        case HLM_IMG_YUV_422:
            sps->patch_info.patch_param[i].patch_width[1]        = sps->patch_info.patch_param[i].patch_width[0] >> 1;
            sps->patch_info.patch_param[i].patch_height[1]       = sps->patch_info.patch_param[i].patch_height[0];
            sps->patch_info.patch_param[i].patch_coded_width[1]  = sps->patch_info.patch_param[i].patch_coded_width[0] >> 1;
            sps->patch_info.patch_param[i].patch_coded_height[1] = sps->patch_info.patch_param[i].patch_coded_height[0];
            break;
        }
    }

    sps->pic_width_in_cus_minus1        = ((seq_horizontal_size + 15) >> 4) - 1;
    sps->pic_height_in_map_units_minus1 = ((seq_vertical_size + 7) >> 3) - 1;
    padded_pic_width                    = (sps->pic_width_in_cus_minus1 + 1) << 4;
    padded_pic_height                   = (sps->pic_height_in_map_units_minus1 + 1) << 3;
    if (seq_horizontal_size != padded_pic_width || seq_vertical_size != padded_pic_height)
    {
        sps->frame_cropping_flag      = 1;
        sps->frame_crop_left_offset   = 0;
        sps->frame_crop_right_offset  = padded_pic_width - seq_horizontal_size;
        sps->frame_crop_top_offset    = 0;
        sps->frame_crop_bottom_offset = padded_pic_height - seq_vertical_size;
    }
    else
    {
        sps->frame_cropping_flag      = 0;
        sps->frame_crop_left_offset   = 0;
        sps->frame_crop_right_offset  = 0;
        sps->frame_crop_top_offset    = 0;
        sps->frame_crop_bottom_offset = 0;
    }

    if (sps->format == HLM_IMG_YUV_420 || sps->format == HLM_IMG_YUV_422)
    {
        sps->intra_8x8_enable_flag = 1;  // 420和422强制intra8x8
    }

    // 校验patch划分的合理性
    assert(0 < sps->patch_info.patch_num && sps->patch_info.patch_num <= HLM_MAX_PATCH_NUM);
    for (i = 1; i < (HLM_S32)sps->patch_info.patch_num; i++)
    {
        if (sps->patch_info.patch_param[i].patch_width[0] != sps->patch_info.patch_param[0].patch_width[0] ||
            sps->patch_info.patch_param[i].patch_height[0] != sps->patch_info.patch_param[0].patch_height[0])
        {
            uniform_patch_split = 0;
            break;
        }
    }
    if (uniform_patch_split == 0 && 0 == HLM_COM_CheckPatchSplit(
        ((sps->pic_width_in_cus_minus1 + 1) << 4) - sps->frame_crop_left_offset - sps->frame_crop_right_offset,  // 真实的图像宽高
        ((sps->pic_height_in_map_units_minus1 + 1) << 3) - sps->frame_crop_top_offset - sps->frame_crop_bottom_offset,
        &sps->patch_info))
    {
        printf("Patch划分信息无法拼成完整图像!\n");
        assert(0);
    }

    return HLM_STS_OK;
}

// 解析图像头
HLM_STATUS HLMD_HLS_parse_pic_header(HLMD_BITSTREAM    *bs,
                                     HLM_PARAM_SPS     *sps,
                                     HLM_PARAM_PPS     *pps)
{
    HLM_S32 value         = 0;
#if FIX_3
    HLM_S32 pic_chroma_qp = 0;
    HLM_S32 max_qp        = HLM_MAX_QP(sps->bitdepth);
#endif

    value         = HLMD_ECD_ReadBits(bs,  1);  // pic_type
    pps->pic_type = value == 1 ? HLM_FRAME_TYPE_I : HLM_FRAME_TYPE_P;  // 0:P, 1:I
    pps->poc      = HLMD_ECD_ReadBits(bs, 16);  // pic_poc
    if (!sps->cu_delta_qp_enable_flag)
    {
        pps->pic_luma_qp         = HLMD_ECD_ReadUeGolomb(bs);  // pic_luma_qp
        pps->pic_chroma_delta_qp = HLMD_ECD_ReadSeGolomb(bs);  // pic_chroma_delta_qp
#if FIX_3
        pic_chroma_qp = pps->pic_luma_qp + pps->pic_chroma_delta_qp;
        HLM_CHECK_ERROR(pps->pic_luma_qp < 0 || pps->pic_luma_qp > max_qp, HLM_STS_ERR_PARAM_VALUE);
        HLM_CHECK_ERROR(pic_chroma_qp    < 0 || pic_chroma_qp    > max_qp, HLM_STS_ERR_PARAM_VALUE);
#endif
    }
    else
    {
#if FIX_3
        pps->pic_luma_qp         = CBR_INIT_QP;
#else
#if QP_RANGE_ADJUST
        pps->pic_luma_qp         = 22;  // 每帧图像第一个CU的基准qp
#else
        pps->pic_luma_qp         = 26;  // 每帧图像第一个CU的基准qp
#endif
#endif
        pps->pic_chroma_delta_qp = 0;
    }

    return HLM_STS_OK;
}

// 解析patch头
HLM_STATUS HLMD_HLS_parse_patch_header(HLMD_BITSTREAM            *bs,
                                       HLMD_NALU_HEADER          *nalu_header,
                                       HLMD_PATCH_CTX            *patch_ctx,
                                       HLM_PARAM_SPS             *sps,
                                       HLM_PARAM_PPS             *pps)
{
    HLM_PATCH_HEADER *sh    = &(patch_ctx->patch_header);
    HLM_S32 val             = 0;
    HLM_S32 ibc_enable_flag = pps->pic_type == HLM_FRAME_TYPE_I ? sps->i_frame_enable_ibc : sps->p_frame_enable_ibc;
    HLM_S32 patch_x         = 0;
    HLM_S32 patch_y         = 0;
    HLM_S32 patch_width     = 0;
    HLM_S32 patch_height    = 0;

#if FIX_2
    sh->patch_idx                       = HLMD_ECD_ReadBits(bs,  8);  // patch_id
#else
    sh->patch_idx                       = HLMD_ECD_ReadUeGolomb(bs);  // patch_id
#endif
    sh->patch_bpp                       = HLMD_ECD_ReadBits(bs, 12);  // patch_bpp
    sh->segment_enable_flag             = HLMD_ECD_ReadBits(bs,  1);  // patch_segment_enable_flag
#if FIX_2
    if (sh->segment_enable_flag)
    {
        val                             = HLMD_ECD_ReadBits(bs,  8);  // patch_log2_segment_width_minus4
        sh->segment_width_in_log2       = val + 4;
        val                             = HLMD_ECD_ReadBits(bs,  8);  // patch_log2_segment_height_minus3
        sh->segment_height_in_log2      = val + 3;
    }
    else
    {
        sh->segment_width_in_log2       = 0;
        sh->segment_height_in_log2      = 0;
    }
#else
    val                                 = HLMD_ECD_ReadBits(bs,  8);  // patch_log2_segment_width_minus4
    sh->segment_width_in_log2           = val + 4;
    val                                 = HLMD_ECD_ReadBits(bs,  8);  // patch_log2_segment_height_minus3
    sh->segment_height_in_log2          = val + 3;
#endif
    sh->patch_extra_params_present_flag = HLMD_ECD_ReadBits(bs,  1);  // patch_extra_params_present_flag
    assert(sh->patch_idx < sps->patch_info.patch_num);
    memcpy(&sh->sps, sps, sizeof(HLM_PARAM_SPS));
    memcpy(&sh->pps, pps, sizeof(HLM_PARAM_PPS));

    patch_x                      = sps->patch_info.patch_param[sh->patch_idx].patch_x;
    patch_y                      = sps->patch_info.patch_param[sh->patch_idx].patch_y;
    patch_width                  = sps->patch_info.patch_param[sh->patch_idx].patch_width[0];
    patch_height                 = sps->patch_info.patch_param[sh->patch_idx].patch_height[0];
    patch_ctx->idr_frame_flag    = (HLM_IDR_NUT == nalu_header->nal_unit_type) ? 1 : 0;
    patch_ctx->nal_ref_idc       = nalu_header->nal_ref_idc;
    patch_ctx->first_cu_in_patch = 0;

    // 校验patch级重传的序列头和图像头语法
    if (sh->patch_extra_params_present_flag)
    {
#if FIX_2
        val = HLMD_ECD_ReadBits(bs, 4);  // patch_bit_depth_luma_minus8
        HLM_CHECK_ERROR(val != sps->bit_depth_luma_minus8, HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 4);  // patch_bit_depth_chroma_minus8
        HLM_CHECK_ERROR(val != sps->bit_depth_chroma_minus8, HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 3);  // patch_chroma_format
        HLM_CHECK_ERROR(val != sps->format, HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 1);  // patch_ibc_enable_flag
        HLM_CHECK_ERROR(val != ibc_enable_flag, HLM_STS_ERR);
        if (ibc_enable_flag)
        {
            val = HLMD_ECD_ReadBits(bs, 1);  // patch_sub_ibc_enable_flag
            HLM_CHECK_ERROR(val != sps->sub_ibc_enable_flag, HLM_STS_ERR);
        }

        val = HLMD_ECD_ReadBits(bs, 1);  // patch_intra_8x8_enable_flag
        HLM_CHECK_ERROR(val != sps->intra_8x8_enable_flag, HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 1);  // patch_intra_16x1_2x8_enable_flag
        HLM_CHECK_ERROR(val != sps->intra_16x1_2x8_enable_flag, HLM_STS_ERR);
#if INTRA_CHROMA_MODE_SEPARATE
        if (sps->format != HLM_IMG_YUV_400)
        {
            val = HLMD_ECD_ReadBits(bs, 1);  // patch_intra_uv_pm_enable_flag
            HLM_CHECK_ERROR(val != sps->intra_chroma_mode_enable_flag, HLM_STS_ERR);
            val = HLMD_ECD_ReadBits(bs, 1);  // patch_intra_uv_sub_pm_enable_flag
            HLM_CHECK_ERROR(val != sps->intra_sub_chroma_mode_enable_flag, HLM_STS_ERR);
        }
#endif
        val = HLMD_ECD_ReadBits(bs, 1);  // patch_cu_delta_qp_enable_flag
        HLM_CHECK_ERROR(val != sps->cu_delta_qp_enable_flag, HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 1);  // patch_mv_search_range_limit_enable_flag
        HLM_CHECK_ERROR(val != sps->mv_limit_enable_flag, HLM_STS_ERR);
        if (sps->mv_limit_enable_flag)
        {
            val = HLMD_ECD_ReadBits(bs, 8);  // patch_mv_search_range_width
            HLM_CHECK_ERROR(val != sps->mv_search_width, HLM_STS_ERR);
            val = HLMD_ECD_ReadBits(bs, 8);  // patch_mv_search_range_height
            HLM_CHECK_ERROR(val != sps->mv_search_height, HLM_STS_ERR);
        }
        val = HLMD_ECD_ReadBits(bs, 32);  // patch_x
        HLM_CHECK_ERROR(val != patch_x, HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 32);  // patch_y
        HLM_CHECK_ERROR(val != patch_y, HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 32);  // patch_width
        HLM_CHECK_ERROR(val != patch_width, HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 32);  // patch_height
        HLM_CHECK_ERROR(val != patch_height, HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 1);  // patch_type
        HLM_CHECK_ERROR(val != (pps->pic_type == HLM_FRAME_TYPE_I ? 1 : 0), HLM_STS_ERR);
        val = HLMD_ECD_ReadBits(bs, 16);  // patch_poc
        HLM_CHECK_ERROR(val != pps->poc, HLM_STS_ERR);
#if FIX_3
        if (!sps->cu_delta_qp_enable_flag)
#else
        if (sps->cu_delta_qp_enable_flag)
#endif
        {
            val = HLMD_ECD_ReadUeGolomb(bs);  // patch_luma_qp
            HLM_CHECK_ERROR(val != pps->pic_luma_qp, HLM_STS_ERR);
            val = HLMD_ECD_ReadSeGolomb(bs);  // patch_chroma_delta_qp
            HLM_CHECK_ERROR(val != pps->pic_chroma_delta_qp, HLM_STS_ERR);
        }
#else
        assert(sps->bit_depth_luma_minus8   == HLMD_ECD_ReadBits(bs,  4));  // patch_bit_depth_luma_minus8
        assert(sps->bit_depth_chroma_minus8 == HLMD_ECD_ReadBits(bs,  4));  // patch_bit_depth_chroma_minus8
        assert(sps->format                  == HLMD_ECD_ReadBits(bs,  3));  // patch_chroma_format
        assert(ibc_enable_flag              == HLMD_ECD_ReadBits(bs,  1));  // patch_ibc_enable_flag
        if (ibc_enable_flag)
        {
            assert(sps->sub_ibc_enable_flag == HLMD_ECD_ReadBits(bs,  1));  // patch_sub_ibc_enable_flag
        }

        assert(sps->intra_8x8_enable_flag      == HLMD_ECD_ReadBits(bs,  1));  // patch_intra_8x8_enable_flag
        assert(sps->intra_16x1_2x8_enable_flag == HLMD_ECD_ReadBits(bs, 1));  // patch_intra_16x1_2x8_enable_flag
#if INTRA_CHROMA_MODE_SEPARATE
        if (sps->format != HLM_IMG_YUV_400)
        {
            assert(sps->intra_chroma_mode_enable_flag == HLMD_ECD_ReadBits(bs, 1));  // patch_intra_uv_pm_enable_flag
            assert(sps->intra_sub_chroma_mode_enable_flag == HLMD_ECD_ReadBits(bs, 1));  // patch_intra_uv_sub_pm_enable_flag
        }
#endif
        assert(sps->cu_delta_qp_enable_flag    == HLMD_ECD_ReadBits(bs,  1));  // patch_cu_delta_qp_enable_flag
        assert(sps->mv_limit_enable_flag       == HLMD_ECD_ReadBits(bs,  1));  // patch_mv_search_range_limit_enable_flag
        if (sps->mv_limit_enable_flag)
        {
            assert(sps->mv_search_width     == HLMD_ECD_ReadBits(bs,  8));  // patch_mv_search_range_width
            assert(sps->mv_search_height    == HLMD_ECD_ReadBits(bs,  8));  // patch_mv_search_range_height
        }
        assert(patch_x                      == HLMD_ECD_ReadBits(bs, 32));  // patch_x
        assert(patch_y                      == HLMD_ECD_ReadBits(bs, 32));  // patch_y
        assert(patch_width                  == HLMD_ECD_ReadBits(bs, 32));  // patch_width
        assert(patch_height                 == HLMD_ECD_ReadBits(bs, 32));  // patch_height
        assert(pps->pic_type                == HLMD_ECD_ReadBits(bs,  1));  // patch_type
        assert(pps->poc                     == HLMD_ECD_ReadBits(bs, 16));  // patch_poc
        if (sps->cu_delta_qp_enable_flag)
        {
            assert(pps->pic_luma_qp         == HLMD_ECD_ReadUeGolomb(bs));  // patch_luma_qp
            assert(pps->pic_chroma_delta_qp == HLMD_ECD_ReadSeGolomb(bs));  // patch_chroma_delta_qp
        }
#endif
    }

    return HLM_STS_OK;
}

// 解析sei
HLM_STATUS HLMD_HLS_parse_sei(HLMD_BITSTREAM    *bs,
                              HLMD_FRAME        *cur_frame,
                              HLM_S32            poc,
                              HLMD_PROCESS_OUT  *yuv_out)
{
    HLM_U08 cur_img_md5[16] = { 0 };
    HLM_PARAM_SEI psei      = { 0 };
    HLM_S32 left_bit_len    = bs->max_bits_num;  // 剩下的待解析的码流长度，单位是比特
    HLM_S32 tmp             = 0;
    HLM_U32 i               = 0;

    // 遍历所有sei数据
    while (left_bit_len > 0)
    {
        // 解析payload_type
        psei.payload_type = 0;
        do
        {
            tmp = HLMD_ECD_ReadBits(bs, 8);
            left_bit_len -= 8;
            psei.payload_type += tmp;
        } while (tmp == 255);

        // 解析payload_size
        psei.payload_size = 0;
        do
        {
            tmp = HLMD_ECD_ReadBits(bs, 8);
            left_bit_len -= 8;
            psei.payload_size += tmp;
        } while (tmp == 255);

        // 计算解码图像md5
        if (psei.payload_type == USER_DATA_MD5)
        {
            if (psei.payload_size != 16)
            {
                yuv_out->is_mismatch = 1;  // 码流中的md5错误
            }
            else
            {
                // cur_frame->ref_pic_data->patch_padding_recon存放一个patch的yuv数据，有padding
                // cur_frame->ref_pic_data->yuv.data存放一帧的yuv数据，是将patch数据裁剪后拼在一起
                // yuv_out->image_out.data用于外部输出，对于rgb序列，存储的是转化后的rgb数据
                HLM_COM_GetMd5(cur_frame->ref_pic_data->yuv.data[0],
                    cur_frame->ref_pic_data->yuv.data[1],
                    cur_frame->ref_pic_data->yuv.data[2],
                    cur_img_md5,
                    yuv_out->image_out.width[0],
                    yuv_out->image_out.height[0],
                    yuv_out->image_out.width[1],
                    yuv_out->image_out.height[1]);
            }
        }

        // 解析payload_data
        for (i = 0; i < psei.payload_size; i++)
        {
            tmp = HLMD_ECD_ReadBits(bs, 8);
            left_bit_len -= 8;

            // 校验md5
            if (psei.payload_type == USER_DATA_MD5)
            {
                if (tmp != cur_img_md5[HLM_MIN(i, 15)])
                {
                    yuv_out->is_mismatch = 1;
                }
            }
        }
    }

    printf(">>> check md5 of poc %d, match = %d\n", poc, !yuv_out->is_mismatch);

    return HLM_STS_OK;
}

// 参考帧padding
HLM_VOID HLMD_HLS_padding(HLMD_SW_SPEC  *spec,
                          HLMD_REGS     *regs)
{
    HLM_S32 i                    = 0;
    HLM_S32 j                    = 0;
    HLM_U32 src_w                = regs->dec_pic_width;
    HLM_U32 src_h                = regs->dec_pic_height;
    HLM_U32 src_stride           = 0;
    HLM_S32 pos_x[4]             = { 0 };
    HLM_S32 pos_y[4]             = { 0 };
    HLM_S32 pad_left_w           = regs->inter_pad_w_left;
    HLM_S32 pad_right_w          = regs->inter_pad_w_right;
    HLM_S32 pad_up_h             = regs->inter_pad_h_up;
    HLM_S32 pad_down_h           = regs->inter_pad_h_down;
    HLM_S32 pad_stride_w         = 0;
    HLM_S32 pad_stride_h         = 0;
    HLM_U16 *pad_base            = HLM_NULL;
    HLM_U16 *recon_base          = HLM_NULL;
    HLM_U16 *pad_left            = HLM_NULL;
    HLM_U16 *pad_src             = HLM_NULL;
    HLM_U16 *pad_right           = HLM_NULL;
    HLM_U16 *recon_left          = HLM_NULL;
    HLM_U16 *recon_right         = HLM_NULL;
    HLM_U16 *avail_pad_base      = HLM_NULL;
    HLM_U16 *avail_recon_base    = HLM_NULL;
    HLMD_FRAME *cur_frame        = spec->cur_frame;
    HLMD_PATCH_CTX *patch_ctx    = &(spec->patch_ctx);
    HLM_PATCH_PARAM *patch_param = regs->cur_patch_param;
    HLM_S32 patch_x              = patch_param->patch_x;
    HLM_S32 patch_y              = patch_param->patch_y;
    HLM_S32 k                    = 0;
    HLM_S32 yuv_comp             = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_S32 real_patch_width     = HLM_MIN(patch_param->patch_width[0], regs->dec_pic_width - patch_param->patch_x);
    HLM_S32 real_patch_height    = HLM_MIN(patch_param->patch_height[0], regs->dec_pic_height - patch_param->patch_y);
    HLM_S32 chroma_offset_x[2]   = { 0 };
    HLM_S32 chroma_offset_y[2]   = { 0 };

    switch (spec->sps.format)
    {
    case HLM_IMG_YUV_422:
        chroma_offset_x[1] = 1;
        chroma_offset_y[1] = 0;
        break;
    case HLM_IMG_YUV_420:
        chroma_offset_x[1] = 1;
        chroma_offset_y[1] = 1;
        break;
    default:
        break;
    }
    pad_right_w += patch_param->patch_coded_width[0] - real_patch_width;
    pad_down_h  += patch_param->patch_coded_height[0] - real_patch_height;

    // 参考帧地址赋值
    regs->dec_ref_y_padding_base  = cur_frame->ref_pic_data->luma_ref_padding_y;
    regs->dec_ref_cb_padding_base = cur_frame->ref_pic_data->luma_ref_padding_cb;
    regs->dec_ref_cr_padding_base = cur_frame->ref_pic_data->luma_ref_padding_cr;

    // padding后patch的stride为pad_stride_w
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
        pos_x[0]          = patch_param->patch_x - regs->inter_pad_w_left;
        pos_y[0]          = patch_param->patch_y - regs->inter_pad_h_up;
        pos_x[1]          = patch_param->patch_x + patch_param->patch_coded_width[0] + regs->inter_pad_w_right;
        pos_y[1]          = patch_param->patch_y + patch_param->patch_coded_height[0] + regs->inter_pad_h_down;
        pos_x[2]          = HLM_MAX(0, pos_x[0]);
        pos_y[2]          = HLM_MAX(0, pos_y[0]);
        pos_x[3]          = HLM_MIN((HLM_S32)regs->dec_pic_width, pos_x[1]);
        pos_y[3]          = HLM_MIN((HLM_S32)regs->dec_pic_height, pos_y[1]);
        pad_up_h          = pos_y[2] - pos_y[0];
        pad_down_h        = pos_y[1] - pos_y[3];
        pad_left_w        = pos_x[2] - pos_x[0];
        pad_right_w       = pos_x[1] - pos_x[3];
        real_patch_width  = pos_x[3] - pos_x[2];
        real_patch_height = pos_y[3] - pos_y[2];
        patch_x           = pos_x[0] + pad_left_w;
        patch_y           = pos_y[0] + pad_up_h;
    }

    for (i = 0; i < yuv_comp; i++)
    {
        pad_stride_w = regs->inter_pad_w_left + regs->inter_pad_w_right + patch_param->patch_coded_width[i != 0];
        pad_stride_h = regs->inter_pad_h_up + regs->inter_pad_h_down + patch_param->patch_coded_height[i != 0];
        if (i == 0)
        {
            pad_base   = regs->dec_ref_y_padding_base;
            recon_base = patch_ctx->ref_list[0].ref_pic_data.yuv.data[0];
            src_stride = regs->dec_output_luma_stride;
            regs->dec_ref_frame_luma_stride = pad_stride_w;
        }
        else if (i == 1)
        {
            pad_base   = regs->dec_ref_cb_padding_base;
            recon_base = patch_ctx->ref_list[0].ref_pic_data.yuv.data[1];
            src_stride = regs->dec_output_chroma_stride;
            regs->dec_ref_frame_chroma_stride = pad_stride_w;
        }
        else
        {
            pad_base   = regs->dec_ref_cr_padding_base;
            recon_base = patch_ctx->ref_list[0].ref_pic_data.yuv.data[2];
            src_stride = regs->dec_output_chroma_stride;
        }
        recon_base += (patch_y>> chroma_offset_y[i != 0]) * src_stride + (patch_x >> chroma_offset_x[i!=0]);

        /*to padding the left and right first*/
        for (j = pad_up_h; j < (pad_stride_h - pad_down_h); j++)
        {
            pad_left    = pad_base + j * pad_stride_w;
            pad_src     = pad_left + pad_left_w;
            pad_right   = pad_src + (real_patch_width >> chroma_offset_x[i != 0]);
            recon_left  = recon_base + (j - pad_up_h) * src_stride;
            recon_right = recon_left + (real_patch_width >> chroma_offset_x[i != 0]) - 1;
            memcpy(pad_src, recon_left, (real_patch_width >> chroma_offset_x[i != 0]) * sizeof(HLM_U16));
            for (k = 0; k <pad_left_w; k++)
            {
                pad_left[k] = recon_left[0];
            }
            for (k = 0; k <pad_right_w; k++)
            {
                pad_right[k] = recon_right[0];
            }
        }

        /*to padding the up*/
        for (j = (pad_up_h - 1); j >= 0; j--)
        {
            pad_left = pad_base + j * pad_stride_w;
            memcpy(pad_left, pad_left + pad_stride_w, pad_stride_w *sizeof(HLM_U16));
        }

        /*to padding the down*/
        for (j = (pad_stride_h - pad_down_h); j < pad_stride_h; j++)
        {
            pad_left = pad_base + j * pad_stride_w;
            memcpy(pad_left, pad_left - pad_stride_w, pad_stride_w * sizeof(HLM_U16));
        }
    }
}

// 编码rbsp结束符
HLM_S32 HLMD_HLS_rbsp_trailing(HLM_U08 *src,
                               HLM_S32  size)
{
    HLM_S32 bit     = 0;
    HLM_S32 val     = 0;
    HLM_S32 bitscnt = 0;

    //寻找不为零的最后一个byte
    while ((src[0] == 0) && (size > 0))
    {
        src--;
        size--;
        bitscnt += 8;
    }
    if (size > 0)
    {
        val = *src;
        for (bit = 1; bit < 9; bit++)
        {
            if (val & 1)
            {
                return (bit + bitscnt);
            }
            val >>= 1;
        }
    }

    return 0;
}

// 解封装，去除防伪起始码
HLM_STATUS HLMD_HLS_ebsp_to_rbsp(HLM_U08  *ebsp_data,
                                 HLM_S32   ebsp_data_len,
                                 HLM_U08 **rbsp_data,
                                 HLM_S32  *rbsp_data_len)
{
    HLM_S32 i     = 0;
    HLM_S32 j     = 0;
    HLM_S32 count = 0;

    for (i = 0; i < ebsp_data_len; i++)
    {
        if (2 == count)
        {
            if (0x03 == ebsp_data[i])
            {
                if ((ebsp_data_len - 1) != i)
                {
                    count = 0;
                    i++;
                }
            }
        }
        ebsp_data[j] = ebsp_data[i];
        if (0 == ebsp_data[i])
        {
            count++;
        }
        else
        {
            count = 0;
        }
        j++;
    }

    memset(ebsp_data + j, 0, ebsp_data_len - j);

    *rbsp_data_len = (j << 3) - HLMD_HLS_rbsp_trailing(ebsp_data + j - 1, j);
    *rbsp_data = ebsp_data;

    return HLM_STS_OK;
}

// 解码一帧
HLM_STATUS HLMD_HLS_decode_frame(HLMD_REGS *regs)
{
    HLM_STATUS      sts       = HLM_STS_ERR;
    HLMD_HW_SPEC    spec      = { 0 };
    HLMD_PATCH_CTX *patch_ctx = regs->patch_ctx;

    // 初始化spec
    spec.regs             = regs;
    spec.cu_cols          = regs->cur_patch_param->patch_coded_width[0] >> 4;
    spec.cu_rows          = regs->cur_patch_param->patch_coded_height[0] >> 3;
    spec.bit_depth_chroma = regs->dec_pic_chroma_bitdepth;
    spec.bit_depth_luma   = regs->dec_pic_luma_bitdepth;
    spec.cur_cu           = regs->cur_cu;
    spec.nbi_info         = regs->nbi_info;
    spec.bs.bits_cnt      = regs->bs->bits_cnt;
    spec.bs.init_buf      = regs->bs->init_buf;
    spec.bs.max_bits_num  = regs->bs->max_bits_num;

    sts = HLMD_PATCH_Process(&spec, patch_ctx);

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：获得一个nalu单元码流数据
* 参  数：*
*         stream_buf            -I    待解码码流指针（码流指针随解析过程偏移）
*         stream_len            -I    待解码码流长度
*         nalu_buf              -O    ebsp nalu地址
*         nalu_len              -O    ebsp nalu长度
*         start_code_len        -O    nalu起始码的长度
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_HLS_GetNalu(HLM_U08           *stream_buf,
                            HLM_S64            stream_len,
                            HLM_U08          **nalu_buf,
                            HLM_S32           *nalu_len,
                            HLM_S32           *start_code_len)
{
    HLM_S64  rest_len         = stream_len;
    HLM_S32  start_code_found = 0;
    HLM_S32  end_code_found   = 0;
    HLM_U08 *stream_ptr       = stream_buf;

    *nalu_len = 0;
    //将3字节起始码和4字节起始码合并处理
    while (!start_code_found && rest_len >= 3)
    {
        start_code_found = ((0x00 == stream_ptr[0]) && (0x00 == stream_ptr[1]) && (0x01 == stream_ptr[2]));
        stream_ptr++;
        rest_len--;
    }

    *start_code_len = 3 & (-start_code_found);                   //找到起始码，将起始码设置为3，否则为0
    *nalu_buf       = stream_buf + stream_len - (rest_len + 1);  //找到起始码则设置nalu_buf,否则不设置
    stream_ptr     += 3 & (-start_code_found);                   //找到起始码后start_code_found为1，3 & (-1) = 3
    rest_len       -= 3 & (-start_code_found);                   //未找到起始码start_code_found为0，3 & (-0) = 0

    /*这里隐含一个条件，一个有效的NALU除去起始码和终止码之外，至少有一个字节的内容，
    当找到起始码后，终止码或者下一个起始码需要在一个有效的字节之后寻找，寻找下一个起始码0x000001或者当前nalu的终止码0x000000，
    所以stream_ptr[2]<=0x01包含stream_ptr[2]=0x01(下一个nalu的起始码)和0x00(当前nalu的终止码)两种情况。*/
    while (!end_code_found && rest_len >= 3)
    {
        end_code_found = ((0x00 == stream_ptr[0]) && (0x00 == stream_ptr[1]) && (0x01 >= stream_ptr[2]));
        stream_ptr++;
        rest_len--;
    }

    /*一共分为4种情况：
    1、start_code_found找到、end_code_found找到，找到一个完整的nalu，该nalu_len包含前缀长度3；
    2、start_code_found找到、end_code_found未找到，将第一个起始码后的所有码流当成一个nalu；
    3、start_code_found未找到、end_code_found找到，码流错误；
    4、start_code_found未找到、end_code_found未找到，码流错误。
    针对第一种情况*nalu_len=stream_len - rest_len - 1 - (*nalu_buf - stream_buf);
    针对第二种情况*nalu_len=stream_len - rest_len - 1 - (*nalu_buf - stream_buf) + 3;
    若要严格判断出四种情况，则需要对算法进行更改：当没有找到起始码时，地址不偏移，然后再找结束码0x000000。
    这里考虑的是：如找不到起始码则码流一定出错,当起始码未找到时，rest_len一定小于3，所以也就不会找第二个起始码或者结束码，此时*nalu_len=0。
    所以第三种和第四种情况在这里都处理为start_code_found未找到,*nalu_len=0,然后返回码流错误。
    当第一个起始码未找到时，也就无所谓第二个起始码或者终止码，也就不需要加上末尾的rest_len字节。当第一个起始码未找到时，nalu_len=0。*/
    if (start_code_found)
    {
        *nalu_len = (HLM_S32)(stream_len - (rest_len + 1) - (*nalu_buf - stream_buf) + ((rest_len + 1) & (end_code_found - 1)));
        return HLM_STS_OK;
    }

    return HLM_STS_ERR;
}

/***************************************************************************************************
* 功  能：解析nalu头信息
* 参  数：*
*         nalu_buf              -I    跳过起始码的ebsp nalu码流
*         nalu_len              -I    nalu码流长度
*         nalu_header           -O    解析后的nalu头信息
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_HLS_ProcessNaluHeader(HLM_U08           *nalu_buf,
                                      HLM_S32            nalu_len,
                                      HLMD_NALU_HEADER  *nalu_header)
{
    HLM_U08  code  = 0;
    HLM_U08  value = 0;

    code = *nalu_buf;
    value = (code & 0x1F);
    nalu_header->nal_unit_type = value;

    code >>= 5;
    value = (code & 0x3);
    nalu_header->nal_ref_idc = value;

    code >>= 2;
    nalu_header->forbidden_zero_bit = code;

    return HLM_STS_OK;;
}

/***************************************************************************************************
* 功  能：处理一个nalu单元
* 参  数：*
*        spec                 -I    解析高层句柄
*        nalu_buf             -I    ebsp nalu码流
*        nalu_len             -I    nalu码流长度
*        yuv_out              -O    输出显示图像
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_HLS_ProcessNalu(HLMD_SW_SPEC      *spec,
                                HLM_U08           *nalu_buf,
                                HLM_S32            nalu_len,
                                HLMD_PROCESS_OUT  *yuv_out)
{
    HLM_STATUS sts               = HLM_STS_ERR;
    HLM_U08 *nalu_ptr            = nalu_buf;  // 指向当前待解析的码流地址
    HLM_S32 rest_len             = nalu_len;  // 剩下的待解析的码流长度
    HLMD_PATCH_CTX *patch_ctx    = &(spec->patch_ctx);
    HLM_U08 *bs_buf              = HLM_NULL;
    HLM_S32 bs_length            = 0;
    HLMD_REGS regs               = { 0 };
    HLMD_BITSTREAM bs            = { 0 };
    HLMD_NALU_HEADER nalu_header = { 0 };

    sts = HLMD_HLS_ProcessNaluHeader(nalu_ptr, rest_len, &nalu_header);

    // 跳过nalu_header(1字节)
    nalu_ptr += HLMD_NALU_HEADER_LEN;
    rest_len -= HLMD_NALU_HEADER_LEN;
    sts = HLMD_HLS_ebsp_to_rbsp(nalu_ptr, rest_len, &bs_buf, &bs_length);
    HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);

    bs.bits_cnt = 0;
    bs.init_buf = bs_buf;
    bs.max_bits_num = bs_length;

    switch (nalu_header.nal_unit_type)
    {
    case HLM_SPS_NUT:
        sts = HLMD_HLS_parse_seq_header(&bs, &spec->sps);
        HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);
        break;
    case HLM_PPS_NUT:
        sts = HLMD_HLS_parse_pic_header(&bs, &spec->sps, &spec->pps);
        HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);
        break;
    case HLM_IDR_NUT:
    case HLM_NONIDR_NUT:
        // 在每帧的第一个patch解码之前，获取当前帧在dpb中的存放空间
        if (spec->coded_patch_cnt == 0)
        {
            sts = HLMD_DPB_GetCurrFrame(spec->dpb_handle, spec->dec_pic_buf, &(spec->cur_frame), &spec->dpb_free_index);
            HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);
        }

        sts = HLMD_HLS_parse_patch_header(&bs, &nalu_header, patch_ctx, &spec->sps, &spec->pps);
        HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);

        // 在获取参考队列之前必须先解析当前帧patch_header获取一部分patch_ctx信息
        sts = HLMD_DPB_GetRefPicLists(&spec->dpb_free_index, patch_ctx, spec->cur_frame, patch_ctx->ref_list, patch_ctx->ref_count);
        HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);

        // 配置寄存器
        HLMD_HLS_write_reg(spec, &bs, spec->pps.pic_type, &regs);
        if (spec->pps.pic_type != HLM_FRAME_TYPE_I)
        {
            HLMD_HLS_padding(spec, &regs);
        }

        // 启动解码器
        sts = HLMD_HLS_decode_frame(&regs);
        printf("decode poc %d, patch %d\n", spec->pps.poc, patch_ctx->patch_header.patch_idx);
        spec->coded_patch_cnt++;

        // 在每帧的最后一个patch解码之后，将重建图像输出给yuv_out，同时更新参考列表
        if (spec->coded_patch_cnt == spec->sps.patch_info.patch_num)
        {
            spec->coded_patch_cnt = 0;
            yuv_out->finish_one_frame = 1;

            // 从寄存器中获取解码结果
            HLMD_HLS_read_reg(yuv_out, &regs);

            // 更新参考帧列表
            sts = HLMD_DPB_UpdateRefList(patch_ctx->nal_ref_idc, spec->cur_frame, patch_ctx, &spec->poc);
            HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);

            // 释放无效参考帧
            sts = HLMD_DPB_ReleaseUnrefFrame(spec->dpb_handle, spec->ability.ref_frm_num + 1, spec->dec_pic_buf);
        }
        break;
    case HLM_SEI_NUT:
        sts = HLMD_HLS_parse_sei(&bs, spec->cur_frame, spec->pps.poc, yuv_out);
        HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);
        break;
    }

    return HLM_STS_OK;
}
