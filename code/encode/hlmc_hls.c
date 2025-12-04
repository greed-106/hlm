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
#include "hlmc_hls.h"
#include "hlmc_ecd.h"

#define QPFACTOR_FIX_POINT         (14)        // qpFactor定点化的放大倍数
#define LAMBDA_FIX_POINT           (10)        // lambdaSadTbl和lambdaSseTbl定点化的放大倍数

// lambdaSad = qpFactorSad * 2^( (qp-12)/6 )
static const HLM_U64 lambdaSadTbl[100] =
{
    0x0000000100, 0x000000011f, 0x0000000143, 0x000000016a, 0x0000000196, 0x00000001c8,
    0x0000000200, 0x000000023f, 0x0000000285, 0x00000002d4, 0x000000032d, 0x0000000390,
    0x0000000400, 0x000000047d, 0x000000050a, 0x00000005a8, 0x0000000659, 0x0000000721,
    0x0000000800, 0x00000008fb, 0x0000000a14, 0x0000000b50, 0x0000000cb3, 0x0000000e41,
    0x0000001000, 0x00000011f6, 0x0000001429, 0x00000016a1, 0x0000001966, 0x0000001c82,
    0x0000002000, 0x00000023eb, 0x0000002851, 0x0000002d41, 0x00000032cc, 0x0000003904,
    0x0000004000, 0x00000047d6, 0x00000050a3, 0x0000005a82, 0x0000006598, 0x0000007209,
    0x0000008000, 0x0000008fad, 0x000000a145, 0x000000b505, 0x000000cb30, 0x000000e412,
    0x0000010000, 0x0000011f5a, 0x000001428a, 0x0000016a0a, 0x0000019660, 0x000001c824,
    0x0000020000, 0x0000023eb3, 0x0000028514, 0x000002d414, 0x0000032cc0, 0x0000039048,
    0x0000040000, 0x0000047d67, 0x0000050a29, 0x000005a828, 0x0000065980, 0x0000072090,
    0x0000080000, 0x000008facd, 0x00000a1451, 0x00000b504f, 0x00000cb2ff, 0x00000e411f,
    0x0000100000, 0x000011f59b, 0x00001428a3, 0x000016a09e, 0x00001965ff, 0x00001c823e,
    0x0000200000, 0x000023eb36, 0x0000285146, 0x00002d413d, 0x000032cbfd, 0x000039047c,
    0x0000400000, 0x000047d66b, 0x000050a28c, 0x00005a827a, 0x00006597fb, 0x00007208f8,
    0x0000800000, 0x00008facd6, 0x0000a14518, 0x0000b504f3, 0x0000cb2ff5, 0x0000e411f0,
    0x0001000000, 0x00011f59ac, 0x0001428a30, 0x00016a09e6
};

// lambdaSse = qpFactorSse * 2^( (qp-12)/3 )
static const HLM_U64 lambdaSseTbl[100] =
{
    0x0000000040, 0x0000000051, 0x0000000066, 0x0000000080, 0x00000000a1, 0x00000000cb,
    0x0000000100, 0x0000000143, 0x0000000196, 0x0000000200, 0x0000000285, 0x000000032d,
    0x0000000400, 0x000000050a, 0x0000000659, 0x0000000800, 0x0000000a14, 0x0000000cb3,
    0x0000001000, 0x0000001429, 0x0000001966, 0x0000002000, 0x0000002851, 0x00000032cc,
    0x0000004000, 0x00000050a3, 0x0000006598, 0x0000008000, 0x000000a145, 0x000000cb30,
    0x0000010000, 0x000001428a, 0x0000019660, 0x0000020000, 0x0000028514, 0x0000032cc0,
    0x0000040000, 0x0000050a29, 0x0000065980, 0x0000080000, 0x00000a1451, 0x00000cb2ff,
    0x0000100000, 0x00001428a3, 0x00001965ff, 0x0000200000, 0x0000285146, 0x000032cbfd,
    0x0000400000, 0x000050a28c, 0x00006597fb, 0x0000800000, 0x0000a14518, 0x0000cb2ff5,
    0x0001000000, 0x0001428a30, 0x0001965fea, 0x0002000000, 0x000285145f, 0x00032cbfd5,
    0x0004000000, 0x00050a28be, 0x0006597fa9, 0x0008000000, 0x000a14517d, 0x000cb2ff53,
    0x0010000000, 0x001428a2fa, 0x001965fea5, 0x0020000000, 0x00285145f3, 0x0032cbfd4a,
    0x0040000000, 0x0050a28be6, 0x006597fa95, 0x0080000000, 0x00a14517cc, 0x00cb2ff52a,
    0x0100000000, 0x01428a2f99, 0x01965fea54, 0x0200000000, 0x0285145f32, 0x032cbfd4a8,
    0x0400000000, 0x050a28be63, 0x06597fa94f, 0x0800000000, 0x0a14517cc7, 0x0cb2ff529f,
    0x1000000000, 0x1428a2f98d, 0x1965fea53d, 0x2000000000, 0x285145f31b, 0x32cbfd4a7b,
    0x4000000000, 0x50a28be636, 0x6597fa94f6, 0x8000000000
};

