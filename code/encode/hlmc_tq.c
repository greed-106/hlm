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
#include "hlmc_tq.h"
#include "hlmc_common.h"

// 对残差系数进行像素域量化
HLM_VOID HLM_TQ_pixel_quant(HLM_COEFF  *src,
                            HLM_COEFF  *dst,
                            HLM_U32     blk_w,
                            HLM_U32     blk_h,
                            HLM_U32     src_stride,
                            HLM_U32     dst_stride,
                            HLM_U08     qp,
                            HLM_U08    *nonzero)
{
    HLM_S32 scale   = tbl_pixel_quant_scale[qp % 6];
    HLM_S32 shift   = HLM_QUANT_SHIFT + (qp / 6);
    HLM_S32 offset  = 1 << (shift - 1);
    HLM_S32 src_val = 0;
    HLM_S32 tmp     = 0;
    HLM_U32 x       = 0;
    HLM_U32 y       = 0;

    *nonzero = 0;

    for (y = 0; y < blk_h; y++)
    {
        for (x = 0; x < blk_w; x++)
        {
            src_val = src[y * src_stride + x];
            tmp = HLM_ABS(src_val);
            tmp = (tmp * scale + offset) >> shift;
            dst[y * dst_stride + x] = src_val < 0 ? (HLM_COEFF)(-tmp) : (HLM_COEFF)tmp;
            if (tmp)
            {
                *nonzero = 1;
            }
        }
    }
}

// 对残差系数进行频域量化
HLM_VOID HLMC_TQ_quant_4x4(HLM_COEFF    *src,
                           HLM_U08       qp,
                           HLM_U08       component,
                           HLM_S32      *mf,
                           HLM_S32       bias_f,
                           HLM_U32       blk_w,
                           HLM_U32       blk_h,
                           HLM_U32       src_stride,
                           HLM_U32       dst_stride,
                           HLM_U08      *nonzero,
                           HLM_COEFF    *dst)
{
    HLM_U08 j       = 0;
    HLM_U08 i       = 0;
    HLM_U08 q_bits  = 0;
    HLM_COEFF value = 0;

    *nonzero = 0;
    q_bits = HLMC_Q_BITS + qp / 6;

    for (j = 0; j < blk_h; j++)
    {
        for (i = 0; i < blk_w; i++)
        {
            value = (HLM_ABS(src[i]) * (HLM_S64)mf[i] + bias_f) >> q_bits;  //16bit情况下   HLM_ABS(src[i]) * mf[i]会超过32位。
            dst[i] = src[i] < 0 ? -value : value;
            if (value)
            {
                *nonzero = 1;
            }
        }
        mf += blk_w;
        src += src_stride;
        dst += dst_stride;
    }
}

// 对残差数据进行DCT
HLM_VOID HLMC_TQ_dct_4x4(HLM_COEFF       *src,
                         HLM_COEFF       *dst,
                         HLM_U32          blk_w,
                         HLM_U32          blk_h,
                         HLM_U32          src_stride,
                         HLM_U32          dst_stride,
                         HLM_U08          bitdepth)
{
    HLM_S32 tmp[16]    = { 0 };
    HLM_U32 i          = 0;
    HLM_S32 p0         = 0;
    HLM_S32 p1         = 0;
    HLM_S32 p2         = 0;
    HLM_S32 p3         = 0;
    HLM_S32 t0         = 0;
    HLM_S32 t1         = 0;
    HLM_S32 t2         = 0;
    HLM_S32 t3         = 0;
    HLM_COEFF *src_tmp = src;

    // 水平
    for (i = 0; i < blk_w; i++)
    {
        p0 = src_tmp[0];
        p1 = src_tmp[1];
        p2 = src_tmp[2];
        p3 = src_tmp[3];

        t0 = p0 + p3;
        t1 = p1 + p2;
        t2 = p1 - p2;
        t3 = p0 - p3;
        src_tmp += src_stride;

        tmp[0 + (i << 2)] = t0 + t1;
        tmp[1 + (i << 2)] = (t3 << 1) + t2;
        tmp[2 + (i << 2)] = t0 - t1;
        tmp[3 + (i << 2)] = t3 - (t2 << 1);
    }

    // 垂直
    for (i = 0; i < blk_h; i++)
    {
        p0 = tmp[(0 << 2) + i];
        p1 = tmp[(1 << 2) + i];
        p2 = tmp[(2 << 2) + i];
        p3 = tmp[(3 << 2) + i];

        t0 = p0 + p3;
        t1 = p1 + p2;
        t2 = p1 - p2;
        t3 = p0 - p3;

        dst[(0 * dst_stride) + i] = (HLM_COEFF)((t0 + t1));
        dst[(1 * dst_stride) + i] = (HLM_COEFF)((t2 + (t3 << 1)));
        dst[(2 * dst_stride) + i] = (HLM_COEFF)((t0 - t1));
        dst[(3 * dst_stride) + i] = (HLM_COEFF)((t3 - (t2 << 1)));
    }
}

