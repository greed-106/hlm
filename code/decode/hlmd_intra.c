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
#include "hlmd_intra.h"
#include "hlmd_tq.h"

#if LINE_BY_LINE
static HLM_VOID HLMD_INTRA_get_ref_pixel_16x1(HLM_NEIGHBOR_INFO     *nbi_info,
                                              HLMD_CU_INFO           *cur_cu,
                                              HLM_U08                  pred_mode,
                                              HLM_U08                  yuv_idx,
                                              HLM_U16                 *ref_pixel)
{
    HLM_U32 pixel_x[3] = { cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2] };
    HLM_U32 pixel_y[3] = { cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2] };
}
#endif

/***************************************************************************************************
* 功  能：帧内预测主函数
* 参  数：*
*        regs                     -I    硬件寄存器
*        nbi_info                 -I    当前cu相邻块信息
*        cur_cu                   -O    当前cu信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMD_INTRA_Process(HLMD_REGS            *regs,
                            HLM_NEIGHBOR_INFO    *nbi_info,
                            HLMD_CU_INFO         *cur_cu)
{
    HLM_U16 ref_pixel_16x8[HLM_INTRA_REF_PIXEL_NUM_16x8] = { 0 };
    HLM_U16 ref_pixel_4x4[HLM_INTRA_REF_PIXEL_NUM_4x4]   = { 0 };
    HLM_U08 intra_pred_mode    = 0;
    HLM_U08 yuv_idx            = 0;
    HLM_U32 raster_idx         = 0;
    HLM_U32 zscan_idx          = 0;
    HLM_S32 j                  = 0;
    HLM_U16 *cur_cu_rec        = HLM_NULL;
    HLM_S32 yuv_comp           = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_U32 rec_stride         = 0;
    HLM_S32 cu_w[3]            = { 1 << cur_cu->com_cu_info.cu_width[0],
                                   1 << cur_cu->com_cu_info.cu_width[1],
                                   1 << cur_cu->com_cu_info.cu_width[2] };
    HLM_S32 cu_h[3]            = { 1 << cur_cu->com_cu_info.cu_height[0],
                                   1 << cur_cu->com_cu_info.cu_height[1],
                                   1 << cur_cu->com_cu_info.cu_height[2] };
    HLM_U16 k                  = 0;
    HLM_U16 default_value      = 1 << (regs->dec_bitdepth - 1);
    HLM_U08 block_num          = cur_cu->com_cu_info.intra_8x8_enable_flag ? 2 : 8;
    HLM_U08 block_size         = cur_cu->com_cu_info.intra_8x8_enable_flag ? 8 : 4;
    HLM_U32 skip_chroma[2]     = { 0 };
    HLM_U32 chroma_size        = block_size;
    HLM_U32 luma_size          = block_size;
    HLM_U32 chroma_size_offset = 0;
    HLM_U32 pu_x               = 0;
    HLM_U32 pu_y               = 0;
    HLM_U08 pu_pos             = 0;

    if (HLM_I_16x8 == cur_cu->com_cu_info.cu_type)
    {
        intra_pred_mode = cur_cu->com_cu_info.cu_pred_info.pu_info[0].intra_pred_mode;
        if (HLM_INTRA_16x8_DC == intra_pred_mode)
        {
            if ((cur_cu->com_cu_info.left_unavail) && (cur_cu->com_cu_info.up_unavail))
            {
                intra_pred_mode = HLM_INTRA_16x8_DC_128;
            }
            else if (cur_cu->com_cu_info.left_unavail)
            {
                intra_pred_mode = HLM_INTRA_16x8_DC_TOP;
            }
            else if (cur_cu->com_cu_info.up_unavail)
            {
                intra_pred_mode = HLM_INTRA_16x8_DC_LEFT;
            }
        }

        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            for (k = 0; k < HLM_INTRA_REF_PIXEL_NUM_16x8; k++)
            {
                ref_pixel_16x8[k] = default_value;
            }
            memset(cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx], 0, HLM_WIDTH_SIZE * HLM_HEIGHT_SIZE * sizeof(HLM_U16));
            HLM_COM_Intra16x8RefPel(nbi_info, &cur_cu->com_cu_info, intra_pred_mode, yuv_idx, ref_pixel_16x8);
            HLM_INTRA_PRED_16x8[intra_pred_mode](ref_pixel_16x8, cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx],
                yuv_idx == 0 ? regs->dec_pic_luma_bitdepth : regs->dec_pic_chroma_bitdepth,
                HLM_WIDTH_SIZE, 1 << cur_cu->com_cu_info.cu_width[yuv_idx], 1 << cur_cu->com_cu_info.cu_height[yuv_idx]);
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
            cur_cu_rec += (cur_cu->com_cu_info.cu_y * rec_stride * (cu_h[yuv_idx]) + cur_cu->com_cu_info.cu_x *  (cu_w[yuv_idx]));
            for (j = 0; j < (cu_h[yuv_idx]); j++)
            {
                memcpy(cur_cu_rec + rec_stride * j,
                    cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + (HLM_WIDTH_SIZE) * j, cu_w[yuv_idx] * sizeof(HLM_U16));
            }
        }
    }
#if LINE_BY_LINE
    else if (HLM_I_LINE == cur_cu->com_cu_info.cu_type || HLM_I_ROW == cur_cu->com_cu_info.cu_type)
    {
        HLM_U08 line_index = 0;
        HLM_U08 i = 0;
        HLM_U32  DC_pixel = 0;
#if  LINE_BY_LINE_4x1
        HLM_U08 pred_lenght = 4;
        HLM_U32 pred_mode[4] = { cur_cu->com_cu_info.cu_pred_info.pu_info[0].intra_pred_mode,
            cur_cu->com_cu_info.cu_pred_info.pu_info[1].intra_pred_mode,
            cur_cu->com_cu_info.cu_pred_info.pu_info[2].intra_pred_mode,
            cur_cu->com_cu_info.cu_pred_info.pu_info[3].intra_pred_mode };
#else
        intra_pred_mode = cur_cu->com_cu_info.cu_pred_info.pu_info[0].intra_pred_mode;
#endif
        if (HLM_I_LINE == cur_cu->com_cu_info.cu_type)
        {
            for (line_index = 0; line_index < 8; line_index++)
            {
                HLM_U08 pu_pos = line_index * HLM_WIDTH_SIZE;
                for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
                {
                    HLM_U16 *cu_pixel = (yuv_idx == HLM_LUMA_Y) ? nbi_info->intra_rec_up_y : (yuv_idx == HLM_CHROMA_U) ? nbi_info->intra_rec_up_u : nbi_info->intra_rec_up_v;
                    HLM_S32 width = (1 << cur_cu->com_cu_info.cu_width[yuv_idx]);
                    HLM_S32 height = (1 << cur_cu->com_cu_info.cu_height[yuv_idx]);
                    HLM_U08 qp = cur_cu->com_cu_info.qp[yuv_idx];
                    HLM_COEFF *src = cur_cu->com_cu_info.cu_pred_info.coeff[yuv_idx] + pu_pos;
                    HLM_COEFF *dst = cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos;
                    cu_pixel += (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[yuv_idx]);
                    if (line_index + cur_cu->com_cu_info.cu_y == 0)
                    {
                        for (j = 1; j < width + 2; j++)
                        {
                            ref_pixel_16x8[j] = 1 << (regs->dec_pic_luma_bitdepth - 1);
                        }
                    }
                    else
                    {
                        if (line_index == 0)
                        {
                            memcpy(ref_pixel_16x8 + 1, cu_pixel, width * sizeof(HLM_U16));
                        }
                        else
                        {
                            memcpy(ref_pixel_16x8 + 1, cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos - HLM_WIDTH_SIZE, width * sizeof(HLM_U16));
                        }
                        ref_pixel_16x8[width + 1] = ref_pixel_16x8[width];
                    }
                    if (0 == (cur_cu->com_cu_info.cu_x))
                    {
                        ref_pixel_16x8[0] = ref_pixel_16x8[1];
                    }
                    else
                    {
                        ref_pixel_16x8[0] = (yuv_idx == HLM_LUMA_Y) ? nbi_info->intra_rec_left_y[line_index] : (yuv_idx == HLM_CHROMA_U) ? nbi_info->intra_rec_left_u[line_index] : nbi_info->intra_rec_left_v[line_index];
                    }
#if  LINE_BY_LINE_4x1
                    HLM_U08   block_lenght = yuv_idx == 0 ? pred_lenght : pred_lenght >> cur_cu->com_cu_info.chroma_offset_x;
                    for (j = 0; j < width / block_lenght; j++)
                    {
                        switch (pred_mode[j])
                        {
                        case 0:
                            for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//垂直
                            {

                                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i + (line_index << 4)] = ref_pixel_16x8[i + 1];
                            }
                            break;
                        case 2:
                            DC_pixel = width >> 1;
                            for (i = 1 + j * block_lenght; i < (j + 1)* block_lenght + 1; i++)//垂直DC
                                DC_pixel += ref_pixel_16x8[i];
                            DC_pixel >>= cur_cu->com_cu_info.cu_width[yuv_idx];
                            for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)
                                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i + (line_index << 4)] = DC_pixel;
                            break;
                        case 1:
                            for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//右下45
                                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i + (line_index << 4)] = (ref_pixel_16x8[i + 1] + ref_pixel_16x8[i + 2] + 1) >> 1;
                            break;
                        case 3:
                            for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//左下45
                                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i + (line_index << 4)] = (ref_pixel_16x8[i] + ref_pixel_16x8[i + 1] + 1) >> 1;
                            break;
                        }

                    }
#else
                    switch (intra_pred_mode)
                    {
                    case 0:
                        for (j = 0; j < width; j++)//垂直
                            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][j + (line_index << 4)] = ref_pixel_16x8[j + 1];
                        break;
                    case 2:
                        DC_pixel = width >> 1;
                        for (i = 1; i < width + 1; i++)//垂直DC
                            DC_pixel += ref_pixel_16x8[i];
                        DC_pixel >>= cur_cu->com_cu_info.cu_width[yuv_idx];
                        for (i = 0; i < width; i++)
                            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i + (line_index << 4)] = DC_pixel;
                        break;
                    case 1:
                        for (i = 0; i < width; i++)//右下45
                            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i + (line_index << 4)] = (ref_pixel_16x8[i + 1] + ref_pixel_16x8[i + 2] + 1) >> 1;
                        break;
                    case 3:
                        for (i = 0; i < width; i++)//左下45
                            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i + (line_index << 4)] = (ref_pixel_16x8[i] + ref_pixel_16x8[i + 1] + 1) >> 1;
                        break;
                    }
#endif
                    HLM_COM_PixelDequant(src, dst, width, 1, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, qp);
                    HLM_COM_AddRes(cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos,
                        cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos,
                        cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
                        yuv_idx == HLM_LUMA_Y ? cur_cu->luma_bitdepth : cur_cu->chroma_bitdepth,
                        width, 1, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
                }
            }

        }
        else
        {
            for (line_index = 0; line_index < 16; line_index++)
            {
                HLM_U08 pu_pos = line_index;
                for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
                {
                    HLM_U16 *cu_pixel = (yuv_idx == HLM_LUMA_Y) ? nbi_info->intra_rec_up_y : (yuv_idx == HLM_CHROMA_U) ? nbi_info->intra_rec_up_u : nbi_info->intra_rec_up_v;
                    HLM_S32 width = (1 << cur_cu->com_cu_info.cu_width[yuv_idx]);
                    HLM_S32 height = (1 << cur_cu->com_cu_info.cu_height[yuv_idx]);
                    HLM_U08 qp = cur_cu->com_cu_info.qp[yuv_idx];
                    HLM_COEFF *src = cur_cu->com_cu_info.cu_pred_info.coeff[yuv_idx] + pu_pos;
                    HLM_COEFF *dst = cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos;
                    HLM_U16 *cu_left = (yuv_idx == HLM_LUMA_Y) ? nbi_info->intra_rec_left_y : (yuv_idx == HLM_CHROMA_U) ? nbi_info->intra_rec_left_u : nbi_info->intra_rec_left_v;
                    cu_pixel += (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[yuv_idx]) + pu_pos;
                    if (line_index + cur_cu->com_cu_info.cu_x == 0)
                    {
                        for (j = 1; j < height + 2; j++)
                        {
                            ref_pixel_16x8[j] = 1 << (regs->dec_pic_luma_bitdepth - 1);
                        }
                    }
                    else
                    {
                        if (line_index == 0)
                            memcpy(ref_pixel_16x8 + 1, cu_left, height * sizeof(HLM_U16));
                        else
                        {
                            for (j = 1; j < height + 1; j++)
                            {
                                ref_pixel_16x8[j] = cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx][pu_pos - 1 + ((j - 1) << 4)];
                            }
                        }
                        ref_pixel_16x8[height + 1] = ref_pixel_16x8[height];
                    }
                    if (0 == (cur_cu->com_cu_info.cu_y))
                    {
                        ref_pixel_16x8[0] = ref_pixel_16x8[1];
                    }
                    else
                    {
                        ref_pixel_16x8[0] = cu_pixel[0];
                    }
#if  LINE_BY_LINE_4x1
                    HLM_U08   block_lenght = yuv_idx == 0 ? pred_lenght : pred_lenght >> cur_cu->com_cu_info.chroma_offset_y;
                    for (j = 0; j < height / block_lenght; j++)
                    {
                        switch (pred_mode[j])
                        {
                        case 0:
                            for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//垂直
                                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i * 16 + (line_index)] = ref_pixel_16x8[i + 1];
                            break;
                        case 2:
                            DC_pixel = block_lenght >> 1;
                            for (i = 1 + j * block_lenght; i < (j + 1)* block_lenght + 1; i++)//垂直DC
                                DC_pixel += ref_pixel_16x8[i];
                            DC_pixel >>= cur_cu->com_cu_info.cu_height[yuv_idx];
                            for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)
                                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i * 16 + (line_index)] = DC_pixel;
                            break;
                        case 1:
                            for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//右下45
                                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i * 16 + (line_index)] = (ref_pixel_16x8[i + 1] + ref_pixel_16x8[i + 2] + 1) >> 1;
                            break;
                        case 3:
                            for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//左下45
                                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i * 16 + (line_index)] = (ref_pixel_16x8[i] + ref_pixel_16x8[i + 1] + 1) >> 1;
                            break;
                        default:
                            printf("error pred mode!!!");
                        }
                    }
#else
                    switch (intra_pred_mode)
                    {
                    case 0:
                        for (i = 0; i < height; i++)//垂直
                            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i * 16 + (line_index)] = ref_pixel_16x8[i + 1];
                        break;
                    case 2:
                        DC_pixel = height >> 1;
                        for (i = 1; i < height + 1; i++)//垂直DC
                            DC_pixel += ref_pixel_16x8[i];
                        DC_pixel >>= cur_cu->com_cu_info.cu_height[yuv_idx];
                        for (i = 0; i < height; i++)
                            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i * 16 + (line_index)] = DC_pixel;
                        break;
                    case 1:
                        for (i = 0; i < height; i++)//右下45
                            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i * 16 + (line_index)] = (ref_pixel_16x8[i + 1] + ref_pixel_16x8[i + 2] + 1) >> 1;
                        break;
                    case 3:
                        for (i = 0; i < height; i++)//左下45
                            cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx][i * 16 + (line_index)] = (ref_pixel_16x8[i] + ref_pixel_16x8[i + 1] + 1) >> 1;
                        break;
                    }
#endif
                    HLM_COM_PixelDequant(src, dst, 1, height, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, qp);
                    HLM_COM_AddRes(cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos,
                        cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos,
                        cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
                        yuv_idx == HLM_LUMA_Y ? cur_cu->luma_bitdepth : cur_cu->chroma_bitdepth,
                        1, height, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
                }
            }
        }

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
            cur_cu_rec += (cur_cu->com_cu_info.cu_y * rec_stride * (cu_h[yuv_idx]) + cur_cu->com_cu_info.cu_x *  (cu_w[yuv_idx]));
            for (j = 0; j < (cu_h[yuv_idx]); j++)
            {
                memcpy(cur_cu_rec + rec_stride * j, cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + (HLM_WIDTH_SIZE)* j, (cu_w[yuv_idx]) * sizeof(HLM_U16));
            }
        }

    }
#endif
    else  // 8x8或4x4
    {
        HLM_COM_Intra8x8ChromaPb(regs->image_format, block_size, skip_chroma, &chroma_size, &chroma_size_offset);
        for (raster_idx = 0; raster_idx < block_num; raster_idx++)
        {
            pu_x = raster_idx % 4;
            pu_y = raster_idx >> 2;
            zscan_idx = HLM_INTRA_RASTER_TO_ZSCAN[raster_idx];
            intra_pred_mode = cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].intra_pred_mode;
            if (HLM_INTRA_4x4_DC == intra_pred_mode)
            {
                if ((cur_cu->com_cu_info.left_unavail) && (cur_cu->com_cu_info.up_unavail) &&
                    (cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].pu_x == 0) &&
                    (cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].pu_y == 0))
                {
                    intra_pred_mode = HLM_INTRA_4x4_DC_128;
                }
                else if ((cur_cu->com_cu_info.left_unavail) && (cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].pu_x == 0))
                {
                    intra_pred_mode = HLM_INTRA_4x4_DC_TOP;
                }
                else if ((cur_cu->com_cu_info.up_unavail) && (cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].pu_y == 0))
                {
                    intra_pred_mode = HLM_INTRA_4x4_DC_LEFT;
                }
            }

            for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
            {
                block_size = yuv_idx == 0 ? luma_size : chroma_size;
                pu_pos = (pu_y * block_size) * HLM_WIDTH_SIZE + (pu_x * block_size);
                if (yuv_idx && skip_chroma[raster_idx] && luma_size == 8)
                {
                    continue;
                }
                for (k = 0; k < HLM_INTRA_REF_PIXEL_NUM_4x4; k++)
                {
                    ref_pixel_4x4[k] = default_value;
                }
                HLM_COM_Intra4x4RefPel(regs->dec_recon_y_base, regs->dec_recon_cb_base, regs->dec_recon_cr_base,
                    regs->dec_output_luma_stride, regs->dec_output_chroma_stride, regs->dec_output_chroma_stride,
                    regs->cur_patch_param->patch_coded_width[0] >> 4, nbi_info, &cur_cu->com_cu_info,
                    intra_pred_mode, yuv_idx, zscan_idx, block_size == 8 ? 3 : 2, ref_pixel_4x4);
                HLM_INTRA_PRED_4x4[intra_pred_mode](ref_pixel_4x4, cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos,
                    yuv_idx == 0 ? regs->dec_pic_luma_bitdepth : regs->dec_pic_chroma_bitdepth, HLM_WIDTH_SIZE, block_size, block_size);
            }

            HLMD_TQ_Process(cur_cu, raster_idx, skip_chroma[raster_idx] && luma_size == 8 ? 1 : yuv_comp);

            // 拷贝重建
            for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
            {
                block_size = yuv_idx == 0 ? luma_size : chroma_size;
                pu_pos = (pu_y * block_size) * HLM_WIDTH_SIZE + (pu_x * block_size);
                if (yuv_idx && skip_chroma[raster_idx] && luma_size == 8)
                {
                    continue;
                }
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
                cur_cu_rec += (cur_cu->com_cu_info.cu_y * cu_h[yuv_idx]) * rec_stride + (cur_cu->com_cu_info.cu_x * cu_w[yuv_idx])
                    + (pu_y * block_size) * rec_stride + (pu_x * block_size);
                for (j = 0; j < block_size; j++)
                {
                    memcpy(cur_cu_rec + rec_stride * j,
                        cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos + HLM_WIDTH_SIZE * j, block_size * sizeof(HLM_U16));
                }
            }
        }
    }
}

/***************************************************************************************************
* 功  能：SCC预测主函数
* 参  数：*
*        regs                     -I    硬件寄存器
*        nbi_info                 -I    当前cu相邻块信息
*        cur_cu                   -O    当前cu信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMD_SCC_Process(HLMD_REGS            *regs,
                          HLM_NEIGHBOR_INFO    *nbi_info,
                          HLMD_CU_INFO         *cur_cu)
{
    HLM_U32 zscan_idx    = 0;  //光栅扫描顺序
    HLM_S32 pu_x         = 0;  //4x4子宏块相对于宏块左上角的x/y坐标，以子宏块为单位
    HLM_S32 pu_y         = 0;
    HLM_S32 pu_x_in_pic  = 0;
    HLM_S32 pu_y_in_pic  = 0;
    HLM_S32 pu_x_in_area = 0;
    HLM_S32 pu_y_in_area = 0;
    HLM_S32 cu_pixel_x   = cur_cu->com_cu_info.cu_x << HLM_LOG2_WIDTH_SIZE;  // 当前CU的坐标，以像素为单位
    HLM_S32 cu_pixel_y   = cur_cu->com_cu_info.cu_y << HLM_LOG2_HEIGHT_SIZE;
    HLM_U16 *rec_ptr[3]  = { 0 };  // 重建图像指针，指向图像的左上角点
    HLM_U32 stride[3]    = { 0 };
    HLM_U16 *dst         = 0;
    HLM_U16 *src         = 0;
    HLM_U08 yuv_idx      = 0;
    HLM_U08 i            = 0;
    HLM_S32 rec_len      = cu_pixel_x - HLM_MAX(0, cu_pixel_x - HLM_IBC_BUFFER_WIDTH);
    HLM_S32 pad_len      = HLM_IBC_BUFFER_WIDTH - rec_len;
    HLM_S32 yuv_comp     = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_S08 hor_shift[3] = { 0 };
    HLM_S08 ver_shift[3] = { 0 };
    HLM_U32 pu_w         = 0;
    HLM_U32 pu_h         = 0;
    HLM_U32 pu_pos       = 0;
    HLM_U08 j            = 0;
    HLM_COEFF *coef_src  = HLM_NULL;
    HLM_COEFF *coef_dst  = HLM_NULL;
    HLM_S32 boundary_x   = 0;
    HLM_S32 boundary_y   = 0;
#if MIX_IBC
    HLM_IBC_PU_INFO pb_info  = { 0 };  // 区分亮色度
    HLM_IBC_PU_INFO *pu_info = HLM_NULL;
    HLM_U16 *cb_pred         = HLM_NULL;
    HLM_MV *cur_bv           = HLM_NULL;
    HLM_MV  cur_bvp          = { 0 };
    HLM_U08 direct           = 0;
    HLM_S32 bvx_offset       = 0;
    HLM_S32 val              = 0;
    HLM_U08 merge_flag       = cur_cu->com_cu_info.merge_flag;
    HLM_U16 *up_rec[3]       = { 0 };  // 上一行的重建像素
#endif

    stride[0]  = regs->dec_output_luma_stride;
    stride[1]  = regs->dec_output_chroma_stride;
    stride[2]  = regs->dec_output_chroma_stride;
    rec_ptr[0] = regs->dec_recon_y_base;
    rec_ptr[1] = regs->dec_recon_cb_base;
    rec_ptr[2] = regs->dec_recon_cr_base;
#if MIX_IBC
    up_rec[0]  = nbi_info->intra_rec_up_y;
    up_rec[1]  = nbi_info->intra_rec_up_u;
    up_rec[2]  = nbi_info->intra_rec_up_v;
#if FIRST_COLUMN_IBC
    cur_cu->com_cu_info.first_column_ibc_flag = cur_cu->com_cu_info.left_unavail && !cur_cu->com_cu_info.up_unavail;
#else
    cur_cu->com_cu_info.first_column_ibc_flag = 0;
#endif
#endif

    if (regs->segment_enable_flag)
    {
        rec_len = cu_pixel_x % (1 << regs->segment_width_in_log2);
        pad_len = HLM_IBC_BUFFER_WIDTH - rec_len;
    }
    boundary_x = regs->segment_enable_flag ? cu_pixel_x % (1 << regs->segment_width_in_log2) : cu_pixel_x;
    boundary_y = regs->segment_enable_flag ? cu_pixel_y % (1 << regs->segment_height_in_log2) : cu_pixel_y;

    // 填充搜索区域
    HLM_COM_GetFormatShift(regs->image_format, hor_shift, ver_shift);
    for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
    {
#if MIX_IBC && FIRST_COLUMN_IBC
        if (cur_cu->com_cu_info.first_column_ibc_flag)
        {
            assert(boundary_x == 0 && boundary_y > 0);
            HLM_COM_CopyUpRefPixel(cur_cu->com_cu_info.search_area[yuv_idx],
                up_rec[yuv_idx] + (cur_cu->com_cu_info.cu_x << 4 >> hor_shift[yuv_idx]), hor_shift[yuv_idx],
                regs->segment_enable_flag ? (1 << regs->segment_width_in_log2) : regs->cur_patch_param->patch_coded_width[0]);
            continue;
        }
#endif
        if (rec_len)
        {
            HLM_COM_SearchAreaCopy(cur_cu->com_cu_info.search_area[yuv_idx], rec_ptr[yuv_idx], stride[yuv_idx],
                rec_len >> hor_shift[yuv_idx], HLM_IBC_SEARCH_AREA_HEIGHT >> ver_shift[yuv_idx],
                (cu_pixel_x - rec_len) >> hor_shift[yuv_idx], cu_pixel_y >> ver_shift[yuv_idx],
                (HLM_IBC_BUFFER_WIDTH - rec_len) >> hor_shift[yuv_idx]);
        }
        if (pad_len)
        {
            HLM_COM_SearchAreaPadding(cur_cu->com_cu_info.search_area[yuv_idx], rec_ptr[yuv_idx],
                                      yuv_idx == 0 ? cur_cu->luma_bitdepth : cur_cu->chroma_bitdepth, stride[yuv_idx],
                                      pad_len >> hor_shift[yuv_idx], HLM_IBC_SEARCH_AREA_HEIGHT >> ver_shift[yuv_idx],
                                      ver_shift[yuv_idx], boundary_x, boundary_y,
                                      cur_cu->com_cu_info.cu_x, cur_cu->com_cu_info.cu_y);
        }
    }

    for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
    {
        pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
        pu_y = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
#if MIX_IBC
        direct = tbl_merge_type[merge_flag][zscan_idx];
        bvx_offset = ((zscan_idx & 1) + 1) << 2;
        pu_info = &cur_cu->com_cu_info.ibc_pu_info[merge_flag][zscan_idx];
        for (i = 0; i < pu_info->sub_pu_num; i++)
        {
            if (!(direct == 0 && i == 0))  // 不是定长码
            {
                HLM_COM_GetBvp(&cur_cu->com_cu_info, merge_flag, zscan_idx, i, &cur_bvp);
                pu_info->sub_bv[i].mvx = cur_bvp.mvx + pu_info->sub_bvd[i].mvx;
            }

            // bv clip
#if FIRST_COLUMN_IBC
            if (cur_cu->com_cu_info.first_column_ibc_flag)
            {
                val = (pu_x << 2) + pu_info->sub_bv[i].mvx;
                val = HLM_CLIP(val, 0, 127);
                pu_info->sub_bv[i].mvx = val - (pu_x << 2);
            }
            else
#endif
            {
                val = -pu_info->sub_bv[i].mvx - bvx_offset;
                val = HLM_CLIP(val, 0, 127);
                pu_info->sub_bv[i].mvx = -val - bvx_offset;
            }
            val = HLM_ABS(pu_info->sub_bv[i].mvy);
            val = HLM_CLIP(val, 0, 3);
            pu_info->sub_bv[i].mvy = pu_y == 0 ? val : -val;
        }
        HLM_COM_UpdateInnerBv(&cur_cu->com_cu_info, zscan_idx, merge_flag, pu_info->part_type, pu_info->sub_bv);
#endif

        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            pu_w         = 4 >> hor_shift[yuv_idx];
            pu_h         = 4 >> ver_shift[yuv_idx];
            pu_pos       = (pu_y * pu_h) * HLM_WIDTH_SIZE + (pu_x * pu_w);
            pu_x_in_pic  = (cu_pixel_x >> hor_shift[yuv_idx]) + pu_x * pu_w;
            pu_y_in_pic  = (cu_pixel_y >> ver_shift[yuv_idx]) + pu_y * pu_h;
            pu_x_in_area = (HLM_IBC_BUFFER_WIDTH >> hor_shift[yuv_idx]) + pu_x * pu_w;
            pu_y_in_area = pu_y * pu_h;
#if MIX_IBC
            cb_pred = cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx];
            pu_info = &cur_cu->com_cu_info.ibc_pu_info[cur_cu->com_cu_info.merge_flag][zscan_idx];
            memcpy(&pb_info, pu_info, sizeof(HLM_IBC_PU_INFO));
            if (yuv_idx > 0 && cur_cu->com_cu_info.cu_width[1] != cur_cu->com_cu_info.cu_width[0])  // 420/422的色度
            {
                HLM_COM_GetChromaPbInfo(&pb_info, hor_shift[yuv_idx], ver_shift[yuv_idx]);
            }
#if FIRST_COLUMN_IBC
            if (cur_cu->com_cu_info.first_column_ibc_flag)
            {
                pu_x_in_area = pu_x * pu_w;
            }
#endif
            for (i = 0; i < pb_info.sub_pu_num; i++)
            {
                cur_bv = &pb_info.sub_bv[i];
                HLM_COM_GetBlock(cur_cu->com_cu_info.search_area[yuv_idx], HLM_IBC_SEARCH_AREA_WIDTH,
                    cb_pred + pu_pos + pb_info.sub_y[i] * HLM_WIDTH_SIZE + pb_info.sub_x[i], HLM_WIDTH_SIZE,
                    pu_x_in_area + pb_info.sub_x[i] + (cur_bv->mvx >> hor_shift[yuv_idx]),
                    pu_y_in_area + pb_info.sub_y[i] + (cur_bv->mvy >> ver_shift[yuv_idx]),
                    pb_info.sub_w[i], pb_info.sub_h[i]);
            }
#else
            HLM_COM_GetBlock(cur_cu->com_cu_info.search_area[yuv_idx], HLM_IBC_SEARCH_AREA_WIDTH,
                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos, HLM_WIDTH_SIZE,
                pu_x_in_area + (cur_cu->bv[zscan_idx].mvx >> hor_shift[yuv_idx]),
                pu_y_in_area + (cur_cu->bv[zscan_idx].mvy >> ver_shift[yuv_idx]),
                pu_w, pu_h);
#endif
        }

        HLMD_TQ_Process(cur_cu, (pu_y << 2) + pu_x, yuv_comp);

        // 拷贝重建像素、更新搜索区域
        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            pu_w         = 4 >> hor_shift[yuv_idx];
            pu_h         = 4 >> ver_shift[yuv_idx];
            pu_pos       = (pu_y * pu_h) * HLM_WIDTH_SIZE + (pu_x * pu_w);
            pu_x_in_pic  = (cu_pixel_x >> hor_shift[yuv_idx]) + pu_x * pu_w;
            pu_y_in_pic  = (cu_pixel_y >> ver_shift[yuv_idx]) + pu_y * pu_h;
            pu_x_in_area = (HLM_IBC_BUFFER_WIDTH >> hor_shift[yuv_idx]) + pu_x * pu_w;
            pu_y_in_area = pu_y * pu_h;

            src = cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos;
            dst = rec_ptr[yuv_idx] + pu_y_in_pic * stride[yuv_idx] + pu_x_in_pic;
            for (i = 0; i < pu_h; i++)
            {
                memcpy(dst, src, pu_w * sizeof(HLM_U16));
                src += HLM_WIDTH_SIZE;
                dst += stride[yuv_idx];
            }

            src = cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos;
            dst = cur_cu->com_cu_info.search_area[yuv_idx] + pu_y_in_area * HLM_IBC_SEARCH_AREA_WIDTH + pu_x_in_area;
            for (i = 0; i < pu_h; i++)
            {
                memcpy(dst, src, pu_w * sizeof(HLM_U16));
                src += HLM_WIDTH_SIZE;
                dst += HLM_IBC_SEARCH_AREA_WIDTH;
            }
        }
    }
}