// 设置lambda
HLM_VOID HLMC_HLS_set_lamda(HLM_U32  qp,
                            HLM_U32 *lamda_sad,
                            HLM_U32 *lamda_sse,
                            HLM_U64  qpFactorSad,
                            HLM_U64  qpFactorSse)
{
    HLM_S32 shift  = QPFACTOR_FIX_POINT + LAMBDA_FIX_POINT - HLMC_LAMBDA_SHIFT;
    HLM_S32 offset = 1 << (shift - 1);
    HLM_U64 lambda = 0;

    lambda = (qpFactorSad * lambdaSadTbl[qp] + offset) >> shift;
    *lamda_sad = (HLM_U32)HLM_MIN(lambda, HLM_MAX_U32);

    lambda = (qpFactorSse * lambdaSseTbl[qp] + offset) >> shift;
    *lamda_sse = (HLM_U32)HLM_MIN(lambda, HLM_MAX_U32);  // 当qp>=93时会超出U32
}

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
                             VENC_RATE_CTRL_OUT_REGS      *rc_regs)
{
    HLMC_DPB_SPEC    *dpb_spec    = (HLMC_DPB_SPEC *)(spec->dpb_handle);
    HLMC_BITSTREAM   *bs          = &(spec->bs);
    HLMC_BITSTREAM   *out_bs      = &(spec->out_bs);
    HLM_STATUS        sts         = HLM_STS_OK;
    HLMC_FRAME       *rec_frame   = &dpb_spec->dpb[rec_dpb_idx];
    HLMC_FRAME       *ref_frame   = &dpb_spec->dpb[ref_dpb_idx];
    HLM_F64           dQPFactor   = 0.760263112;  // sqrt(0.15);
    HLM_U32           qpFactorSad = (HLM_U32)((dQPFactor * (1 << QPFACTOR_FIX_POINT)) + 0.5f);
    HLM_U32           qpFactorSse = (HLM_U32)((dQPFactor * dQPFactor * (1 << QPFACTOR_FIX_POINT)) + 0.5f);
    HLM_S32           qp          = 0;
    HLM_S32           max_qp      = HLM_MAX_QP(input->image_in.bitdepth);

    regs->enc_frame_coding_type        = patch_type == HLMC_BASE_IDRPATCH ? HLM_FRAME_TYPE_I : HLM_FRAME_TYPE_P;
    regs->enc_output_strm_base         = bs->ptr;
    regs->enc_output_strm              = out_bs->ptr;
    regs->enc_output_strm_buffer_limit = (bs->bit_size - bs->bit_cnt) >> 3;
    regs->enc_real_input_y_base        = (HLM_U16 *)(input->image_in.data[0]);
    regs->enc_real_input_cb_base       = (HLM_U16 *)(input->image_in.data[1]);
    regs->enc_real_input_cr_base       = (HLM_U16 *)(input->image_in.data[2]);
    regs->enc_real_rec_y_base          = (HLM_U16 *)(rec_frame->data[0]);
    regs->enc_real_rec_cb_base         = (HLM_U16 *)(rec_frame->data[1]);
    regs->enc_real_rec_cr_base         = (HLM_U16 *)(rec_frame->data[2]);
    regs->enc_ref_y_base               = (HLM_U16 *)(ref_frame->data[0]);
    regs->enc_ref_cb_base              = (HLM_U16 *)(ref_frame->data[1]);
    regs->enc_ref_cr_base              = (HLM_U16 *)(ref_frame->data[2]);
    regs->enc_ref_y_padding_base       = ref_frame->luma_ref_padding_y;
    regs->enc_ref_cb_padding_base      = ref_frame->luma_ref_padding_cb;
    regs->enc_ref_cr_padding_base      = ref_frame->luma_ref_padding_cr;
    regs->enc_real_input_luma_stride   = input->image_in.step[0];
    regs->enc_real_input_chroma_stride = input->image_in.step[1];
    regs->mv_ref_cross_patch           = spec->sps.mv_ref_cross_patch;
    regs->inter_pad_w_left             = spec->sps.mv_search_width >> 1;
    regs->inter_pad_w_right            = spec->sps.mv_search_width - 1 - regs->inter_pad_w_left;
    regs->inter_pad_h_up               = spec->sps.mv_search_height >> 1;
    regs->inter_pad_h_down             = spec->sps.mv_search_height - 1 - regs->inter_pad_h_up;
    regs->enc_ref_frame_luma_stride    = input->image_in.step[0] + regs->inter_pad_w_left + regs->inter_pad_w_right;
    regs->enc_ref_frame_chroma_stride  = input->image_in.step[1] + regs->inter_pad_w_left + regs->inter_pad_w_right;
    regs->enc_real_pic_width[0]        = input->image_in.width[0];
    regs->enc_real_pic_height[0]       = input->image_in.height[0];
    regs->enc_real_pic_width[1]        = input->image_in.width[1];
    regs->enc_real_pic_height[1]       = input->image_in.height[1];
    regs->image_format                 = spec->sps.format;
    regs->intra_8x8_enable_flag        = spec->sps.intra_8x8_enable_flag;
    regs->cu_delta_qp_enable_flag      = spec->sps.cu_delta_qp_enable_flag;
    regs->i_frame_enable_ibc           = spec->sps.i_frame_enable_ibc;
    regs->p_frame_enable_ibc           = spec->sps.p_frame_enable_ibc;
    regs->bitdepth                     = input->image_in.bitdepth;
    regs->enc_poc                      = spec->poc;
    regs->enc_pic_init_qp              = rc_regs->reg_patch_qp;
    regs->enc_pic_qp                   = rc_regs->reg_patch_qp;
    regs->enc_pic_target_bits          = rc_regs->enc_pic_target_bits;
    regs->segment_enable_flag          = spec->coding_ctrl.segment_enable_flag;
    regs->segment_width_in_log2        = spec->coding_ctrl.segment_width_in_log2;
    regs->segment_height_in_log2       = spec->coding_ctrl.segment_height_in_log2;
    regs->enc_qp_min                   = rc_regs->reg_qpmin;
    regs->enc_qp_max                   = rc_regs->reg_qpmax;
    regs->enc_ram_buf                  = spec->ram_buf;
    regs->enc_ram_len                  = spec->ram_size;
    regs->enc_i_ave_row_qp             = 0;
    regs->enc_p_ave_row_qp             = 0;

    for (qp = 0; qp <= max_qp; qp++)
    {
        HLMC_HLS_set_lamda(qp, regs->enc_satd_lamda + qp, regs->enc_sse_lamda + qp, qpFactorSad, qpFactorSse);
    }
#if WRITE_PARAMETERS
    regs->fp_param                     = input->fp_param;
#endif

    return sts;
}

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
                          HLM_U32           sps_pps_bits)
{
    HLM_S32 code_width    = HLM_SIZE_ALIGN_16(spec->coding_ctrl.width);
    HLM_S32 code_height   = HLM_SIZE_ALIGN_8(spec->coding_ctrl.height);
    HLM_S32 frame_size[2] = { 0 };

    frame_size[0] = (code_width * code_height);
    switch (spec->coding_ctrl.img_format)
    {
    case HLM_IMG_YUV_444:
    case HLM_IMG_RGB:
        frame_size[1] = frame_size[0];
        break;
    case HLM_IMG_YUV_422:
        frame_size[1] = frame_size[0] >> 1;
        break;
    case HLM_IMG_YUV_420:
        frame_size[1] = frame_size[0] >> 2;
        break;
    case HLM_IMG_YUV_400:
        frame_size[1] = 0;
        break;
    default:
        printf("Not Support This Format");
        break;
    }

    output->stream_len   = (sps_pps_bits >> 3) + spec->total_bs_len;
    output->rec_stride_y = regs->enc_real_input_luma_stride;
    output->rec_stride_c = regs->enc_real_input_chroma_stride;
    memcpy(output->recon_dbk_y, regs->enc_real_rec_y_base,  frame_size[0] * sizeof(HLM_U16));
    memcpy(output->recon_dbk_u, regs->enc_real_rec_cb_base, frame_size[1] * sizeof(HLM_U16));
    memcpy(output->recon_dbk_v, regs->enc_real_rec_cr_base, frame_size[1] * sizeof(HLM_U16));
}