// 对4x4子块进行变换量化
HLM_VOID HLMC_TQ_process_4x4(HLM_U32         raster_idx,
                             HLM_U08         bitdepth,
                             HLM_U32         comp,
                             HLMC_CU_INFO   *cur_cu)
{
    HLM_U08 j            = 0;
    HLM_U08 i            = 0;
    HLM_U08 qp           = 0;
    HLM_U08 nonzero      = 0;
    HLM_U08 shift        = bitdepth == 8 ? 6 : 5;
    HLM_U08 offset       = 1 << (shift - 1);
    HLM_U08 part_4x4_num = (1 << (cur_cu->com_cu_info.cu_width[comp] + cur_cu->com_cu_info.cu_height[comp] - 4));
    HLM_U08 pu_x         = raster_idx % 4;
    HLM_U08 pu_y         = raster_idx >> 2;
    HLM_U32 pu_w         = 1 << cur_cu->com_cu_info.cu_width[comp] >> 2;  // 变换跳过时，色度TB跟着亮度走
    HLM_U32 pu_h         = 1 << cur_cu->com_cu_info.cu_height[comp] >> 1;
    HLM_U32 pu_pos       = (pu_y * pu_h) * HLM_WIDTH_SIZE + (pu_x * pu_w);
    HLM_COEFF *src       = cur_cu->com_cu_info.cu_pred_info.coeff[comp] + pu_pos;
    HLM_COEFF *dst       = cur_cu->com_cu_info.cu_pred_info.res[comp] + pu_pos;

    if (!(cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_cu->com_cu_info.ts_flag == 1))
    {
        pu_w = 4;  // 做变换时，色度TB强制4x4
        pu_h = 4;
        HLM_COM_4x4_Block_To_Coeff_Pos(part_4x4_num, raster_idx, &pu_x, &pu_y, &cur_cu->com_cu_info, comp);
        pu_pos = (pu_y * pu_h) * HLM_WIDTH_SIZE + (pu_x * pu_w);
        src = cur_cu->com_cu_info.cu_pred_info.coeff[comp] + pu_pos;
        dst = cur_cu->com_cu_info.cu_pred_info.res[comp] + pu_pos;
    }
    qp = cur_cu->com_cu_info.qp[comp];

    // 变换量化
    if (cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_cu->com_cu_info.ts_flag == 1)
    {
        HLM_TQ_pixel_quant(dst, src, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, qp, &nonzero);
    }
    else
    {
        HLMC_TQ_dct_4x4(dst, dst, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, bitdepth);
        HLMC_TQ_quant_4x4(dst, qp, HLM_LUMA_Y,
            cur_cu->quant_params.quant_mf[qp % 6], cur_cu->quant_params.bias_f[comp],
            pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, &nonzero, src);
    }

    // 反量化反变换
    if (nonzero)
    {
        if (cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_cu->com_cu_info.ts_flag == 1)
        {
            HLM_COM_PixelDequant(src, dst, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, qp);
        }
        else
        {
            HLM_COM_Dequant4x4(src, qp, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE,
                cur_cu->quant_params.dequant_v[qp % 6], dst);
            HLM_COM_Idct4x4(dst, dst, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, bitdepth);
            for (i = 0; i < pu_h; i++)
            {
                for (j = 0; j < pu_w; j++)
                {
                    dst[j] = (dst[j] + offset) >> shift;
                }
                dst += HLM_WIDTH_SIZE;
            }
        }
    }
    else
    {
        for (i = 0; i < pu_h; i++)
        {
            for (j = 0; j < pu_w; j++)
            {
                dst[j] = 0;
            }
            dst += HLM_WIDTH_SIZE;
        }
    }
}