/***************************************************************************************************
* 功  能：初始化编码库spec
* 参  数：*
*         regs         -I    寄存器参数结构体
*         spec         -O    硬件抽象层工作参数集
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_InitSpec(HLMC_REGS *regs,
                           HLM_SPEC  *spec)
{
    HLMC_BITSTREAM *bs        = &(spec->bs);
    HLMC_BITSTREAM *out_bs    = &(spec->out_bs);
    HLM_RAM_BUF    *ram_buf   = &(spec->ram_buf_pic);

    spec->regs                = regs;
    spec->cu_rows             = HLM_SIZE_ALIGN(regs->enc_real_pic_height[0], 1 << HLM_LOG2_HEIGHT_SIZE) >> HLM_LOG2_HEIGHT_SIZE;
    spec->cu_cols             = HLM_SIZE_ALIGN(regs->enc_real_pic_width[0],  1 << HLM_LOG2_WIDTH_SIZE ) >> HLM_LOG2_WIDTH_SIZE;
    spec->cu_total            = spec->cu_cols * spec->cu_rows;
    spec->cu_cnt              = 0;
    spec->total_bs_len        = 0;
    spec->patch_ctx.patch_idx = 0;  // 用于遍历所有patch

    bs->bits_left             = 32;
    bs->byte_cache            = 0;
    bs->bit_size              = (regs->enc_output_strm_buffer_limit << 3);
    bs->bit_cnt               = 0;
    bs->emul_bytes            = 0;
    bs->ptr_start             = bs->ptr = regs->enc_output_strm_base;
    out_bs->ptr_start         = out_bs->ptr = regs->enc_output_strm;
    out_bs->bit_size          = (regs->enc_output_strm_buffer_limit << 3);

    ram_buf->start            = regs->enc_ram_buf;
    ram_buf->end              = regs->enc_ram_buf + regs->enc_ram_len;
    ram_buf->cur_pos          = regs->enc_ram_buf;
#if WRITE_PARAMETERS
    spec->fp_param            = regs->fp_param;
#endif
}

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
                             HLMC_REGS           *regs)
{
    HLM_U32 i          = 0;
    HLM_U32 j          = 0;
    HLM_U32 cu_width   = regs->enc_real_pic_width[0] >> HLM_LOG2_WIDTH_SIZE;
    HLM_U32 patch_size = regs->enc_real_pic_width[0] * regs->enc_real_pic_height[0];
    HLM_U32 yuv_comp   = (regs->image_format== HLM_IMG_YUV_400) ? 1 : 3;

    for (i = 0; i < yuv_comp; i++)
    {
        cur_cu->com_cu_info.cu_pred_info.pred[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.pred[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.rec[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.rec[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.res[i] = (HLM_COEFF *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.res[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.coeff[i] = (HLM_COEFF *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.coeff[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.skip_pred[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.skip_pred[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.inter_pred[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.inter_pred[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.direct_rec[i] = (HLM_U16*)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.direct_rec[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.direct_res[i] = (HLM_COEFF*)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.direct_res[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.direct_coeff[i] = (HLM_COEFF*)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.direct_coeff[i], HLM_STS_ERR_MEM_LACK);

        best_cu->com_cu_info.cu_pred_info.inter_pred[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == best_cu->com_cu_info.cu_pred_info.inter_pred[i], HLM_STS_ERR_MEM_LACK);
        best_cu->com_cu_info.cu_pred_info.pred[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == best_cu->com_cu_info.cu_pred_info.pred[i], HLM_STS_ERR_MEM_LACK);
        best_cu->com_cu_info.cu_pred_info.rec[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == best_cu->com_cu_info.cu_pred_info.rec[i], HLM_STS_ERR_MEM_LACK);
        best_cu->com_cu_info.cu_pred_info.res[i] = (HLM_COEFF *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == best_cu->com_cu_info.cu_pred_info.res[i], HLM_STS_ERR_MEM_LACK);
        best_cu->com_cu_info.cu_pred_info.coeff[i] = (HLM_COEFF *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == best_cu->com_cu_info.cu_pred_info.coeff[i], HLM_STS_ERR_MEM_LACK);
        best_cu->com_cu_info.cu_pred_info.skip_pred[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == best_cu->com_cu_info.cu_pred_info.skip_pred[i], HLM_STS_ERR_MEM_LACK);
        best_cu->com_cu_info.cu_pred_info.direct_rec[i] = (HLM_U16*)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == best_cu->com_cu_info.cu_pred_info.direct_rec[i], HLM_STS_ERR_MEM_LACK);
        best_cu->com_cu_info.cu_pred_info.direct_res[i] = (HLM_COEFF*)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == best_cu->com_cu_info.cu_pred_info.direct_res[i], HLM_STS_ERR_MEM_LACK);
        best_cu->com_cu_info.cu_pred_info.direct_coeff[i] = (HLM_COEFF*)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == best_cu->com_cu_info.cu_pred_info.direct_coeff[i], HLM_STS_ERR_MEM_LACK);
    }

    // nbi_info
    nbi_info->intra_rec_up_y = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, (cu_width << HLM_LOG2_WIDTH_SIZE) * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->intra_rec_up_y, HLM_STS_ERR_MEM_LACK);
    nbi_info->intra_rec_up_u = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, (cu_width << HLM_LOG2_WIDTH_SIZE) * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->intra_rec_up_u, HLM_STS_ERR_MEM_LACK);
    nbi_info->intra_rec_up_v = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, (cu_width << HLM_LOG2_WIDTH_SIZE) * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->intra_rec_up_v, HLM_STS_ERR_MEM_LACK);
    nbi_info->intra_pred_mode_up = (HLM_U08 *)HLM_MEM_Calloc(ram_buf, cu_width << 2, 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->intra_pred_mode_up, HLM_STS_ERR_MEM_LACK);
    nbi_info->inter_mv_up = (HLM_MV *)HLM_MEM_Calloc(ram_buf, sizeof(HLM_MV)*(cu_width << 2), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->inter_mv_up, HLM_STS_ERR_MEM_LACK);
    nbi_info->pred_type_up = (HLM_CU_TYPE *)HLM_MEM_Calloc(ram_buf, sizeof(HLM_CU_TYPE)* (cu_width << 2), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->pred_type_up, HLM_STS_ERR_MEM_LACK);

    //patch内部分配
    regs->enc_input_y_base   = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, patch_size * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == regs->enc_input_y_base, HLM_STS_ERR_MEM_LACK);
    regs->enc_input_cb_base  = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, patch_size * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == regs->enc_input_cb_base, HLM_STS_ERR_MEM_LACK);
    regs->enc_input_cr_base  = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, patch_size * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == regs->enc_input_cr_base, HLM_STS_ERR_MEM_LACK);
    regs->enc_recon_y_base   = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, patch_size * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == regs->enc_recon_y_base, HLM_STS_ERR_MEM_LACK);
    regs->enc_recon_cb_base  = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, patch_size * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == regs->enc_recon_cb_base, HLM_STS_ERR_MEM_LACK);
    regs->enc_recon_cr_base  = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, patch_size * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == regs->enc_recon_cr_base, HLM_STS_ERR_MEM_LACK);

    return HLM_STS_OK;
}

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
                                HLMC_RATE_CTRL   *rate_ctrl)
{
    HLM_U32 width             = 0;
    HLM_U32 height            = 0;
    HLM_S32 patch_width       = 0;
    HLM_S32 patch_height      = 0;
    HLM_S32 patch_num_in_cols = 0;
    HLM_S32 patch_num_in_rows = 0;
    HLM_S32 i                 = 0;

    sps->profile                        = coding_ctrl->profile;
    width                               = 16 * ((coding_ctrl->width + 15) / 16);
    height                              = 8 * ((coding_ctrl->height + 7) / 8);
    sps->pic_width_in_cus_minus1        = width / 16 - 1;
    sps->pic_height_in_map_units_minus1 = height / 8 - 1;
    sps->bitdepth                       = coding_ctrl->bitdepth;
    sps->format                         = coding_ctrl->img_format;
    sps->intra_8x8_enable_flag          = coding_ctrl->intra_8x8_enable_flag;
    if (sps->format == HLM_IMG_YUV_420 || sps->format == HLM_IMG_YUV_422)
    {
        sps->intra_8x8_enable_flag = 1;  // 420和422强制intra8x8
    }
    if (coding_ctrl->uniform_patch_split)
    {
        patch_width               = HLM_CLIP(coding_ctrl->patch_info.patch_param[0].patch_width[0],  1, (HLM_S32)coding_ctrl->width);
        patch_height              = HLM_CLIP(coding_ctrl->patch_info.patch_param[0].patch_height[0], 1, (HLM_S32)coding_ctrl->height);
        patch_num_in_cols         = (coding_ctrl->width + patch_width - 1) / patch_width;
        patch_num_in_rows         = (coding_ctrl->height + patch_height - 1) / patch_height;
        sps->patch_info.patch_num = patch_num_in_cols * patch_num_in_rows;
        assert(0 < sps->patch_info.patch_num && sps->patch_info.patch_num < HLM_MAX_PATCH_NUM);
        for (i = 0; i < sps->patch_info.patch_num; i++)
        {
            sps->patch_info.patch_param[i].patch_x               = (i % patch_num_in_cols) * patch_width;
            sps->patch_info.patch_param[i].patch_y               = (i / patch_num_in_cols) * patch_height;
            sps->patch_info.patch_param[i].patch_width[0]        = patch_width;
            sps->patch_info.patch_param[i].patch_height[0]       = patch_height;
            sps->patch_info.patch_param[i].patch_coded_width[0]  = HLM_SIZE_ALIGN_16(patch_width);
            sps->patch_info.patch_param[i].patch_coded_height[0] = HLM_SIZE_ALIGN_8(patch_height);
        }
    }
    else
    {
        memcpy(&sps->patch_info, &coding_ctrl->patch_info, sizeof(HLM_PATCH_INFO));
    }

    // 赋值色度分量
    for (i = 0; i < sps->patch_info.patch_num; i++)
    {
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

    sps->bpp_i                   = rate_ctrl->rc_cbr_ctrl.bpp_i;
    sps->bpp_p                   = rate_ctrl->rc_cbr_ctrl.bpp_p;
    sps->cu_delta_qp_enable_flag = (rate_ctrl->rate_ctrl_mode == HLMC_RC_FIXQP) ? 0 : 1;
    sps->i_frame_enable_ibc      = coding_ctrl->i_frame_enable_ibc;
    sps->p_frame_enable_ibc      = coding_ctrl->p_frame_enable_ibc;
    sps->mv_ref_cross_patch      = coding_ctrl->mv_ref_cross_patch;
    sps->mv_limit_enable_flag    = 1;
    sps->mv_search_width         = coding_ctrl->mv_search_width;
    sps->mv_search_height        = coding_ctrl->mv_search_height;
    if (width != coding_ctrl->width || height != coding_ctrl->height)
    {
        sps->frame_cropping_flag      = 1;
        sps->frame_crop_left_offset   = 0;
        sps->frame_crop_right_offset  = width - coding_ctrl->width;
        sps->frame_crop_top_offset    = 0;
        sps->frame_crop_bottom_offset = height - coding_ctrl->height;
    }
    else
    {
        sps->frame_cropping_flag      = 0;
        sps->frame_crop_left_offset   = 0;
        sps->frame_crop_right_offset  = 0;
        sps->frame_crop_top_offset    = 0;
        sps->frame_crop_bottom_offset = 0;
    }
}

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
                                HLM_PARAM_PPS        *pps)
{
    pps->pic_type            = frame_type == HLMC_BASE_IDRPATCH ? HLM_FRAME_TYPE_I : HLM_FRAME_TYPE_P;
    pps->poc                 = poc;
    pps->pic_luma_qp         = pic_luma_qp;
    pps->pic_chroma_delta_qp = coding_ctrl->chroma_qp_offset;
}

/***************************************************************************************************
* 功  能：生成SPS参数集
* 参  数：*
*         sps              -I         SPS结构体
*         bs               -I         码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_GeneratSeqHeader(HLM_PARAM_SPS       *sps,
                                   HLMC_BITSTREAM      *bs)
{
    HLM_S32 i                   = 0;
    HLM_S32 seq_horizontal_size = ((sps->pic_width_in_cus_minus1 + 1) << 4)
                                - sps->frame_crop_left_offset - sps->frame_crop_right_offset;
    HLM_S32 seq_vertical_size   = ((sps->pic_height_in_map_units_minus1 + 1) << 3)
                                - sps->frame_crop_top_offset - sps->frame_crop_bottom_offset;

    HLMC_ECD_PutStartCode4Byte(bs);
    HLMC_ECD_PutNaluHeader(HLM_SPS_NUT, 1, bs);

    HLMC_ECD_PutShortBits(bs, sps->profile,                  8, "profile_id");
    HLMC_ECD_PutLongBits( bs, seq_horizontal_size,          32, "seq_horizontal_size");
    HLMC_ECD_PutLongBits( bs, seq_vertical_size,            32, "seq_vertical_size");
    HLMC_ECD_PutShortBits(bs, sps->bitdepth - 8,             4, "seq_bit_depth_luma_minus8");
    HLMC_ECD_PutShortBits(bs, sps->bitdepth - 8,             4, "seq_bit_depth_chroma_minus8");
    HLMC_ECD_PutLongBits( bs, sps->bpp_i,                   12, "seq_bpp_i_picture");
    HLMC_ECD_PutLongBits( bs, sps->bpp_p,                   12, "seq_bpp_p_picture");
    HLMC_ECD_PutShortBits(bs, sps->format,                   3, "seq_chroma_format");
    HLMC_ECD_PutShortBits(bs, sps->i_frame_enable_ibc,       1, "seq_i_picture_ibc_enable_flag");
    HLMC_ECD_PutShortBits(bs, sps->p_frame_enable_ibc,       1, "seq_p_picture_ibc_enable_flag");
    HLMC_ECD_PutShortBits(bs, sps->mv_ref_cross_patch,       1, "seq_mv_cross_patch_enable_flag");
    HLMC_ECD_PutShortBits(bs, sps->intra_8x8_enable_flag,    1, "seq_intra_8x8_enable_flag");
    HLMC_ECD_PutShortBits(bs, sps->cu_delta_qp_enable_flag,  1, "seq_cu_delta_qp_enable_flag");
    HLMC_ECD_PutShortBits(bs, sps->mv_limit_enable_flag,     1, "seq_mv_search_range_limit_enable_flag");
    if (sps->mv_limit_enable_flag)
    {
        HLMC_ECD_PutShortBits(bs, sps->mv_search_width,      8, "seq_mv_search_range_width");
        HLMC_ECD_PutShortBits(bs, sps->mv_search_height,     8, "seq_mv_search_range_height");
    }
    HLMC_ECD_PutShortBits(bs, sps->patch_info.patch_num - 1, 8, "seq_patch_num_minus1");
    for (i = 0; i < sps->patch_info.patch_num; i++)
    {
        HLMC_ECD_PutLongBits(bs, sps->patch_info.patch_param[i].patch_x,         32, "seq_patch_x");
        HLMC_ECD_PutLongBits(bs, sps->patch_info.patch_param[i].patch_y,         32, "seq_patch_y");
        HLMC_ECD_PutLongBits(bs, sps->patch_info.patch_param[i].patch_width[0],  32, "seq_patch_width");
        HLMC_ECD_PutLongBits(bs, sps->patch_info.patch_param[i].patch_height[0], 32, "seq_patch_height");
    }

    HLMC_ECD_RbspTrailingBits(bs);
}

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
                                   HLMC_BITSTREAM      *bs)
{
    HLMC_ECD_PutStartCode4Byte(bs);
    HLMC_ECD_PutNaluHeader(HLM_PPS_NUT, 1, bs);

    HLMC_ECD_PutShortBits(bs, pps->pic_type == HLM_FRAME_TYPE_I, 1, "pic_type");
    HLMC_ECD_PutLongBits( bs, pps->poc, 16, "pic_poc");
    if (!sps->cu_delta_qp_enable_flag)
    {
        HLMC_ECD_PutUeBits(bs, pps->pic_luma_qp, 1, "pic_luma_qp");
        HLMC_ECD_PutSeBits(bs, pps->pic_chroma_delta_qp, 1, "pic_chroma_delta_qp");
    }

    HLMC_ECD_RbspTrailingBits(bs);
}

/***************************************************************************************************
* 功  能：生成SEI参数集
* 参  数：*
*         regs             -I         寄存器
*         bs               -IO        码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_HLS_GeneratSEI(HLMC_REGS        *regs,
                             HLMC_BITSTREAM   *bs)
{
    HLM_U08 cur_img_md5[16] = { 0 };
    HLM_PARAM_SEI psei      = { 0 };
    HLM_S32 tmp             = 0;
    HLM_U32 i               = 0;
    HLM_U32 payload_type    = 0;
    HLM_U32 write_flag      = 0;

#if START_CODE_FIX
    HLMC_ECD_PutStartCode3Byte(bs);
#else
    HLMC_ECD_PutStartCode4Byte(bs);
#endif
    HLMC_ECD_PutNaluHeader(HLM_SEI_NUT, 1, bs);

    for (payload_type = 0; payload_type < SEI_PAYLOAD_TYPE_NUM; payload_type++)
    {
        write_flag = 0;
        if (payload_type == USER_DATA_MD5)
        {
            HLM_COM_GetMd5(regs->enc_real_rec_y_base, regs->enc_real_rec_cb_base, regs->enc_real_rec_cr_base,
                cur_img_md5, regs->enc_real_pic_width[0], regs->enc_real_pic_height[0],
                regs->enc_real_pic_width[1], regs->enc_real_pic_height[1]);
            psei.payload_type = payload_type;
            psei.payload_size = 16;
            psei.payload_data = cur_img_md5;
            write_flag = 1;
        }

        if (write_flag)
        {
            tmp = psei.payload_type;
            while (tmp >= 255)
            {
                HLMC_ECD_PutShortBits(bs, 0xff, 8, "ff_byte");
                tmp -= 255;
            }
            HLMC_ECD_PutShortBits(bs, tmp, 8, "payload_type");

            tmp = psei.payload_size;
            while (tmp >= 255)
            {
                HLMC_ECD_PutShortBits(bs, 0xff, 8, "ff_byte");
                tmp -= 255;
            }
            HLMC_ECD_PutShortBits(bs, tmp, 8, "payload_size");

            for (i = 0; i < psei.payload_size; i++)
            {
                HLMC_ECD_PutShortBits(bs, psei.payload_data[i], 8, "payload_data");
            }
        }
    }

    HLMC_ECD_RbspTrailingBits(bs);
}