/***************************************************************************************************
* 功  能：根据qp，设置量化信息和lambda
* 参  数：*
*        regs                 -I    寄存器
*        cur_cu               -IO   当前CU信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_TQ_SetQuantInfo(HLMC_CU_INFO *cur_cu,
                              HLMC_REGS    *regs)
{
    HLM_U32 i                      = 0;
    HLM_U32 j                      = 0;
    HLM_U32 idx                    = 0;
    HLM_U08 q_bits[3]              = { 0 };
    HLM_U08 shift                  = regs->bitdepth == 8 ? 0 : 1;
    HLMC_QUANT_PARAMS *quant_param = &cur_cu->quant_params;

    // 量化相关参数初始化
    q_bits[0] = HLMC_Q_BITS + cur_cu->com_cu_info.qp[0] / 6;
    q_bits[1] = HLMC_Q_BITS + cur_cu->com_cu_info.qp[1] / 6;
    q_bits[2] = HLMC_Q_BITS + cur_cu->com_cu_info.qp[2] / 6;
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 16; j++)
        {
            idx = (j & 1) + ((j >> 2) & 1);
            quant_param->quant_mf[i][j] = HLMC_QUANT_MF[i][idx];
            quant_param->dequant_v[i][j] = (HLM_DEQUANT_V[i][idx] << (4 - shift));
        }
    }
    if (HLM_FRAME_TYPE_I == regs->enc_frame_coding_type)
    {
        quant_param->bias_f[0] = HLMC_DEFAULT_I_BIAS << (q_bits[0] - HLMC_BIAS_SHIFT_BITS);
        quant_param->bias_f[1] = HLMC_DEFAULT_I_BIAS << (q_bits[1] - HLMC_BIAS_SHIFT_BITS);
        quant_param->bias_f[2] = HLMC_DEFAULT_I_BIAS << (q_bits[2] - HLMC_BIAS_SHIFT_BITS);
    }
    else if (HLM_FRAME_TYPE_P == regs->enc_frame_coding_type)
    {
        quant_param->bias_f[0] = HLMC_DEFAULT_P_BIAS << (q_bits[0] - HLMC_BIAS_SHIFT_BITS);
        quant_param->bias_f[1] = HLMC_DEFAULT_P_BIAS << (q_bits[1] - HLMC_BIAS_SHIFT_BITS);
        quant_param->bias_f[2] = HLMC_DEFAULT_P_BIAS << (q_bits[2] - HLMC_BIAS_SHIFT_BITS);
    }
}

/***************************************************************************************************
* 功  能：变换量化
* 参  数：*
*        raster_idx           -I    当前TU的索引
*        channel_size         -I    当前通道的大小
*        bitdepth             -I    比特深度
*        yuv_comp             -I    yuv分量个数
*        cur_cu               -IO   当前CU信息
* 备  注：调用前对cu_type先赋值，i块赋值i_16x8，p块赋值p_16x8
***************************************************************************************************/
HLM_VOID HLMC_TQ_Process(HLM_U32         raster_idx,
                         HLM_U32         channel_size,
                         HLM_U08         bitdepth,
                         HLM_U32         yuv_comp,
                         HLMC_CU_INFO   *cur_cu)
{
    HLM_U08 cur_ts_flag      = cur_cu->com_cu_info.ts_flag;
    HLM_U08 part_4x4         = 0;
    HLM_U08 part_4x4_num     = 0;
    HLM_U08 i                = 0;
    HLM_U08 tmp_index        = 0;
    HLM_U08 pu8[8]           = { 0,1,4,5,2,3,6,7 };
    HLM_U08 chroma_4x4_block = cur_cu->com_cu_info.chroma_offset_y ? 1 : 4;

    if (4 == channel_size)
    {
        for (i = HLM_LUMA_Y; i < yuv_comp; i++)
        {
            if (i > 0 && cur_cu->com_cu_info.cu_width[1] != cur_cu->com_cu_info.cu_width[0]
                && cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_ts_flag == 0)
            {
                cur_cu->com_cu_info.ts_flag = 1;  // // 420/422的色度，IBC强制变换跳过
            }
            HLMC_TQ_process_4x4(raster_idx, bitdepth, i, cur_cu);
            cur_cu->com_cu_info.ts_flag = cur_ts_flag;
        }
    }
    else if (8 == channel_size)
    {
        for (i = HLM_LUMA_Y; i < yuv_comp; i++)
        {
            part_4x4_num = (i == HLM_LUMA_Y) ? 4 : chroma_4x4_block;
            for (part_4x4 = 0; part_4x4 < part_4x4_num; part_4x4++)
            {
                if (cur_cu->com_cu_info.chroma_offset_x + cur_cu->com_cu_info.chroma_offset_y != 0 && i != HLM_LUMA_Y)
                {
                    tmp_index = part_4x4 + raster_idx;
                }
                else
                {
                    tmp_index = pu8[part_4x4 + raster_idx * part_4x4_num];
                }
                HLMC_TQ_process_4x4(tmp_index, bitdepth, i, cur_cu);
            }
        }
    }
    else
    {
        for (i = HLM_LUMA_Y; i < yuv_comp; i++)
        {
            part_4x4_num = (1 << (cur_cu->com_cu_info.cu_width[i] + cur_cu->com_cu_info.cu_height[i] - 4));
            for (part_4x4 = 0; part_4x4 < part_4x4_num; part_4x4++)
            {
                HLMC_TQ_process_4x4(part_4x4, bitdepth, i, cur_cu);
            }
        }
    }
}
#if LINE_BY_LINE
/***************************************************************************************************
* 功  能：16x1的量化
* 参  数：*
*        regs                 -I    寄存器
*        cur_cu               -IO   当前CU信息
*        line_index           -I    当前TU的索引
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_INTRA_16x1_TQ_Process(HLMC_REGS              *regs, 
                                   HLMC_CU_INFO           *cur_cu, 
                                   HLM_U08                 line_index)
{
    HLM_U08 yuv_idx = 0;
    HLM_U16 *ori_pixel[3] = { 0 };
    HLM_U16 *rec_pixel[3] = { 0 };
    HLM_U08 nonzero = 0;
    HLM_U32 sse = 0;
    HLM_S32 i = 0;
    HLM_U32 distortion = 0;
    HLM_U08  bitdepth = regs->bitdepth;
    HLM_U08  shift = bitdepth == 8 ? 6 : 5;
    HLM_U08  offset = 1 << (shift - 1);
    HLM_U08 num_comp = (regs->image_format == HLM_IMG_YUV_400) ? 1 : 3;
    HLM_U32 pixel_x[3] = { cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2] };
    HLM_U32 pixel_y[3] = { cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2] };
    HLM_U32 stride_yuv[3] = { regs->enc_input_luma_stride ,regs->enc_input_chroma_stride,regs->enc_input_chroma_stride };
    ori_pixel[0] = regs->enc_input_y_base + stride_yuv[0] * (pixel_y[0] + line_index) + pixel_x[0];
    ori_pixel[1] = regs->enc_input_cb_base + stride_yuv[1] * (pixel_y[1] + line_index) + pixel_x[1];
    ori_pixel[2] = regs->enc_input_cr_base + stride_yuv[2] * (pixel_y[2] + line_index) + pixel_x[2];
    rec_pixel[0] = regs->enc_recon_y_base + stride_yuv[0] * (pixel_y[0] + line_index) + pixel_x[0];
    rec_pixel[1] = regs->enc_recon_cb_base + stride_yuv[1] * (pixel_y[1] + line_index) + pixel_x[1];
    rec_pixel[2] = regs->enc_recon_cr_base + stride_yuv[2] * (pixel_y[2] + line_index) + pixel_x[2];
    cur_cu->com_cu_info.ts_flag = 1;
    for (yuv_idx = 0; yuv_idx < num_comp; yuv_idx++)
    {
        HLM_U32  pu_pos = line_index * HLM_WIDTH_SIZE;
        HLM_U32 width = (1 << cur_cu->com_cu_info.cu_width[yuv_idx]);
        HLM_U32 height = (1 << cur_cu->com_cu_info.cu_height[yuv_idx]);
        HLM_COEFF *src = cur_cu->com_cu_info.cu_pred_info.coeff[yuv_idx] + pu_pos;
        HLM_COEFF *dst = cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos;
        
        if (line_index > height)
            continue;
        HLMC_COM_ComputeRes(ori_pixel[yuv_idx],
            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos,
            cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
            width,
            1,
            stride_yuv[yuv_idx],
            HLM_WIDTH_SIZE,
            HLM_WIDTH_SIZE);

        HLM_U08  qp = cur_cu->com_cu_info.qp[yuv_idx];
        HLM_TQ_pixel_quant(dst, src, width, 1, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, qp, &nonzero);
        if (nonzero)
        {
            HLM_COM_PixelDequant(src, dst, width, 1, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, qp);
        }
        else
        {
            memset(dst, 0, width * sizeof(HLM_COEFF));
        }

        HLM_COM_AddRes(cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos,
            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos,
            cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
            regs->bitdepth,
            width,
            1,
            HLM_WIDTH_SIZE,
            HLM_WIDTH_SIZE,
            HLM_WIDTH_SIZE);
        sse = HLMC_COM_ComputeSse(ori_pixel[yuv_idx],
            cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos,
            HLM_WIDTH_SIZE,
            1,
            stride_yuv[yuv_idx],
            HLM_WIDTH_SIZE);
        distortion += sse;
        memcpy(rec_pixel[yuv_idx], cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos, HLM_WIDTH_SIZE * sizeof(HLM_U16));
    }
    return distortion;
}
/***************************************************************************************************
* 功  能：1x8的量化
* 参  数：*
*        regs                 -I    寄存器
*        cur_cu               -IO   当前CU信息
*        line_index           -I    当前TU的索引
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_INTRA_1x8_TQ_Process(HLMC_REGS              *regs,
                                  HLMC_CU_INFO           *cur_cu, 
                                  HLM_U08                 line_index)
{
    HLM_U08 yuv_idx = 0;
    HLM_U08 j = 0;
    HLM_U32 sse = 0;
    HLM_U32 distortion = 0;
    HLM_U16 *ori_pixel[3] = { 0 };
    HLM_U16 *rec_pixel[3] = { 0 };
    HLM_U08 nonzero = 0;
    HLM_U08 num_comp = (regs->image_format == HLM_IMG_YUV_400) ? 1 : 3;
    HLM_U32 pixel_x[3] = { cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2] };
    HLM_U32 pixel_y[3] = { cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2] };
    HLM_U32 stride_yuv[3] = { regs->enc_input_luma_stride ,regs->enc_input_chroma_stride,regs->enc_input_chroma_stride };
    ori_pixel[0] = regs->enc_input_y_base + stride_yuv[0] * (pixel_y[0]) + pixel_x[0] + line_index;
    ori_pixel[1] = regs->enc_input_cb_base + stride_yuv[1] * (pixel_y[1]) + pixel_x[1] + line_index;
    ori_pixel[2] = regs->enc_input_cr_base + stride_yuv[2] * (pixel_y[2]) + pixel_x[2] + line_index;
    rec_pixel[0] = regs->enc_recon_y_base + stride_yuv[0] * pixel_y[0] + pixel_x[0] + line_index;
    rec_pixel[1] = regs->enc_recon_cb_base + stride_yuv[1] * pixel_y[1] + pixel_x[1] + line_index;
    rec_pixel[2] = regs->enc_recon_cr_base + stride_yuv[2] * pixel_y[2] + pixel_x[2] + line_index;
    for (yuv_idx = 0; yuv_idx < num_comp; yuv_idx++)
    {
        HLM_U32  pu_pos = line_index;
        HLM_U32 width = (1 << cur_cu->com_cu_info.cu_width[yuv_idx]);
        HLM_U32 height = (1 << cur_cu->com_cu_info.cu_height[yuv_idx]);
        HLM_COEFF *src = cur_cu->com_cu_info.cu_pred_info.coeff[yuv_idx] + pu_pos;
        HLM_COEFF *dst = cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos;
        HLM_U08  qp = cur_cu->com_cu_info.qp[yuv_idx];
        HLM_COEFF tmp[8] = { 0 };
        if (line_index > width)
            continue;
        HLMC_COM_ComputeRes(ori_pixel[yuv_idx],
            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos,
            cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
            1,
            height,
            stride_yuv[yuv_idx],
            HLM_WIDTH_SIZE,
            HLM_WIDTH_SIZE);
        cur_cu->com_cu_info.ts_flag = 1;
        
        HLM_TQ_pixel_quant(dst, src, 1, height, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, qp, &nonzero);
        HLM_COM_PixelDequant(src, dst, 1, height, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, qp);
        

        HLM_COM_AddRes(cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos,
            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos,
            cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
            regs->bitdepth,
            1,
            height,
            HLM_WIDTH_SIZE,
            HLM_WIDTH_SIZE,
            HLM_WIDTH_SIZE);
        sse = HLMC_COM_ComputeSse(ori_pixel[yuv_idx],
            cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos,
            1,
            height,
            stride_yuv[yuv_idx],
            HLM_WIDTH_SIZE);
        distortion += sse;
        for (j = 0; j < height; j++)
        {
            rec_pixel[yuv_idx][(j *  stride_yuv[yuv_idx])] = cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx][pu_pos + (j * HLM_WIDTH_SIZE)];
        }
    }
    return distortion;

}
#endif
