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
#include "hlmc_intra.h"
#include "hlmc_common.h"
#include "hlmc_ecd.h"
#include "hlmc_tq.h"
#include "hlmc_rc.h"

#if LINE_BY_LINE
/***************************************************************************************************
* 功  能：获取1x8的参考像素
* 参  数：*
*        regs                    -I       硬件寄存器
*        *cur_cu                 -I       当前ctu信息
*        *ref_pixel               -O       参考像素
*        line_index               -I       预测行
*        org_pixel_flag           -I       原始值做参考标志
*        pred_mode                -I       预测模式
* 返回值：
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTRA_MODE_get_ref_pixel_1x8(HLMC_REGS      *regs,
                                           HLMC_CU_INFO   *cur_cu,
                                           HLM_U16        *ref_pixel,
                                           HLM_U08         line_index,
                                           HLM_U08         org_pixel_flag,
#if  LINE_BY_LINE_4x1
                                           HLM_U08         pred_lenght,
                                           HLM_U08        *pred_mode)
#else
                                           HLM_U08         *pred_mode)
#endif

{
    HLM_U32 pixel_x[3] = { cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2] };
    HLM_U32 pixel_y[3] = { cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2] };
    HLM_U32 stride[3] = { regs->enc_input_luma_stride ,regs->enc_input_chroma_stride,regs->enc_input_chroma_stride };
    HLM_U32 i = 0;
    HLM_U32 j = 0;
    HLM_U32 channel = 0;
    HLM_U32 DC_pixel = 0;
    HLM_U32 yuv_comp = regs->image_format == HLM_IMG_YUV_400 ? 1 : HLM_MAX_COMP_NUM;
    HLM_U16  *ref_y = org_pixel_flag ? regs->enc_input_y_base : regs->enc_recon_y_base;
    HLM_U16  *ref_cb = org_pixel_flag ? regs->enc_input_cb_base : regs->enc_recon_cb_base;
    HLM_U16  *ref_cr = org_pixel_flag ? regs->enc_input_cr_base : regs->enc_recon_cr_base;
    for (channel = 0; channel < yuv_comp; channel++)
    {
        HLM_U16 *cu_pixel = (channel == HLM_LUMA_Y) ? ref_y : (channel == HLM_CHROMA_U) ? ref_cb : ref_cr;
        HLM_U16 *ref_pixel_yuv = ref_pixel;
        HLM_U32 width = (1 << cur_cu->com_cu_info.cu_width[channel]);
        HLM_U32 height = (1 << cur_cu->com_cu_info.cu_height[channel]);
        if (line_index > width)
            continue;
        cu_pixel += stride[channel] * (pixel_y[channel]) + pixel_x[channel] + line_index;
        if (0 == (cur_cu->com_cu_info.cu_x + line_index))
        {
            for (i = 1; i < height + 2; i++)
            {
                ref_pixel_yuv[i] = 1 << (regs->bitdepth - 1);
            }
        }
        else
        {
            for (i = 1; i < height + 1; i++)
            {
                ref_pixel_yuv[i] = *(cu_pixel + (stride[channel] * (i - 1)) - 1);
            }
            ref_pixel_yuv[height + 1] = ref_pixel_yuv[height];
        }
        if (0 == (cur_cu->com_cu_info.cu_y))
        {
            ref_pixel_yuv[0] = ref_pixel_yuv[1];
        }
        else
        {
            ref_pixel_yuv[0] = *(cu_pixel - stride[channel]);
        }
#if  LINE_BY_LINE_4x1
        HLM_U08   block_lenght = channel == 0 ? pred_lenght : pred_lenght >> cur_cu->com_cu_info.chroma_offset_y;
        for (j = 0; j < height / block_lenght; j++)
        {
            switch (pred_mode[j])
            {
            case 0:
                for (i = j* block_lenght; i < (j + 1)* block_lenght; i++)//垂直
                    cur_cu->com_cu_info.cu_pred_info.pred[channel][i * 16 + (line_index)] = ref_pixel_yuv[i + 1];
                break;
            case 2:
                DC_pixel = block_lenght >> 1;
                for (i = 1 + j * block_lenght; i < (j + 1)* block_lenght + 1; i++)//垂直DC
                    DC_pixel += ref_pixel_yuv[i];
                DC_pixel >>= cur_cu->com_cu_info.cu_height[channel];
                for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)
                    cur_cu->com_cu_info.cu_pred_info.pred[channel][i * 16 + (line_index)] = DC_pixel;
                break;
            case 1:
                for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//右下45
                    cur_cu->com_cu_info.cu_pred_info.pred[channel][i * 16 + (line_index)] = (ref_pixel_yuv[i + 1] + ref_pixel_yuv[i + 2] + 1) >> 1;
                break;
            case 3:
                for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//左下45
                    cur_cu->com_cu_info.cu_pred_info.pred[channel][i * 16 + (line_index)] = (ref_pixel_yuv[i] + ref_pixel_yuv[i + 1] + 1) >> 1;
                break;
              default:
                  printf("error pred mode!!!");
            }
        }
#else
        switch (pred_mode[0])
        {
        case 0:
            for (i = 0; i < height; i++)//垂直
                cur_cu->com_cu_info.cu_pred_info.pred[channel][i * 16 + (line_index)] = ref_pixel_yuv[i + 1];
            break;
        case 2:
            DC_pixel = height >> 1;
            for (i = 1; i < height + 1; i++)//垂直DC
                DC_pixel += ref_pixel_yuv[i];
            DC_pixel >>= cur_cu->com_cu_info.cu_height[channel];
            for (i = 0; i < height; i++)
                cur_cu->com_cu_info.cu_pred_info.pred[channel][i * 16 + (line_index)] = DC_pixel;
            break;
        case 1:
            for (i = 0; i < height; i++)//右下45
                cur_cu->com_cu_info.cu_pred_info.pred[channel][i * 16 + (line_index)] = (ref_pixel_yuv[i + 1] + ref_pixel_yuv[i + 2] + 1) >> 1;
            break;
        case 3:
            for (i = 0; i < height; i++)//左下45
                cur_cu->com_cu_info.cu_pred_info.pred[channel][i * 16 + (line_index)] = (ref_pixel_yuv[i] + ref_pixel_yuv[i + 1] + 1) >> 1;
            break;
        }
#endif
    }
}
/***************************************************************************************************
* 功  能：获取16x1的参考像素
* 参  数：*
*        regs                    -I       硬件寄存器
*        *cur_cu                 -I       当前ctu信息
*        *ref_pixel               -O       参考像素
*        line_index               -I       预测行
*        org_pixel_flag           -I       原始值做参考标志
*        pred_mode                -I       预测行
* 返回值：
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTRA_MODE_get_ref_pixel_16x1(HLMC_REGS      *regs,
                                            HLMC_CU_INFO   *cur_cu,
                                            HLM_U16        *ref_pixel,
                                            HLM_U08         line_index,
                                            HLM_U08         org_pixel_flag,
#if  LINE_BY_LINE_4x1
                                            HLM_U08         pred_lenght,
#endif
                                            HLM_U08         *pred_mode)
{
    HLM_U32 pixel_x[3] = { cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1],
        cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2] };
    HLM_U32 pixel_y[3] = { cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1],
        cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2] };
    HLM_U32 stride[3] = { regs->enc_input_luma_stride ,regs->enc_input_chroma_stride,regs->enc_input_chroma_stride };
    HLM_U32 i = 0;
    HLM_U32 j = 0;
    HLM_U32 channel = 0;
    HLM_U32 DC_pixel = 0;
    HLM_U32 yuv_comp = regs->image_format == HLM_IMG_YUV_400 ? 1 : HLM_MAX_COMP_NUM;
    HLM_U16  *ref_y = org_pixel_flag ? regs->enc_input_y_base : regs->enc_recon_y_base;
    HLM_U16  *ref_cb = org_pixel_flag ? regs->enc_input_cb_base : regs->enc_recon_cb_base;
    HLM_U16  *ref_cr = org_pixel_flag ? regs->enc_input_cr_base : regs->enc_recon_cr_base;
    for (channel = 0; channel < yuv_comp; channel++)
    {
        HLM_U16 *cu_pixel = (channel == HLM_LUMA_Y) ? ref_y : (channel == HLM_CHROMA_U) ? ref_cb : ref_cr;
        HLM_U16 *ref_pixel_yuv = ref_pixel;
        HLM_U32 width = (1 << cur_cu->com_cu_info.cu_width[channel]);
        HLM_U32 height = (1 << cur_cu->com_cu_info.cu_height[channel]);

        if (line_index > height)
            continue;
        cu_pixel += stride[channel] * (pixel_y[channel] + line_index) + pixel_x[channel];

        if (0 == (cur_cu->com_cu_info.cu_y + line_index))
        {
            for (i = 1; i < width + 2; i++)
            {
                ref_pixel_yuv[i] = 1 << (regs->bitdepth - 1);
            }
        }
        else
        {
            memcpy(ref_pixel_yuv + 1, cu_pixel - stride[channel], width * sizeof(HLM_U16));
            ref_pixel_yuv[width + 1] = ref_pixel_yuv[width];
        }
        if (0 == (cur_cu->com_cu_info.cu_x))
        {
            ref_pixel_yuv[0] = ref_pixel_yuv[1];
        }
        else
        {
            ref_pixel_yuv[0] = *(cu_pixel - 1);
        }
#if  LINE_BY_LINE_4x1
        HLM_U08   block_lenght = channel == 0 ? pred_lenght : pred_lenght >> cur_cu->com_cu_info.chroma_offset_x;
        for (j = 0; j < width / block_lenght; j++)
        {
            switch (pred_mode[j])
            {
            case 0:
                for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//垂直
                {
                        cur_cu->com_cu_info.cu_pred_info.pred[channel][i + (line_index << 4)] = ref_pixel_yuv[i + 1];
                }
                break;
            case 2:
                DC_pixel = width >> 1;
                for (i = 1 + j * block_lenght; i < (j + 1)* block_lenght + 1; i++)//垂直DC
                    DC_pixel += ref_pixel_yuv[i];
                DC_pixel >>= cur_cu->com_cu_info.cu_width[channel];
                for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)
                    cur_cu->com_cu_info.cu_pred_info.pred[channel][i + (line_index << 4)] = DC_pixel;
                break;
            case 1:
                for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//右下45
                    cur_cu->com_cu_info.cu_pred_info.pred[channel][i + (line_index << 4)] = (ref_pixel_yuv[i + 1] + ref_pixel_yuv[i + 2] + 1) >> 1;
                break;
            case 3:
                for (i = j * block_lenght; i < (j + 1)* block_lenght; i++)//左下45
                    cur_cu->com_cu_info.cu_pred_info.pred[channel][i + (line_index << 4)] = (ref_pixel_yuv[i] + ref_pixel_yuv[i + 1] + 1) >> 1;
                break;
            }
        }
#else
        switch (pred_mode[0])
        {
        case 0:
            for (i = 0; i < width; i++)//垂直
                cur_cu->com_cu_info.cu_pred_info.pred[channel][i + (line_index << 4)] = ref_pixel_yuv[i + 1];
            break;
        case 2:
#if  LINE_BY_LINE_4x1
            for (j = 0; j < width / block_lenght; j++)
            {
                DC_pixel = block_lenght >> 1;
                for (i = 1 + j * block_lenght; i < 1 + block_lenght + j * block_lenght; i++)//垂直DC
                    DC_pixel += ref_pixel_yuv[i];
                DC_pixel /= block_lenght;
                for (i = j * block_lenght; i < (j + 1) * block_lenght; i++)
                    cur_cu->com_cu_info.cu_pred_info.pred[channel][i + (line_index << 4)] = DC_pixel;
            }
#else
            DC_pixel = width >> 1;
            for (i = 1; i < width + 1; i++)//垂直DC
                DC_pixel += ref_pixel_yuv[i];
            DC_pixel >>= cur_cu->com_cu_info.cu_width[channel];
            for (i = 0; i < width; i++)
                cur_cu->com_cu_info.cu_pred_info.pred[channel][i + (line_index << 4)] = DC_pixel;
#endif
            break;
        case 1:
            for (i = 0; i < width; i++)//右下45
                cur_cu->com_cu_info.cu_pred_info.pred[channel][i + (line_index << 4)] = (ref_pixel_yuv[i + 1] + ref_pixel_yuv[i + 2] + 1) >> 1;
            break;
        case 3:
            for (i = 0; i < width; i++)//左下45
                cur_cu->com_cu_info.cu_pred_info.pred[channel][i + (line_index << 4)] = (ref_pixel_yuv[i] + ref_pixel_yuv[i + 1] + 1) >> 1;
            break;
        }
#endif
      

    }

}
#endif

// 获取16x8的参考像素
HLM_VOID HLMC_INTRA_MODE_get_ref_16x8(HLMC_REGS       *regs,
                                      HLMC_CU_INFO    *cur_cu,
                                      HLM_U16         *ref_pixel,
                                      HLM_U08         *ref_flag)
{
    HLM_U32 pixel_x[3]     = { cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0],
                               cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1],
                               cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2] };
    HLM_U32 pixel_y[3]     = { cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0],
                               cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1],
                               cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2] };
    HLM_U32 stride[3]      = { regs->enc_input_luma_stride ,regs->enc_input_chroma_stride,regs->enc_input_chroma_stride };
    HLM_U32 i              = 0;
    HLM_U32 channel        = 0;
    HLM_U32 yuv_comp       = regs->image_format == HLM_IMG_YUV_400 ? 1 : HLM_MAX_COMP_NUM;
    HLM_U16 *cu_pixel      = HLM_NULL;
    HLM_U16 *ref_pixel_yuv = HLM_NULL;
    HLM_U32 width          = 0;
    HLM_U32 height         = 0;

    for (channel = 0; channel < yuv_comp; channel++)
    {
        if (channel == HLM_LUMA_Y)
        {
            cu_pixel = regs->enc_input_y_base;
        }
        else if (channel == HLM_CHROMA_U)
        {
            cu_pixel = regs->enc_input_cb_base;
        }
        else
        {
            cu_pixel = regs->enc_input_cr_base;
        }
        ref_pixel_yuv = ref_pixel + channel * HLM_INTRA_REF_PIXEL_NUM_16x8;
        width         = (1 << cur_cu->com_cu_info.cu_width[channel]);
        height        = (1 << cur_cu->com_cu_info.cu_height[channel]);
        cu_pixel     += stride[channel] * pixel_y[channel] + pixel_x[channel];
        if ((cur_cu->com_cu_info.left_unavail) && (cur_cu->com_cu_info.up_unavail))
        {
            *ref_flag = 0;
        }
        else if (cur_cu->com_cu_info.left_unavail)
        {
            *ref_flag = HLM_INTRA_TOP;
            memcpy(ref_pixel_yuv + HLM_INTRA_TOP_REF_IDX, cu_pixel - stride[channel], width * sizeof(HLM_U16));
        }
        else if (cur_cu->com_cu_info.up_unavail)
        {
            *ref_flag = HLM_INTRA_LEFT;
            for (i = 0; i < height; i++)
            {
                ref_pixel_yuv[HLM_INTRA_LEFT_REF_IDX + i] = *(cu_pixel - 1 + i * stride[channel]);
            }
        }
        else
        {
            *ref_flag = HLM_INTRA_LEFT | HLM_INTRA_TOP | HLM_INTRA_TOP_LEFT;
            memcpy(ref_pixel_yuv + HLM_INTRA_TOP_REF_IDX, cu_pixel - stride[channel], width * sizeof(HLM_U16));
            for (i = 0; i < height; i++)
            {
                ref_pixel_yuv[HLM_INTRA_LEFT_REF_IDX + i] = *(cu_pixel - 1 + i * stride[channel]);
            }
            ref_pixel_yuv[HLM_INTRA_TOP_LEFT_REF_IDX] = *(cu_pixel - 1 - stride[channel]);
        }
    }
}

// 生成16x8的预测像素
HLM_U08 HLMC_INTRA_MODE_pred_16x8(HLM_U08      pred_mode,
                                  HLM_U16     *ref_pixel,
                                  HLM_U08      ref_flag,
                                  HLM_U08      bitdepth,
                                  HLM_U08      cu_w[3],
                                  HLM_U08      cu_h[3],
                                  HLM_U08      skip_luma,
                                  HLM_U08      skip_chroma,
                                  HLM_U16    **pred_pixel)
{
    HLM_U16 *ref_pixel_y = ref_pixel;
    HLM_U16 *ref_pixel_u = ref_pixel + HLM_INTRA_REF_PIXEL_NUM_16x8;
    HLM_U16 *ref_pixel_v = ref_pixel + HLM_INTRA_REF_PIXEL_NUM_16x8 * 2;
    HLM_U08  sts         = 1;  // 是否预测成功

    if (pred_mode == HLM_INTRA_16x8_DC)
    {
        if ((HLM_INTRA_MODE_AVAIL_16x8[pred_mode] & ref_flag) == HLM_INTRA_MODE_AVAIL_16x8[pred_mode])
        {
            if (!skip_luma)
            {
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC](ref_pixel_y, pred_pixel[0], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_LUMA_Y], 1 << cu_h[HLM_LUMA_Y]);
            }
            if (!skip_chroma)
            {
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC](ref_pixel_u, pred_pixel[1], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_U], 1 << cu_h[HLM_CHROMA_U]);
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC](ref_pixel_v, pred_pixel[2], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_V], 1 << cu_h[HLM_CHROMA_V]);
            }
        }
        else if ((HLM_INTRA_MODE_AVAIL_16x8[pred_mode] & ref_flag) == HLM_INTRA_MODE_AVAIL_16x8[HLM_INTRA_16x8_DC_LEFT])
        {
            if (!skip_luma)
            {
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC_LEFT](ref_pixel_y, pred_pixel[0], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_LUMA_Y], 1 << cu_h[HLM_LUMA_Y]);
            }
            if (!skip_chroma)
            {
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC_LEFT](ref_pixel_u, pred_pixel[1], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_U], 1 << cu_h[HLM_CHROMA_U]);
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC_LEFT](ref_pixel_v, pred_pixel[2], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_V], 1 << cu_h[HLM_CHROMA_V]);
            }
        }
        else if ((HLM_INTRA_MODE_AVAIL_16x8[pred_mode] & ref_flag) == HLM_INTRA_MODE_AVAIL_16x8[HLM_INTRA_16x8_DC_TOP])
        {
            if (!skip_luma)
            {
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC_TOP](ref_pixel_y, pred_pixel[0], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_LUMA_Y], 1 << cu_h[HLM_LUMA_Y]);
            }
            if (!skip_chroma)
            {
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC_TOP](ref_pixel_u, pred_pixel[1], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_U], 1 << cu_h[HLM_CHROMA_U]);
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC_TOP](ref_pixel_v, pred_pixel[2], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_V], 1 << cu_h[HLM_CHROMA_V]);
            }
        }
        else
        {
            if (!skip_luma)
            {
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC_128](ref_pixel_y, pred_pixel[0], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_LUMA_Y], 1 << cu_h[HLM_LUMA_Y]);
            }
            if (!skip_chroma)
            {
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC_128](ref_pixel_u, pred_pixel[1], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_U], 1 << cu_h[HLM_CHROMA_U]);
                HLM_INTRA_PRED_16x8[HLM_INTRA_16x8_DC_128](ref_pixel_v, pred_pixel[2], bitdepth,
                    HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_V], 1 << cu_h[HLM_CHROMA_V]);
            }
        }
    }
    else
    {
        if (!skip_luma)
        {
            HLM_INTRA_PRED_16x8[pred_mode](ref_pixel_y, pred_pixel[0], bitdepth,
                HLM_WIDTH_SIZE, 1 << cu_w[HLM_LUMA_Y], 1 << cu_h[HLM_LUMA_Y]);
        }
        if (!skip_chroma)
        {
            HLM_INTRA_PRED_16x8[pred_mode](ref_pixel_u, pred_pixel[1], bitdepth,
                HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_U], 1 << cu_h[HLM_CHROMA_U]);
            HLM_INTRA_PRED_16x8[pred_mode](ref_pixel_v, pred_pixel[2], bitdepth,
                HLM_WIDTH_SIZE, 1 << cu_w[HLM_CHROMA_V], 1 << cu_h[HLM_CHROMA_V]);
        }
    }

    return sts;
}

// 计算16x8的代价
HLM_VOID HLMC_INTRA_MODE_cal_16x8(HLMC_REGS             *regs,
                                  HLM_U08                pred_mode,
                                  HLMC_CU_INFO          *cur_cu,
                                  HLM_U08                skip_luma,
                                  HLM_U08                skip_chroma,
                                  HLM_U32               *satd_record_4x4,
                                  HLMC_INTRA_MODE_COST  *cost)
{
    HLM_U32 lamda            = regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]];
    HLM_U32 satd             = 0;
    HLM_U32 record_chroma[8] = { 0 };
    HLM_U16 *ori_y           = HLM_NULL;
    HLM_U16 *ori_u           = HLM_NULL;
    HLM_U16 *ori_v           = HLM_NULL;

    if (!skip_luma)
    {
        ori_y = regs->enc_input_y_base
            + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0]) * regs->enc_input_luma_stride
            + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0]);

        cost->bits = HLMC_INTRA_MODE_BIT_COST_16x8[pred_mode];
        satd = HLMC_COM_ComputeDist16x8(ori_y, cur_cu->com_cu_info.cu_pred_info.pred[0], satd_record_4x4,
            1 << cur_cu->com_cu_info.cu_width[0], 1 << cur_cu->com_cu_info.cu_height[0], regs->enc_input_luma_stride, HLM_WIDTH_SIZE);
        cost->satd += satd;
        cost->satd_comp[0] = satd;
    }
    if (!skip_chroma)
    {
        ori_u = regs->enc_input_cb_base
            + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1]) * regs->enc_input_chroma_stride
            + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1]);
        ori_v = regs->enc_input_cr_base
            + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2]) * regs->enc_input_chroma_stride
            + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2]);

        satd = HLMC_COM_ComputeDist16x8(ori_u, cur_cu->com_cu_info.cu_pred_info.pred[1], record_chroma,
            1 << cur_cu->com_cu_info.cu_width[1], 1 << cur_cu->com_cu_info.cu_height[1], regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
        cost->satd += satd;
        cost->satd_comp[1] = satd;

        satd = HLMC_COM_ComputeDist16x8(ori_v, cur_cu->com_cu_info.cu_pred_info.pred[2], record_chroma,
            1 << cur_cu->com_cu_info.cu_width[2], 1 << cur_cu->com_cu_info.cu_height[2], regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
        cost->satd += satd;
        cost->satd_comp[2] = satd;
    }
#if SAD_SATD_TH_SCALE
    cost->total_cost = cost->satd + ((lamda * cost->bits) >> (HLMC_LAMBDA_SHIFT));
#else
    cost->total_cost = cost->satd + ((lamda * cost->bits) >> (HLMC_LAMBDA_SHIFT + 4));
#endif
}

// 获取4x4的参考像素
HLM_VOID HLMC_INTRA_MODE_get_ref_4x4(HLMC_REGS       *regs,
                                     HLMC_CU_INFO    *cur_cu,
                                     HLM_U08          block_shape,
                                     HLM_U08          component,
                                     HLM_U32          zscan_idx,
                                     HLM_U16         *ref_pixel,
                                     HLM_U08         *ref_flag)
{
    HLM_U32 cu_width        = regs->cur_patch_param->patch_coded_width[0] >> 4;
    HLM_U32 cu_x            = cur_cu->com_cu_info.cu_x;
    HLM_U32 cu_y            = cur_cu->com_cu_info.cu_y;
    HLM_U08 offset_w        = (block_shape == 8) ? 3 : 2;
    HLM_U08 offset_h        = (block_shape == 8) ? 3 : 2;
    HLM_U32 chroma_offset_w = component ? cur_cu->com_cu_info.chroma_offset_x : 0;
    HLM_U32 chroma_offset_h = component ? cur_cu->com_cu_info.chroma_offset_y : 0;
    HLM_U32 pixel_x         = (cur_cu->com_cu_info.cu_x << (4 - chroma_offset_w)) + (HLM_INTRA_ZSCAN_TO_PELX[zscan_idx] << offset_w);
    HLM_U32 pixel_y         = (cur_cu->com_cu_info.cu_y << (3 - chroma_offset_h)) + (HLM_INTRA_ZSCAN_TO_PELY[zscan_idx] << offset_h);
    HLM_U16 *ref            = ref_pixel + component * HLM_INTRA_REF_PIXEL_NUM_4x4;
    HLM_U32 stride_pixel    = component == 0? regs->enc_input_luma_stride: regs->enc_input_chroma_stride;
    HLM_U16 *cu_pixel       = HLM_NULL;
    HLM_U32 i               = 0;

    if (component == 0)
    {
        cu_pixel = regs->enc_input_y_base;
    }
    else if (component == 1)
    {
        cu_pixel = regs->enc_input_cb_base;
    }
    else
    {
        cu_pixel = regs->enc_input_cr_base;
    }
    cu_pixel += stride_pixel * pixel_y + pixel_x;

    if (zscan_idx == 0)
    {
        *ref_flag = (0 == cu_x  ? 0 : HLM_INTRA_LEFT)
            | ( 0 == cu_y ? 0 : HLM_INTRA_TOP | HLM_INTRA_TOP_RIGHT)
            | (((0 == cu_x) || (0 == cu_y)) ? 0 : HLM_INTRA_TOP_LEFT);
    }
    else if ((zscan_idx == 2) || (zscan_idx == 8) || (zscan_idx == 10))
    {
        *ref_flag = (0 == cu_x ? 0 : HLM_INTRA_LEFT | HLM_INTRA_TOP_LEFT)
            | HLM_INTRA_TOP
            | HLM_INTRA_TOP_RIGHT;
    }
    else if ((zscan_idx == 1) || (zscan_idx == 4))
    {
        *ref_flag = (0 == cu_y ? 0 : HLM_INTRA_TOP | HLM_INTRA_TOP_LEFT | HLM_INTRA_TOP_RIGHT)
            | HLM_INTRA_LEFT;
    }
    else if (zscan_idx == 5)
    {
        *ref_flag = (0 == cu_y ? 0 : HLM_INTRA_TOP | HLM_INTRA_TOP_LEFT)
            | (((cu_width - 1 == cu_x) || (0 == cu_y)) ? 0 : HLM_INTRA_TOP_RIGHT)
            | HLM_INTRA_LEFT;
    }
    else if ((zscan_idx == 3) || (zscan_idx == 7) || (zscan_idx == 11) || (zscan_idx == 13) || (zscan_idx == 15))
    {
        *ref_flag = HLM_INTRA_LEFT | HLM_INTRA_TOP | HLM_INTRA_TOP_LEFT;
    }
    else
    {
        *ref_flag = HLM_INTRA_ALL_NEIGHBORS;
    }

    if (cur_cu->com_cu_info.left_unavail && pixel_x == 0)
    {
        *ref_flag &= (HLM_INTRA_TOP | HLM_INTRA_TOP_RIGHT);   // HLM_INTRA_TOP || HLM_INTRA_TOP_RIGHT
    }
    if (cur_cu->com_cu_info.up_unavail && pixel_y == 0)
    {
        *ref_flag &= (HLM_INTRA_LEFT);  // HLM_INTRA_LEFT
    }

    if (*ref_flag & HLM_INTRA_LEFT)
    {
        for (i = 0; i < block_shape; i++)
        {
            ref[HLM_INTRA_LEFT_REF_IDX + i] = *(cu_pixel - 1 + i * stride_pixel);
        }
    }
    if (*ref_flag & HLM_INTRA_TOP)
    {
        memcpy(ref + HLM_INTRA_TOP_REF_IDX, cu_pixel - stride_pixel, block_shape * sizeof(HLM_U16));
        if (*ref_flag & HLM_INTRA_TOP_RIGHT)
        {
            memcpy(ref + HLM_INTRA_TOP_REF_IDX + block_shape, cu_pixel - stride_pixel + block_shape, block_shape * sizeof(HLM_U16));
        }
        else
        {
            for (i = 0; i < block_shape; i++)
            {
                ref[HLM_INTRA_TOP_REF_IDX + block_shape + i] = *(cu_pixel + block_shape - 1 - stride_pixel);
            }
            *ref_flag = *ref_flag | HLM_INTRA_TOP_RIGHT;
        }
    }
    if (*ref_flag & HLM_INTRA_TOP_LEFT)
    {
        ref[HLM_INTRA_TOP_LEFT_REF_IDX] = *(cu_pixel - 1 - stride_pixel);
    }
}

// 生成4x4的预测像素
HLM_U08 HLMC_INTRA_MODE_pred_4x4(HLM_U08     pred_mode,
                                 HLM_U16    *ref_pixel,
                                 HLM_U08     ref_flag,
                                 HLM_U16   **pred_pixel,
                                 HLM_U08     chroma_offset_w,
                                 HLM_U08     chroma_offset_h,
                                 HLM_U08     block_shape,
                                 HLM_U08     skip_luma,
                                 HLM_U08     skip_chroma,
                                 HLM_U16     bitdepth)
{
    HLM_U16 *ref_pixel_y = ref_pixel;
    HLM_U16 *ref_pixel_u = ref_pixel + HLM_INTRA_REF_PIXEL_NUM_4x4;
    HLM_U16 *ref_pixel_v = ref_pixel + HLM_INTRA_REF_PIXEL_NUM_4x4 * 2;
    HLM_U08  sts         = 1;  // 是否预测成功

    if (pred_mode == HLM_INTRA_4x4_DC)
    {
        if ((HLM_INTRA_MODE_AVAIL_4x4[pred_mode] & ref_flag) == HLM_INTRA_MODE_AVAIL_4x4[pred_mode])
        {
            if (!skip_luma)
            {
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC](ref_pixel_y, pred_pixel[0], bitdepth,
                    HLM_WIDTH_SIZE, (HLM_U08)block_shape, (HLM_U08)block_shape);
            }
            if (!skip_chroma)
            {
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC](ref_pixel_u, pred_pixel[1], bitdepth,
                    HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC](ref_pixel_v, pred_pixel[2], bitdepth,
                    HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
            }
        }
        else if ((HLM_INTRA_MODE_AVAIL_4x4[pred_mode] & ref_flag) == HLM_INTRA_MODE_AVAIL_4x4[HLM_INTRA_4x4_DC_LEFT])
        {
            if (!skip_luma)
            {
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC_LEFT](ref_pixel_y, pred_pixel[0], bitdepth,
                    HLM_WIDTH_SIZE, (HLM_U08)block_shape, (HLM_U08)block_shape);
            }
            if (!skip_chroma)
            {
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC_LEFT](ref_pixel_u, pred_pixel[1], bitdepth,
                    HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC_LEFT](ref_pixel_v, pred_pixel[2], bitdepth,
                    HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
            }
        }
        else if ((HLM_INTRA_MODE_AVAIL_4x4[pred_mode] & ref_flag) == HLM_INTRA_MODE_AVAIL_4x4[HLM_INTRA_4x4_DC_TOP])
        {
            if (!skip_luma)
            {
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC_TOP](ref_pixel_y, pred_pixel[0], bitdepth,
                    HLM_WIDTH_SIZE, (HLM_U08)block_shape, (HLM_U08)block_shape);
            }
            if (!skip_chroma)
            {
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC_TOP](ref_pixel_u, pred_pixel[1], bitdepth,
                    HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC_TOP](ref_pixel_v, pred_pixel[2], bitdepth,
                    HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
            }
        }
        else
        {
            if (!skip_luma)
            {
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC_128](ref_pixel_y, pred_pixel[0], bitdepth,
                    HLM_WIDTH_SIZE, (HLM_U08)block_shape, (HLM_U08)block_shape);
            }
            if (!skip_chroma)
            {
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC_128](ref_pixel_u, pred_pixel[1], bitdepth,
                    HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
                HLM_INTRA_PRED_4x4[HLM_INTRA_4x4_DC_128](ref_pixel_v, pred_pixel[2], bitdepth,
                    HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
            }
        }
    }
    else
    {
        if (!skip_luma)
        {
            HLM_INTRA_PRED_4x4[pred_mode](ref_pixel_y, pred_pixel[0], bitdepth,
                HLM_WIDTH_SIZE, (HLM_U08)block_shape, (HLM_U08)block_shape);
        }
        if (!skip_chroma)
        {
            HLM_INTRA_PRED_4x4[pred_mode](ref_pixel_u, pred_pixel[1], bitdepth,
                HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
            HLM_INTRA_PRED_4x4[pred_mode](ref_pixel_v, pred_pixel[2], bitdepth,
                HLM_WIDTH_SIZE, block_shape >> chroma_offset_w, block_shape >> chroma_offset_h);
        }
    }

    return sts;
}

// 计算4x4的代价
HLM_VOID HLMC_INTRA_MODE_cal_4x4(HLMC_REGS             *regs,
                                 HLM_U08                pred_mode,
                                 HLM_U08                mpm,
                                 HLMC_CU_INFO          *cur_cu,
                                 HLM_U08                pu_index,
                                 HLM_U08                skip_luma,
                                 HLM_U08                skip_chroma,
                                 HLM_U08                block_shape,
                                 HLM_U32               *satd_record_4x4,
                                 HLM_U08                block_shape_offset,
                                 HLMC_INTRA_MODE_COST  *cost)
{
    HLM_U32 lamda            = regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]];
    HLM_U32 offset_w         = block_shape == 8 ? 3 : 2;
    HLM_U32 offset_h         = block_shape == 8 ? 3 : 2;
    HLM_U32 pixel_x          = 0;
    HLM_U32 pixel_y          = 0;
    HLM_U16 pu_h             = 0;
    HLM_U16 pu_w             = 0;
    HLM_U16 *ori_y           = HLM_NULL;
    HLM_U16 *ori_u           = HLM_NULL;
    HLM_U16 *ori_v           = HLM_NULL;
    HLM_U32 record_chroma[8] = { 0 };

    if (!skip_luma)
    {
        pixel_x = (cur_cu->com_cu_info.cu_x << 4) + (HLM_INTRA_ZSCAN_TO_PELX[pu_index] << offset_w);
        pixel_y = (cur_cu->com_cu_info.cu_y << 3) + (HLM_INTRA_ZSCAN_TO_PELY[pu_index] << offset_h);
        pu_h = block_shape;
        pu_w = block_shape;
        ori_y = regs->enc_input_y_base + pixel_y * regs->enc_input_luma_stride + pixel_x;

        cost->bits = ((pred_mode == mpm) ? 1 : 4);
        cost->satd += HLMC_COM_ComputeDist16x8(ori_y, cur_cu->com_cu_info.cu_pred_info.pred[0], satd_record_4x4,
            pu_w, pu_h, regs->enc_input_luma_stride, HLM_WIDTH_SIZE);
        cost->satd_comp[0] = cost->satd;
    }
    if (!skip_chroma)
    {
        pixel_x = (cur_cu->com_cu_info.cu_x << (4 - cur_cu->com_cu_info.chroma_offset_x))
                + (HLM_INTRA_ZSCAN_TO_PELX[pu_index] << (offset_w - block_shape_offset));
        pixel_y = (cur_cu->com_cu_info.cu_y << (3 - cur_cu->com_cu_info.chroma_offset_y))
                + (HLM_INTRA_ZSCAN_TO_PELY[pu_index] << (offset_h - block_shape_offset));
        pu_h = block_shape >> block_shape_offset;
        pu_w = block_shape >> block_shape_offset;
        ori_u = regs->enc_input_cb_base + pixel_y * regs->enc_input_chroma_stride + pixel_x;
        ori_v = regs->enc_input_cr_base + pixel_y * regs->enc_input_chroma_stride + pixel_x;

        cost->satd += HLMC_COM_ComputeDist16x8(ori_u, cur_cu->com_cu_info.cu_pred_info.pred[1], record_chroma,
            pu_w, pu_h, regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
        cost->satd_comp[1] = cost->satd - cost->satd_comp[0];
        cost->satd += HLMC_COM_ComputeDist16x8(ori_v, cur_cu->com_cu_info.cu_pred_info.pred[2], record_chroma,
            pu_w, pu_h, regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
        cost->satd_comp[2] = cost->satd - (cost->satd_comp[0] + cost->satd_comp[1]);
    }
#if SAD_SATD_TH_SCALE
    cost->total_cost = cost->satd + ((lamda * cost->bits) >> (HLMC_LAMBDA_SHIFT));
#else
    cost->total_cost = cost->satd + ((lamda * cost->bits) >> (HLMC_LAMBDA_SHIFT + 4));
#endif
}

/***************************************************************************************************
* 功  能：帧内模式选择主函数
* 参  数：*
*        regs                  -I    硬件寄存器
*        nbi_info              -I    当前cu相邻块信息
*        cur_cu                -IO   当前cu信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTRA_MODE(HLMC_REGS            *regs,
                         HLM_NEIGHBOR_INFO    *nbi_info,
                         HLMC_CU_INFO         *cur_cu)
{
    HLMC_INTRA_MODE_COST cost_total_16x8           = { 0 };
    HLMC_INTRA_MODE_COST cost_tmp_16x8             = { 0 };
    HLMC_INTRA_MODE_COST cost_total_4x4            = { 0 };
    HLMC_INTRA_MODE_COST cost_tmp_4x4              = { 0 };
    HLMC_INTRA_MODE_COST cost_one_4x4              = { 0 };
    HLM_U08 ref_flag                               = 0;
    HLM_U16 **pred_pixel_16x8                      = cur_cu->com_cu_info.cu_pred_info.pred;
    HLM_U16 **pred_pixel_4x4                       = cur_cu->com_cu_info.cu_pred_info.pred;
    HLM_U32 pred_mode                              = 0;
    HLM_U08 best_mode_16x8                         = 255;
    HLM_U08 best_mode_4x4[8]                       = { 0 };
    HLM_U32 raster_idx                             = 0;
    HLM_U32 zscan_idx                               = 0;
    HLM_U08 mpm                                    = 0;
    HLM_U08 yuv_idx                                = 0;
    HLM_U32 satd_record_4x4[HLM_CU_SIZE >> 4]      = { 0 };
    HLM_S32 yuv_comp                               = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_U32 satd_record_4x4_temp[HLM_CU_SIZE >> 4] = { 0 };
    HLM_U16 k                                      = 0;
    HLM_U16 default_value                          = 1 << (regs->bitdepth - 1);
    HLM_U08 sub_block_size                         = regs->intra_8x8_enable_flag == 0 ? 4 : 8;
    HLM_U08 block_num                              = HLM_CU_SIZE / (sub_block_size * sub_block_size);
    HLM_U08 block_4x4_num                          = 8 / block_num;
    HLM_U32 skip_chroma[2]                         = { 0, 0 };
    HLM_U08 skip_y                                 = 0;
    HLM_U08 skip_c                                 = 0;
    HLM_U32 chroma_size                            = sub_block_size;
    HLM_U32 chroma_size_offset                     = 0;
    HLM_U16 ref_pixel_16x8[HLM_INTRA_REF_PIXEL_NUM_16x8 * 3] = { 0 };
    HLM_U16 ref_pixel_4x4[HLM_INTRA_REF_PIXEL_NUM_4x4 * 3] = { 0 };

    // 16x8
    cost_total_16x8.total_cost = HLM_MAX_U32;
    memset(ref_pixel_16x8, 0, 3 * HLM_INTRA_REF_PIXEL_NUM_16x8 * sizeof(HLM_U16));
    for (k = 0; k < HLM_INTRA_REF_PIXEL_NUM_16x8 * 3; k++)
    {
        ref_pixel_16x8[k] = default_value;
    }
    HLMC_INTRA_MODE_get_ref_16x8(regs, cur_cu, ref_pixel_16x8, &ref_flag);

#if INTRA_FAST
    skip_y = 0;
    skip_c = 1;
#else
    skip_y = 0;
    skip_c = (regs->image_format == HLM_IMG_YUV_400) ? 1 : 0;
#endif
    for (pred_mode = 0; pred_mode < HLM_INTRA_PRED_MODE_NUM_16x8; pred_mode++)
    {
        memset(&cost_tmp_16x8, 0, sizeof(HLMC_INTRA_MODE_COST));
        memset(&satd_record_4x4_temp, 0, sizeof(HLM_U32) * 8);
        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            memset(pred_pixel_16x8[yuv_idx], 0, 16 * 8 * sizeof(HLM_U16));
        }
        if (1 == HLMC_INTRA_MODE_pred_16x8(pred_mode, ref_pixel_16x8, ref_flag, regs->bitdepth,
            cur_cu->com_cu_info.cu_width, cur_cu->com_cu_info.cu_height, skip_y, skip_c, pred_pixel_16x8))
        {
            HLMC_INTRA_MODE_cal_16x8(regs, pred_mode, cur_cu, skip_y, skip_c, satd_record_4x4_temp, &cost_tmp_16x8);
            if (cost_tmp_16x8.total_cost < cost_total_16x8.total_cost ||
                (cost_tmp_16x8.satd == cost_total_16x8.satd && pred_mode == HLM_INTRA_16x8_DC))
            {
                memcpy(satd_record_4x4, satd_record_4x4_temp, (HLM_CU_SIZE >> 4) * sizeof(HLM_U32));
                best_mode_16x8               = pred_mode;
                cost_total_16x8.bits         = cost_tmp_16x8.bits;
                cost_total_16x8.satd         = cost_tmp_16x8.satd;
                cost_total_16x8.total_cost   = cost_tmp_16x8.total_cost;
                cost_total_16x8.satd_comp[0] = cost_tmp_16x8.satd_comp[0];
                cost_total_16x8.satd_comp[1] = cost_tmp_16x8.satd_comp[1];
                cost_total_16x8.satd_comp[2] = cost_tmp_16x8.satd_comp[2];
            }
        }
    }

#if INTRA_FAST
    // 色度分量satd计算
    if (regs->image_format != HLM_IMG_YUV_400)
    {
        skip_y = 1;
        skip_c = 0;
        if (1 == HLMC_INTRA_MODE_pred_16x8(best_mode_16x8, ref_pixel_16x8, ref_flag, regs->bitdepth,
            cur_cu->com_cu_info.cu_width, cur_cu->com_cu_info.cu_height, skip_y, skip_c, pred_pixel_16x8))
        {
            HLMC_INTRA_MODE_cal_16x8(regs, best_mode_16x8, cur_cu, skip_y, skip_c, satd_record_4x4_temp, &cost_total_16x8);
        }
    }
#endif

    if (best_mode_16x8 != 255)
    {
        cur_cu->pu_info_enc[0].inter_pu_info.intra_pred_mode = best_mode_16x8;
    }

    // 8x8或4x4
    cost_total_4x4.total_cost = HLM_MAX_U32;
    if (regs->intra_8x8_enable_flag == 0)  // 4x4
    {
#if SAD_SATD_TH_SCALE
        cost_total_4x4.total_cost = (72 * regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]]) >> (HLMC_LAMBDA_SHIFT);
#else
        cost_total_4x4.total_cost = (72 * regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]]) >> (HLMC_LAMBDA_SHIFT + 4);
#endif
        for (raster_idx = 0; raster_idx < 8; raster_idx++)
        {
            zscan_idx = HLM_INTRA_RASTER_TO_ZSCAN[raster_idx];
            cost_one_4x4.total_cost = HLM_MAX_U32;
            for (k = 0; k < HLM_INTRA_REF_PIXEL_NUM_4x4 * 3; k++)
            {
                ref_pixel_4x4[k] = default_value;
            }
            best_mode_4x4[raster_idx] = 255;
            HLMC_INTRA_MODE_get_ref_4x4(regs, cur_cu, sub_block_size, 0, zscan_idx, ref_pixel_4x4, &ref_flag);
            HLMC_INTRA_MODE_get_ref_4x4(regs, cur_cu, sub_block_size, 1, zscan_idx, ref_pixel_4x4, &ref_flag);
            HLMC_INTRA_MODE_get_ref_4x4(regs, cur_cu, sub_block_size, 2, zscan_idx, ref_pixel_4x4, &ref_flag);
            HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, zscan_idx, &mpm, PROC_PRE);
#if INTRA_FAST
            skip_y = 0;
            skip_c = 1;
#else
            skip_y = 0;
            skip_c = (regs->image_format == HLM_IMG_YUV_400) ? 1 : 0;
#endif
            for (pred_mode = 0; pred_mode < HLM_INTRA_PRED_MODE_NUM_4x4; pred_mode++)
            {
                memset(&cost_tmp_4x4, 0, sizeof(HLMC_INTRA_MODE_COST));
                memset(&satd_record_4x4_temp, 0, sizeof(HLM_U32) * 8);
                if (1 == HLMC_INTRA_MODE_pred_4x4(pred_mode, ref_pixel_4x4, ref_flag, pred_pixel_4x4,
                    cur_cu->com_cu_info.chroma_offset_x, cur_cu->com_cu_info.chroma_offset_y,
                    sub_block_size, skip_y, skip_c, regs->bitdepth))
                {
                    HLMC_INTRA_MODE_cal_4x4(regs, pred_mode, mpm, cur_cu, zscan_idx, skip_y, skip_c,
                        sub_block_size, satd_record_4x4_temp + raster_idx, 0, &cost_tmp_4x4);
                    if (cost_tmp_4x4.total_cost < cost_one_4x4.total_cost ||
                        ((cost_tmp_4x4.satd == cost_one_4x4.satd) && (pred_mode == HLM_INTRA_4x4_DC)))
                    {
                        satd_record_4x4[raster_idx] = satd_record_4x4_temp[raster_idx];
                        best_mode_4x4[raster_idx]   = pred_mode;
                        cost_one_4x4.total_cost     = cost_tmp_4x4.total_cost;
                        cost_one_4x4.bits           = cost_tmp_4x4.bits;
                        cost_one_4x4.satd           = cost_tmp_4x4.satd;
                        cost_one_4x4.satd_comp[0]   = cost_tmp_4x4.satd_comp[0];
                        cost_one_4x4.satd_comp[1]   = cost_tmp_4x4.satd_comp[1];
                        cost_one_4x4.satd_comp[2]   = cost_tmp_4x4.satd_comp[2];
                    }
                }
            }

#if INTRA_FAST
            // 色度分量satd计算
            if (regs->image_format != HLM_IMG_YUV_400)
            {
                skip_y = 1;
                skip_c = 0;
                if (1 == HLMC_INTRA_MODE_pred_4x4(best_mode_4x4[raster_idx], ref_pixel_4x4, ref_flag, pred_pixel_4x4,
                    cur_cu->com_cu_info.chroma_offset_x, cur_cu->com_cu_info.chroma_offset_y,
                    sub_block_size, skip_y, skip_c, regs->bitdepth))
                {
                    HLMC_INTRA_MODE_cal_4x4(regs, best_mode_4x4[raster_idx], mpm, cur_cu, zscan_idx, skip_y, skip_c,
                        sub_block_size, satd_record_4x4_temp + zscan_idx * block_4x4_num, chroma_size_offset, &cost_one_4x4);
                }
            }
#endif

            if (best_mode_4x4[raster_idx] != 255)
            {
                cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].intra_pred_mode = best_mode_4x4[raster_idx];
                cost_total_4x4.total_cost   += cost_one_4x4.total_cost;
                cost_total_4x4.bits         += cost_one_4x4.bits;
                cost_total_4x4.satd         += cost_one_4x4.satd;
                cost_total_4x4.satd_comp[0] += cost_one_4x4.satd_comp[0];
                cost_total_4x4.satd_comp[1] += cost_one_4x4.satd_comp[1];
                cost_total_4x4.satd_comp[2] += cost_one_4x4.satd_comp[2];
            }
        }
    }
    else  // 8x8
    {
        HLM_COM_Intra8x8ChromaPb(regs->image_format, sub_block_size, skip_chroma, &chroma_size, &chroma_size_offset);
#if SAD_SATD_TH_SCALE
        cost_total_4x4.total_cost = (72 * regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]]) >> (HLMC_LAMBDA_SHIFT);
#else
        cost_total_4x4.total_cost = (72 * regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]]) >> (HLMC_LAMBDA_SHIFT + 4);
#endif
        for (raster_idx = 0; raster_idx < block_num; raster_idx++)
        {
            zscan_idx = HLM_INTRA_RASTER_TO_ZSCAN[raster_idx];
            cost_one_4x4.total_cost = HLM_MAX_U32;
            for (k = 0; k < HLM_INTRA_REF_PIXEL_NUM_4x4 * 3; k++)
            {
                ref_pixel_4x4[k] = default_value;
            }
            best_mode_4x4[raster_idx] = 255;
            HLMC_INTRA_MODE_get_ref_4x4(regs, cur_cu, sub_block_size, 0, zscan_idx, ref_pixel_4x4, &ref_flag);
            if (!skip_chroma[raster_idx])
            {
                HLMC_INTRA_MODE_get_ref_4x4(regs, cur_cu, chroma_size, 1, zscan_idx, ref_pixel_4x4, &ref_flag);
                HLMC_INTRA_MODE_get_ref_4x4(regs, cur_cu, chroma_size, 2, zscan_idx, ref_pixel_4x4, &ref_flag);
            }
            HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, zscan_idx, &mpm, PROC_PRE);
#if INTRA_FAST
            skip_y = 0;
            skip_c = 1;
#else
            skip_y = 0;
            skip_c = skip_chroma[raster_idx];
#endif
            for (pred_mode = 0; pred_mode < HLM_INTRA_PRED_MODE_NUM_4x4; pred_mode++)
            {
                memset(&cost_tmp_4x4, 0, sizeof(HLMC_INTRA_MODE_COST));
                memset(&satd_record_4x4_temp, 0, sizeof(HLM_U32) * 8);
                if (1 == HLMC_INTRA_MODE_pred_4x4(pred_mode, ref_pixel_4x4, ref_flag, pred_pixel_4x4,
                    chroma_size_offset, chroma_size_offset, sub_block_size, skip_y, skip_c, regs->bitdepth))
                {
                    HLMC_INTRA_MODE_cal_4x4(regs, pred_mode, mpm, cur_cu, zscan_idx, skip_y, skip_c,
                        sub_block_size, satd_record_4x4_temp + raster_idx * block_4x4_num, chroma_size_offset, &cost_tmp_4x4);
                    if (cost_tmp_4x4.total_cost < cost_one_4x4.total_cost ||
                        ((cost_tmp_4x4.satd == cost_one_4x4.satd) && (pred_mode == HLM_INTRA_4x4_DC)))
                    {
                        memcpy(satd_record_4x4 + raster_idx * block_4x4_num,
                            satd_record_4x4_temp + raster_idx * block_4x4_num, block_4x4_num * sizeof(HLM_U32));
                        best_mode_4x4[raster_idx] = pred_mode;
                        cost_one_4x4.total_cost   = cost_tmp_4x4.total_cost;
                        cost_one_4x4.bits         = cost_tmp_4x4.bits;
                        cost_one_4x4.satd         = cost_tmp_4x4.satd;
                        cost_one_4x4.satd_comp[0] = cost_tmp_4x4.satd_comp[0];
                        cost_one_4x4.satd_comp[1] = cost_tmp_4x4.satd_comp[1];
                        cost_one_4x4.satd_comp[2] = cost_tmp_4x4.satd_comp[2];
                    }
                }
            }

#if INTRA_FAST
            // 色度分量satd计算
            if (regs->image_format != HLM_IMG_YUV_400)
            {
                skip_y = 1;
                skip_c = skip_chroma[raster_idx];
                if (1 == HLMC_INTRA_MODE_pred_4x4(best_mode_4x4[raster_idx], ref_pixel_4x4, ref_flag, pred_pixel_4x4,
                    cur_cu->com_cu_info.chroma_offset_x, cur_cu->com_cu_info.chroma_offset_y,
                    sub_block_size, skip_y, skip_c, regs->bitdepth))
                {
                    HLMC_INTRA_MODE_cal_4x4(regs, best_mode_4x4[raster_idx], mpm, cur_cu, zscan_idx, skip_y, skip_c,
                        sub_block_size, satd_record_4x4_temp + zscan_idx * block_4x4_num, chroma_size_offset, &cost_one_4x4);
                }
            }
#endif

            if (best_mode_4x4[raster_idx] != 255)
            {
                cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].intra_pred_mode = best_mode_4x4[raster_idx];
                cost_total_4x4.total_cost   += cost_one_4x4.total_cost;
                cost_total_4x4.bits         += cost_one_4x4.bits;
                cost_total_4x4.satd         += cost_one_4x4.satd;
                cost_total_4x4.satd_comp[0] += cost_one_4x4.satd_comp[0];
                cost_total_4x4.satd_comp[1] += cost_one_4x4.satd_comp[1];
                cost_total_4x4.satd_comp[2] += cost_one_4x4.satd_comp[2];
            }
        }
    }

    // 检测4x4块的stad差距进行判定
    cur_cu->mix_flag = HLMC_RC_CalMixFlag(satd_record_4x4, regs->bitdepth);
    if (cost_total_4x4.total_cost < cost_total_16x8.total_cost)
    {
        cur_cu->com_cu_info.cu_type = HLM_I_4x4;
        cur_cu->intra_satd_cost     = cost_total_4x4.satd;
        cur_cu->satd_comp[0]        = cost_total_4x4.satd_comp[0];
        cur_cu->satd_comp[1]        = cost_total_4x4.satd_comp[1];
        cur_cu->satd_comp[2]        = cost_total_4x4.satd_comp[2];
#if LINE_BY_LINE
        cur_cu->intra_mode_cost = cost_total_4x4.total_cost;
#endif
    }
    else
    {
        cur_cu->com_cu_info.cu_type = HLM_I_16x8;
        cur_cu->intra_satd_cost     = cost_total_16x8.satd;
#if LINE_BY_LINE
        cur_cu->intra_mode_cost = cost_total_4x4.total_cost;
#endif
        cur_cu->satd_comp[0]        = cost_total_16x8.satd_comp[0];
        cur_cu->satd_comp[1]        = cost_total_16x8.satd_comp[1];
        cur_cu->satd_comp[2]        = cost_total_16x8.satd_comp[2];
    }
#if  IBC_SCALE
    cur_cu->best_cu_type = cur_cu->com_cu_info.cu_type;
#endif
}
#if LINE_BY_LINE
/***************************************************************************************************
* 功  能：逐行逐列模式初筛主函数
* 参  数：*
*        regs               -I        硬件寄存器
*        nbi_info           -I        当前ctu相邻块信息
*        cur_cu             -IO       当前ctu信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_LINE_BY_LINE_MODE(HLMC_REGS            *regs,
                                HLM_NEIGHBOR_INFO   *nbi_info,
                                HLMC_CU_INFO         *cur_cu)
{
    HLM_U32 satd = 0;
    HLM_U08 yuv_idx = 0;
    HLM_U08 line = 0;
    HLM_U08 i = 0;
    HLM_U16 *src = 0;
    HLM_U16 *dst = 0;
    HLM_U16 ref_pixel_16x1[16 + 3] = { 0 };
    HLM_U08 pred_mode = 0;
    HLM_U08 num_comp = (regs->image_format == HLM_IMG_YUV_400) ? 1 : 3;
    HLM_U32 satd_record_4x4_temp[HLM_CU_SIZE >> 4] = { 0 };
    HLM_U32 satd_record_4x4[HLM_CU_SIZE >> 4] = { 0 };
#if  LINE_BY_LINE_4x1
    HLM_U08 pred_lenght = 4;
    HLM_U08 best_mode_4x1[4] = { 0 };
    HLM_U08 best_mode_1x4[4] = { 0 };
    HLMC_INTRA_MODE_COST cost_tmp_4x1[4] = { 0 };
    HLMC_INTRA_MODE_COST cost_total_4x1[4] = { 0 };
    HLMC_INTRA_MODE_COST cost_total_1x4[4] = { 0 };
    for (i = 0; i < 16 / pred_lenght; i++)
    {
        cost_total_4x1[i].total_cost = HLM_MAX_U32;
        cost_total_1x4[i].total_cost = HLM_MAX_U32;
    }

#endif
    HLM_U08 best_mode_16x1 = 0;
    HLMC_INTRA_MODE_COST cost_tmp_16x1 = { 0 };
    HLMC_INTRA_MODE_COST cost_total_16x1 = { 0 };
    cost_total_16x1.total_cost = HLM_MAX_U32;
#if LINE_BY_LINE_ONE_MODE
    for (pred_mode = 0; pred_mode < 1; pred_mode++)
#else
    for (pred_mode = 0; pred_mode < HLM_INTRA_PRED_MODE_NUM_16x8; pred_mode++)
#endif
    {
#if  LINE_BY_LINE_4x1
        HLM_U08 tmp_mode_4x1[4] = { pred_mode,pred_mode,pred_mode,pred_mode };
#endif
        for (line = 0; line < 1 << cur_cu->com_cu_info.cu_height[yuv_idx]; line++)
        {
            HLMC_INTRA_MODE_get_ref_pixel_16x1(regs,
                cur_cu,
                ref_pixel_16x1,
                line,
                1,
#if  LINE_BY_LINE_4x1
                pred_lenght,
                tmp_mode_4x1);
#else
                &pred_mode);
#endif


        }
        // HLMC_INTRA_MODE_cal_16x8(regs, pred_mode, cur_cu, regs->image_format == HLM_IMG_YUV_400, satd_record_4x4_temp, &cost_tmp_16x1);
        HLM_U16 *ori_y = regs->enc_input_y_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0])* regs->enc_input_luma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0]);
#if  LINE_BY_LINE_4x1
        for (i = 0; i < (1 << cur_cu->com_cu_info.cu_width[0])/ pred_lenght; i++ )
        {
            cost_tmp_4x1[i].satd = 0;
            cost_tmp_4x1[i].satd += HLMC_COM_ComputeSad(ori_y + i * pred_lenght, cur_cu->com_cu_info.cu_pred_info.pred[0] + i * pred_lenght,
                pred_lenght, 1 << cur_cu->com_cu_info.cu_height[0], regs->enc_input_luma_stride, HLM_WIDTH_SIZE);
        }

#if !INTRA_FAST ||1
        if (num_comp > 1)
        {
            HLM_U16 *ori_u = regs->enc_input_cb_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1])* regs->enc_input_chroma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1]);
            HLM_U16 *ori_v = regs->enc_input_cr_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2])* regs->enc_input_chroma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2]);
            HLM_U16 chroma_length = pred_lenght >> cur_cu->com_cu_info.chroma_offset_x;
            for (i = 0; i < (1 << cur_cu->com_cu_info.cu_width[1])/ chroma_length; i++)
            {
                cost_tmp_4x1[i].satd += HLMC_COM_ComputeSad(ori_u + i * chroma_length, cur_cu->com_cu_info.cu_pred_info.pred[1] + i * chroma_length,
                    chroma_length, 1 << cur_cu->com_cu_info.cu_height[1], regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
            }
            for (i = 0; i < (1 << cur_cu->com_cu_info.cu_width[1]) / chroma_length; i++)
            {
                cost_tmp_4x1[i].satd += HLMC_COM_ComputeSad(ori_v + i * chroma_length, cur_cu->com_cu_info.cu_pred_info.pred[2] + i * chroma_length,
                    chroma_length, 1 << cur_cu->com_cu_info.cu_height[2], regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
            }
        }
#endif
        for (i = 0; i < 4; i++)
        {
            cost_tmp_4x1[i].total_cost = cost_tmp_4x1[i].satd;

            if (cost_tmp_4x1[i].total_cost < cost_total_4x1[i].total_cost ||
                (cost_tmp_4x1[i].satd == cost_total_4x1[i].satd && (pred_mode == HLM_INTRA_16x8_DC)))
            {
                cost_total_4x1[i].total_cost   = cost_tmp_4x1[i].total_cost;
                cost_total_4x1[i].satd         = cost_tmp_4x1[i].satd;
                cost_total_4x1[i].satd_comp[0] = cost_tmp_4x1[i].satd_comp[0];
                cost_total_4x1[i].satd_comp[1] = cost_tmp_4x1[i].satd_comp[1];
                cost_total_4x1[i].satd_comp[2] = cost_tmp_4x1[i].satd_comp[2];
                best_mode_4x1[i] = pred_mode;
            }
        }
#else
        cost_tmp_16x1.satd = 0;
        cost_tmp_16x1.satd += HLMC_COM_ComputeSad(ori_y, cur_cu->com_cu_info.cu_pred_info.pred[0], 1 << cur_cu->com_cu_info.cu_width[0], 1 << cur_cu->com_cu_info.cu_height[0], regs->enc_input_luma_stride, HLM_WIDTH_SIZE);

#if !INTRA_FAST
        if (num_comp > 1)
        {
            HLM_U16 *ori_u = regs->enc_input_cb_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1])* regs->enc_input_chroma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1]);
            HLM_U16 *ori_v = regs->enc_input_cr_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2])* regs->enc_input_chroma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2]);
            cost_tmp_16x1.satd += HLMC_COM_ComputeSad(ori_u, cur_cu->com_cu_info.cu_pred_info.pred[1], 1 << cur_cu->com_cu_info.cu_width[1], 1 << cur_cu->com_cu_info.cu_height[1], regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
            cost_tmp_16x1.satd += HLMC_COM_ComputeSad(ori_v, cur_cu->com_cu_info.cu_pred_info.pred[2], 1 << cur_cu->com_cu_info.cu_width[2], 1 << cur_cu->com_cu_info.cu_height[2], regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
        }
#endif
        cost_tmp_16x1.total_cost = cost_tmp_16x1.satd;

        if (cost_tmp_16x1.total_cost < cost_total_16x1.total_cost ||
            ((cost_tmp_16x1.satd == cost_total_16x1.satd) && (pred_mode == HLM_INTRA_16x8_DC)))
        {
            cost_total_16x1.total_cost = cost_tmp_16x1.total_cost;
            cost_total_16x1.satd = cost_tmp_16x1.satd;
            cost_total_16x1.satd_comp[0] = cost_tmp_16x1.satd_comp[0];
            cost_total_16x1.satd_comp[1] = cost_tmp_16x1.satd_comp[1];
            cost_total_16x1.satd_comp[2] = cost_tmp_16x1.satd_comp[2];

            best_mode_16x1 = pred_mode;
        }
#endif
    }
#if  LINE_BY_LINE_4x1
    cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[0] = best_mode_4x1[0];
    cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[1] = best_mode_4x1[1];
    cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[2] = best_mode_4x1[2];
    cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[3] = best_mode_4x1[3];
#endif
#if LINE_BY_LINE_ONE_MODE
    for (pred_mode = 0; pred_mode < 1; pred_mode++)
#else
    for (pred_mode = 0; pred_mode < HLM_INTRA_PRED_MODE_NUM_16x8; pred_mode++)
#endif
    {
        cost_tmp_16x1.satd = 0;
#if  LINE_BY_LINE_4x1
        HLM_U08 tmp_mode_4x1[4] = { pred_mode,pred_mode,pred_mode,pred_mode };
#endif
        for (line = 0; line < 1 << cur_cu->com_cu_info.cu_width[yuv_idx]; line++)
        {
            HLMC_INTRA_MODE_get_ref_pixel_1x8(regs,
                cur_cu,
                ref_pixel_16x1,
                line,
                1,
#if  LINE_BY_LINE_4x1
                pred_lenght,
                tmp_mode_4x1);
#else
                &pred_mode);
#endif

        }
       
        HLM_U16 *ori_y = regs->enc_input_y_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0])* regs->enc_input_luma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0]);
#if  LINE_BY_LINE_4x1
        for (i = 0; i < (1 << cur_cu->com_cu_info.cu_height[0]) / pred_lenght; i++)
        {
            cost_tmp_4x1[i].satd = 0;
            cost_tmp_4x1[i].satd += HLMC_COM_ComputeSad(ori_y + (i * pred_lenght) * regs->enc_input_luma_stride, cur_cu->com_cu_info.cu_pred_info.pred[0] + (i * pred_lenght)*HLM_WIDTH_SIZE,
                HLM_WIDTH_SIZE, pred_lenght, regs->enc_input_luma_stride, HLM_WIDTH_SIZE);
        }
#if !INTRA_FAST ||1
        if (num_comp > 1)
        {
            HLM_U16 *ori_u = regs->enc_input_cb_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1])* regs->enc_input_chroma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1]);
            HLM_U16 *ori_v = regs->enc_input_cr_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2])* regs->enc_input_chroma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2]);
            HLM_U16 chroma_length = pred_lenght >> cur_cu->com_cu_info.chroma_offset_x;
            for (i = 0; i < (1 << cur_cu->com_cu_info.cu_height[1]) / chroma_length; i++)
            {
                cost_tmp_4x1[i].satd += HLMC_COM_ComputeSad(ori_u + (i * chroma_length)*regs->enc_input_chroma_stride, cur_cu->com_cu_info.cu_pred_info.pred[1] + (i * chroma_length)*HLM_WIDTH_SIZE,
                    1 << cur_cu->com_cu_info.cu_width[1], chroma_length, regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
            }
            for (i = 0; i < (1 << cur_cu->com_cu_info.cu_height[1]) / chroma_length; i++)
            {
                cost_tmp_4x1[i].satd += HLMC_COM_ComputeSad(ori_v + (i * chroma_length)*regs->enc_input_chroma_stride, cur_cu->com_cu_info.cu_pred_info.pred[2] + (i * chroma_length)*HLM_WIDTH_SIZE,
                    1 << cur_cu->com_cu_info.cu_width[2], chroma_length, regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
            }
        }
        for (i = 0; i < 2; i++)
        {
            cost_tmp_4x1[i].total_cost = cost_tmp_4x1[i].satd;

            if (cost_tmp_4x1[i].total_cost < cost_total_1x4[i].total_cost ||
                (cost_tmp_4x1[i].satd == cost_total_1x4[i].satd && (pred_mode == HLM_INTRA_16x8_DC)))
            {
                cost_total_1x4[i].total_cost = cost_tmp_4x1[i].total_cost;
                cost_total_1x4[i].satd = cost_tmp_4x1[i].satd;
                cost_total_1x4[i].satd_comp[0] = cost_tmp_4x1[i].satd_comp[0];
                cost_total_1x4[i].satd_comp[1] = cost_tmp_4x1[i].satd_comp[1];
                cost_total_1x4[i].satd_comp[2] = cost_tmp_4x1[i].satd_comp[2];
                best_mode_1x4[i] = pred_mode;
            }
        }
#endif
#else
        cost_tmp_16x1.satd += HLMC_COM_ComputeSad(ori_y, cur_cu->com_cu_info.cu_pred_info.pred[0], 1 << cur_cu->com_cu_info.cu_width[0], 1 << cur_cu->com_cu_info.cu_height[0], regs->enc_input_luma_stride, HLM_WIDTH_SIZE);
#if !INTRA_FAST
        HLM_U16 *ori_u = regs->enc_input_cb_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1])* regs->enc_input_chroma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1]);
        HLM_U16 *ori_v = regs->enc_input_cr_base + (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2])* regs->enc_input_chroma_stride + (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2]);
        cost_tmp_16x1.satd += HLMC_COM_ComputeSad(ori_u, cur_cu->com_cu_info.cu_pred_info.pred[1], 1 << cur_cu->com_cu_info.cu_width[1], 1 << cur_cu->com_cu_info.cu_height[1], regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
        cost_tmp_16x1.satd += HLMC_COM_ComputeSad(ori_v, cur_cu->com_cu_info.cu_pred_info.pred[2], 1 << cur_cu->com_cu_info.cu_width[2], 1 << cur_cu->com_cu_info.cu_height[2], regs->enc_input_chroma_stride, HLM_WIDTH_SIZE);
#endif
        cost_tmp_16x1.total_cost = cost_tmp_16x1.satd;
        if (cost_tmp_16x1.total_cost < cost_total_16x1.total_cost ||
            ((cost_tmp_16x1.satd == cost_total_16x1.satd) && (pred_mode == HLM_INTRA_16x8_DC)))
        {
            cost_total_16x1.total_cost = cost_tmp_16x1.total_cost;
            cost_total_16x1.satd = cost_tmp_16x1.satd;
            cost_total_16x1.satd_comp[0] = cost_tmp_16x1.satd_comp[0];
            cost_total_16x1.satd_comp[1] = cost_tmp_16x1.satd_comp[1];
            cost_total_16x1.satd_comp[2] = cost_tmp_16x1.satd_comp[2];

            best_mode_16x1 = pred_mode + HLM_INTRA_PRED_MODE_NUM_16x8;
        }
#endif
    }
#if  LINE_BY_LINE_4x1
    cur_cu->pu_info_enc[0].inter_pu_info.row_by_row_mode[0] = best_mode_1x4[0];
    cur_cu->pu_info_enc[0].inter_pu_info.row_by_row_mode[1] = best_mode_1x4[1];
#else
    if (best_mode_16x1 != 255)
    {
        cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[0] = best_mode_16x1;
    }
#endif
    HLM_U32 lamda = regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]];
    
#if  LINE_BY_LINE_4x1
        for (line = 0; line < 1 << cur_cu->com_cu_info.cu_width[yuv_idx]; line++)
        {
            HLMC_INTRA_MODE_get_ref_pixel_1x8(regs,
                cur_cu,
                ref_pixel_16x1,
                line,
                1,
                pred_lenght,
                best_mode_1x4);
        }
        cost_total_4x1[0].satd = 0;
        HLMC_INTRA_MODE_cal_16x8(regs, pred_mode, cur_cu, 0, 0, satd_record_4x4_temp, &cost_total_4x1[0]);
        cost_total_4x1[0].bits = 2 + 4;
        cost_total_4x1[0].total_cost = cost_total_4x1[0].satd + ((lamda * cost_total_4x1[0].bits) >> (HLMC_LAMBDA_SHIFT));
        if (cost_total_4x1[0].total_cost < cur_cu->intra_mode_cost)
        {
            cur_cu->com_cu_info.line_row_pread_mode = HLM_I_ROW;
            cur_cu->intra_mode_cost = cost_total_4x1[0].total_cost;
            cur_cu->intra_satd_cost = cost_total_4x1[0].satd;
            cur_cu->satd_comp[0] = cost_total_4x1[0].satd_comp[0];
            cur_cu->satd_comp[1] = cost_total_4x1[0].satd_comp[1];
            cur_cu->satd_comp[2] = cost_total_4x1[0].satd_comp[2];
        }

        for (line = 0; line < 1 << cur_cu->com_cu_info.cu_height[yuv_idx]; line++)
        {
            HLMC_INTRA_MODE_get_ref_pixel_16x1(regs,
                cur_cu,
                ref_pixel_16x1,
                line,
                1,
                pred_lenght,
                best_mode_4x1);
        }
        cost_total_4x1[0].satd = 0;
        HLMC_INTRA_MODE_cal_16x8(regs, pred_mode, cur_cu, 0, 0, satd_record_4x4_temp, &cost_total_4x1[0]);
        cost_total_4x1[0].bits = 2 + 8;
        cost_total_4x1[0].total_cost = cost_total_4x1[0].satd + ((lamda * cost_total_4x1[0].bits) >> (HLMC_LAMBDA_SHIFT));
        if (cost_total_4x1[0].total_cost < cur_cu->intra_satd_cost)
        {

            cur_cu->com_cu_info.line_row_pread_mode = HLM_I_LINE;
            cur_cu->intra_mode_cost = cost_total_4x1[0].total_cost;
            cur_cu->intra_satd_cost = cost_total_4x1[0].satd;
            cur_cu->satd_comp[0] = cost_total_4x1[0].satd_comp[0];
            cur_cu->satd_comp[1] = cost_total_4x1[0].satd_comp[1];
            cur_cu->satd_comp[2] = cost_total_4x1[0].satd_comp[2];
        }

#endif
}
#endif
// BV搜索
HLM_VOID HLMC_SCC_ibc_search(HLM_U16      *ori_ptr[3],
                             HLM_U32       stride[3],
                             HLM_U16      *search_area,
                             HLM_U08       num_comp,
                             HLM_S32       pu_x_in_pic,
                             HLM_S32       pu_y_in_pic,
                             HLM_S32       pu_x_in_area,
                             HLM_S32       pu_y_in_area,
                             HLMC_CU_INFO *cur_cu,
                             HLM_U32       lamda,
                             HLM_U08       zscan_idx,
                             HLM_U08       segment_enable_flag,
                             HLM_U32       segment_width_in_log2)
{
#if MIX_IBC
    HLM_S32 bvx_bits              = 0;
#else
    HLM_S32 bvx_bits              = (cur_cu->com_cu_info.cu_x >= 8) ? HLM_IBC_HOR_SEARCH_LOG : tbl_bvx_bits[cur_cu->com_cu_info.cu_x];
#endif
    HLM_U16 yuv_idx               = 0;
    HLM_U16 blk_org_4x4[3][16]    = { { 0 } };
    HLM_U16 blk_pred_4x4[3][16]   = { { 0 } };
    HLM_U16 blk_org_16x8[3][128]  = { { 0 } };
    HLM_U16 blk_pred_16x8[3][128] = { { 0 } };
    HLM_U08 merge_flag            = 0;
    HLM_U08 direct                = 0;
    HLM_S32 pu_x                  = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
    HLM_S32 pu_y                  = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
    HLM_S32 bvx_offset            = ((zscan_idx & 1) + 1) << 2;
    HLM_S16 best_x[4]             = { 0 };
    HLM_S16 best_y[4]             = { 0 };
    HLM_S16 i                     = 0;
    HLM_S16 cur_x                 = 0;
    HLM_S16 cur_y                 = 0;
    HLM_S16 start_x               = 0;
    HLM_S16 start_y               = 0;
    HLM_S16 end_x                 = pu_x_in_area - bvx_offset;
    HLM_S16 end_y                 = HLM_IBC_SEARCH_AREA_HEIGHT - 4;
    HLM_S32 cost                  = 0;
    HLM_S32 satd                  = 0;
    HLM_S32 bits                  = 0;
    HLM_S32 best_cost[4]          = { HLM_MAX_S32, HLM_MAX_S32, HLM_MAX_S32, HLM_MAX_S32 };
    HLM_S32 best_satd[4]          = { HLM_MAX_S32, HLM_MAX_S32, HLM_MAX_S32, HLM_MAX_S32 };
    HLM_S32 best_bits[4]          = { HLM_MAX_S32, HLM_MAX_S32, HLM_MAX_S32, HLM_MAX_S32 };
    HLM_MV best_bv[4]             = { { 0 } };
    HLM_MV cur_bv                 = { 0 };
    HLM_S32 boundary_x            = cur_cu->com_cu_info.cu_x % (1 << (segment_width_in_log2 - HLM_LOG2_WIDTH_SIZE));
#if MIX_IBC
    HLM_MV  best_mix_bv[HLM_BV_MERGE_NUM][HLM_IBC_PART_NUM][4]   = { { { { 0 } } } };
    HLM_MV  best_mix_bvd[HLM_BV_MERGE_NUM][HLM_IBC_PART_NUM][4]  = { { { { 0 } } } };
    HLM_S32 best_mix_cost[HLM_BV_MERGE_NUM][HLM_IBC_PART_NUM][4] = { { { 0 } } };
    HLM_U32 tmp_sad[3][HLM_IBC_PART_NUM][4]                      = { { { 0 } } };  // 3分量，4个子PU
    HLM_U32 sad_4x4[HLM_IBC_PART_NUM][4][HLM_IBC_SEARCH_AREA_HEIGHT][HLM_IBC_SEARCH_AREA_WIDTH] = { { { { 0 } } } };
    HLM_IBC_PART_TYPE best_part_type                             = 0;
    HLM_IBC_PART_TYPE part_type                                  = 0;
    HLM_U08 pu_idx                                               = 0;
    HLM_U08 pu_num                                               = 0;
    HLM_IBC_PU_INFO *pu_info                                     = HLM_NULL;
    HLM_S32 tmp_cost                                             = HLM_MAX_S32;
    HLM_MV cur_bvp                                               = { 0 };
    HLM_MV cur_bvd                                               = { 0 };
#endif

    if (segment_enable_flag)
    {
        bvx_bits = (boundary_x >= 8) ? HLM_IBC_HOR_SEARCH_LOG : tbl_bvx_bits[boundary_x];
    }

    start_x = HLM_MAX(0, end_x - (1 << HLM_IBC_HOR_SEARCH_LOG) + 1);
    for (yuv_idx = 0; yuv_idx < num_comp; yuv_idx++)
    {
        HLM_COM_GetBlock(ori_ptr[yuv_idx], stride[yuv_idx], blk_org_4x4[yuv_idx], 4, pu_x_in_pic, pu_y_in_pic, 4, 4);
    }

#if MIX_IBC
    bvx_bits = HLM_COM_GetBvxLen(&cur_cu->com_cu_info, segment_enable_flag, segment_width_in_log2);
#if FIRST_COLUMN_IBC
    if (cur_cu->com_cu_info.first_column_ibc_flag)
    {
        start_x = 0;
        end_x = (1 << HLM_IBC_HOR_SEARCH_LOG) - 1;
        pu_x_in_area -= HLM_IBC_BUFFER_WIDTH;
    }
#endif
#endif

    // 先搜索x，最多105个位置
#if IBC_SEARCH_YUV
    for (i = 0; i < 4; i++)  // 全搜索
    {
        cur_y = pu_y_in_area + (pu_y == 0 ? i : -i);
#else
    cur_y = pu_y_in_area;
    {
#endif
        for (cur_x = end_x; cur_x >= start_x; cur_x--)  // 由近及远搜索
        {
#if MIX_IBC
#if FIRST_COLUMN_IBC
            if (cur_cu->com_cu_info.first_column_ibc_flag && cur_y != pu_y_in_area)
            {
                continue;  // 只搜一行
            }
#endif
            for (yuv_idx = 0; yuv_idx < num_comp; yuv_idx++)
            {
                HLM_COM_GetBlock(search_area + yuv_idx * HLM_IBC_SEARCH_AREA_WIDTH * HLM_IBC_SEARCH_AREA_HEIGHT,
                    HLM_IBC_SEARCH_AREA_WIDTH, blk_pred_4x4[yuv_idx], 4, cur_x, cur_y, 4, 4);
                HLMC_COM_ComputeIbc4x4Sad(blk_org_4x4[yuv_idx], blk_pred_4x4[yuv_idx], 4, 4, tmp_sad[yuv_idx]);
                for (part_type = 0; part_type < HLM_IBC_PART_NUM; part_type++)
                {
                    pu_num = (part_type == HLM_IBC_NO_SPLIT) ? 1 : 4;
                    for (pu_idx = 0; pu_idx < pu_num; pu_idx++)
                    {
                        sad_4x4[part_type][pu_idx][cur_y][cur_x] += tmp_sad[yuv_idx][part_type][pu_idx];
                    }
                }
            }
#else
            cur_bv.mvx = cur_x - pu_x_in_area;
            cur_bv.mvy = cur_y - pu_y_in_area;
            satd = 0;
            for (yuv_idx = 0; yuv_idx < num_comp; yuv_idx++)
            {
                HLM_COM_GetBlock(search_area + yuv_idx * HLM_IBC_SEARCH_AREA_WIDTH * HLM_IBC_SEARCH_AREA_HEIGHT,
                    HLM_IBC_SEARCH_AREA_WIDTH, blk_pred_4x4[yuv_idx], 4, cur_x, cur_y, 4, 4);
                satd += HLMC_COM_ComputeSad(blk_org_4x4[yuv_idx], blk_pred_4x4[yuv_idx], 4, 4, 4, 4);
            }
            for (merge_flag = 0; merge_flag < HLM_BV_MERGE_NUM; merge_flag++)
            {
                direct = tbl_merge_type[merge_flag][zscan_idx];
                if (direct == 0 && -cur_bv.mvx - bvx_offset >= (1 << bvx_bits))  // 定长码限制bv_x
                {
                    continue;
                }
                cur_cu->bv_enc[merge_flag][zscan_idx] = cur_bv;
                bits = HLMC_ECD_EncodeBV(segment_enable_flag ? cur_cu->com_cu_info.cu_x % (1 << (segment_width_in_log2 - HLM_LOG2_WIDTH_SIZE)) : cur_cu->com_cu_info.cu_x,
#if BVY_ZERO
                                         cur_bv.mvy == 0,
#endif
                                         merge_flag, zscan_idx, cur_cu->bv_enc[merge_flag], HLM_NULL, 1);
#if SAD_SATD_TH_SCALE
                cost = satd + ((lamda * bits) >> (HLMC_LAMBDA_SHIFT));
#else
                cost = satd + ((lamda * bits) >> (HLMC_LAMBDA_SHIFT + 4));
#endif
                if (cost < best_cost[merge_flag])
                {
                    best_x[merge_flag]    = cur_x;
                    best_y[merge_flag]    = cur_y;
                    best_cost[merge_flag] = cost;
                    best_satd[merge_flag] = satd;
                    best_bits[merge_flag] = bits;
                    best_bv[merge_flag]   = cur_bv;
                }
            }
#endif
        }
    }

#if !IBC_SEARCH_YUV && !MIX_IBC
    // 快速搜索算法是：先搜水平128个点，再搜垂直3(bvy)x4(merge)=12个点
    for (merge_flag = 0; merge_flag < HLM_BV_MERGE_NUM; merge_flag++)
    {
        for (i = 1; i < 4; i++)
        {
            cur_x = best_x[merge_flag];
            cur_y = pu_y_in_area + (pu_y == 0 ? i : -i);
            cur_bv.mvx = cur_x - pu_x_in_area;
            cur_bv.mvy = cur_y - pu_y_in_area;
            satd = 0;
            for (yuv_idx = 0; yuv_idx < num_comp; yuv_idx++)
            {
                HLM_COM_GetBlock(search_area + yuv_idx * HLM_IBC_SEARCH_AREA_WIDTH * HLM_IBC_SEARCH_AREA_HEIGHT,
                    HLM_IBC_SEARCH_AREA_WIDTH, blk_pred_4x4[yuv_idx], 4, cur_x, cur_y, 4, 4);
                satd += HLMC_COM_ComputeSad(blk_org_4x4[yuv_idx], blk_pred_4x4[yuv_idx], 4, 4, 4, 4);
            }

            cur_cu->bv_enc[merge_flag][zscan_idx] = cur_bv;
            bits = HLMC_ECD_EncodeBV(segment_enable_flag ? cur_cu->com_cu_info.cu_x % (1 << (segment_width_in_log2 - HLM_LOG2_WIDTH_SIZE)) : cur_cu->com_cu_info.cu_x,
#if BVY_ZERO
                                     cur_bv.mvy == 0,
#endif
                                     merge_flag, zscan_idx, cur_cu->bv_enc[merge_flag], HLM_NULL, 1);
#if SAD_SATD_TH_SCALE
            cost = satd + ((lamda * bits) >> (HLMC_LAMBDA_SHIFT));
#else
            cost = satd + ((lamda * bits) >> (HLMC_LAMBDA_SHIFT + 4));
#endif
            if (cost < best_cost[merge_flag])
            {
                best_x[merge_flag]    = cur_x;
                best_y[merge_flag]    = cur_y;
                best_cost[merge_flag] = cost;
                best_satd[merge_flag] = satd;
                best_bits[merge_flag] = bits;
                best_bv[merge_flag]   = cur_bv;
            }
        }
    }
#endif

#if MIX_IBC
    // 确定所有划分下、所有子PU的最优bv和cost
    for (merge_flag = 0; merge_flag < HLM_BV_MERGE_NUM; merge_flag++)
    {
        direct = tbl_merge_type[merge_flag][zscan_idx];
        pu_info = &cur_cu->com_cu_info.ibc_pu_info[merge_flag][zscan_idx];
        for (part_type = 0; part_type < HLM_IBC_PART_NUM; part_type++)
        {
#if FIRST_COLUMN_IBC
            if (cur_cu->com_cu_info.first_column_ibc_flag && part_type != HLM_IBC_HOR_SYM4)
            {
                continue;
            }
#endif
            HLM_COM_GetIbcPuInfo(part_type, pu_info);
            for (pu_idx = 0; pu_idx < pu_info->sub_pu_num; pu_idx++)
            {
                tmp_cost = HLM_MAX_S32;
                if (direct == 0 && pu_idx == 0)  // 定长码
                {
                    memset(&cur_bvp, 0, sizeof(HLM_MV));
                    memset(&cur_bvd, 0, sizeof(HLM_MV));
                }
                else
                {
                    HLM_COM_GetBvp(&cur_cu->com_cu_info, merge_flag, zscan_idx, pu_idx, &cur_bvp);
                }
#if IBC_SEARCH_YUV
                for (i = 0; i < 4; i++)  // 全搜索
                {
                    cur_y = pu_y_in_area + (pu_y == 0 ? i : -i);
#else
                cur_y = pu_y_in_area;
                {
#endif
                    for (cur_x = end_x; cur_x >= start_x; cur_x--)  // 由近及远搜索
                    {
#if FIRST_COLUMN_IBC
                        if (cur_cu->com_cu_info.first_column_ibc_flag && cur_y != pu_y_in_area)
                        {
                            continue;  // 只搜一行
                        }
#endif
                        cur_bv.mvx = cur_x - pu_x_in_area;
                        cur_bv.mvy = cur_y - pu_y_in_area;
                        satd = sad_4x4[part_type][pu_idx][cur_y][cur_x];
                        bits = 0;
                        if (direct == 0 && pu_idx == 0)  // 定长码
                        {
                            if (-cur_bv.mvx - bvx_offset >= (1 << bvx_bits))  // 定长码限制bv_x
                            {
                                continue;
                            }
                            bits += bvx_bits;
                        }
                        else
                        {
                            cur_bvd.mvx = cur_bv.mvx - cur_bvp.mvx;
                            cur_bvd.mvy = cur_bv.mvy - cur_bvp.mvy;
                            bits += HLMC_ECD_PutSeBits(HLM_NULL, cur_bvd.mvx, 0, "bv_x");
                        }
                        bits += (cur_bv.mvy == 0) ? 0 : 2;  // y定长
                        cost = satd + ((lamda * bits) >> HLMC_LAMBDA_SHIFT);
                        if (cost < tmp_cost)
                        {
                            tmp_cost = cost;
                            best_mix_bv[merge_flag][part_type][pu_idx] = cur_bv;
                            best_mix_bvd[merge_flag][part_type][pu_idx] = cur_bvd;
                            best_mix_cost[merge_flag][part_type][pu_idx] = cost;
                        }
                    }
                }
                pu_info->sub_bv[pu_idx] = best_mix_bv[merge_flag][part_type][pu_idx];  // 用于更新bvp
            }
        }
    }
#endif

    // 存储最优bv
    for (merge_flag = 0; merge_flag < HLM_BV_MERGE_NUM; merge_flag++)
    {
#if !MIX_IBC
        cur_cu->bv_enc[merge_flag][zscan_idx] = best_bv[merge_flag];
#else
        // 决策最优part_type
#if FIRST_COLUMN_IBC
        if (cur_cu->com_cu_info.first_column_ibc_flag)
        {
            best_part_type = HLM_IBC_HOR_SYM4;
        }
        else
#endif
        {
            tmp_cost = HLM_MAX_S32;
            for (part_type = 0; part_type < HLM_IBC_PART_NUM; part_type++)
            {
                pu_num = (part_type == HLM_IBC_NO_SPLIT) ? 1 : 4;
                cost = 0;
                for (pu_idx = 0; pu_idx < pu_num; pu_idx++)
                {
                    cost += best_mix_cost[merge_flag][part_type][pu_idx];
                }
                bits = HLMC_ECD_EncodePartType(part_type, HLM_NULL, 1);
                cost += ((lamda * bits) >> HLMC_LAMBDA_SHIFT);
                if (cost < tmp_cost)
                {
                    tmp_cost = cost;
                    best_part_type = part_type;
                }
            }
        }

        pu_info = &cur_cu->com_cu_info.ibc_pu_info[merge_flag][zscan_idx];
        HLM_COM_GetIbcPuInfo(best_part_type, pu_info);
        for (pu_idx = 0; pu_idx < pu_info->sub_pu_num; pu_idx++)
        {
            pu_info->sub_bv[pu_idx] = best_mix_bv[merge_flag][best_part_type][pu_idx];
            pu_info->sub_bvd[pu_idx] = best_mix_bvd[merge_flag][best_part_type][pu_idx];
        }
        HLM_COM_UpdateInnerBv(&cur_cu->com_cu_info, zscan_idx, merge_flag, pu_info->part_type, pu_info->sub_bv);
#endif
    }
}

/***************************************************************************************************
* 功  能：SCC模式初筛主函数
* 参  数：*
*        regs               -I        硬件寄存器
*        nbi_info           -I        当前cu相邻块信息
*        cur_cu             -IO       当前cu信息
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_SCC_MODE(HLMC_REGS            *regs,
                      HLM_NEIGHBOR_INFO    *nbi_info,
                      HLMC_CU_INFO         *cur_cu)
{
    HLM_S32 zscan_idx            = 0;  // Z字扫描顺序
    HLM_S32 pu_x                 = 0;  // 4x4子宏块相对于宏块左上角的x/y坐标，以子宏块为单位
    HLM_S32 pu_y                 = 0;
    HLM_S32 pu_x_in_pic          = 0;
    HLM_S32 pu_y_in_pic          = 0;
    HLM_S32 pu_x_in_area         = 0;
    HLM_S32 pu_y_in_area         = 0;
    HLM_S32 cu_pixel_x           = cur_cu->com_cu_info.cu_x << HLM_LOG2_WIDTH_SIZE;  // 当前CU的坐标，以像素为单位
    HLM_S32 cu_pixel_y           = cur_cu->com_cu_info.cu_y << HLM_LOG2_HEIGHT_SIZE;
    HLM_U16 *ori_ptr[3]          = { 0 };  // 原始图像指针，指向图像的左上角点
    HLM_U16 *rec_ptr[3]          = { 0 };  // 重建图像指针，指向图像的左上角点
    HLM_U32 stride[3]            = { 0 };
    HLM_U16 blk_org[16]          = { 0 };
    HLM_U16 blk_pred[16]         = { 0 };
    HLM_U32 satd                 = 0;
    HLM_U08 yuv_idx              = 0;
    HLM_U08 i                    = 0;
    HLM_U16 *src                 = 0;
    HLM_U16 *dst                 = 0;
    HLM_S32 valid_len            = cu_pixel_x - HLM_MAX(0, cu_pixel_x - HLM_IBC_BUFFER_WIDTH);
    HLM_S32 org_len              = (cur_cu->com_cu_info.cu_x == 0) ? 0 : 16;  // 左相邻用原始，更左侧用重建
    HLM_S32 rec_len              = valid_len - org_len;
    HLM_S32 pad_len              = HLM_IBC_BUFFER_WIDTH - valid_len;
    HLM_U32 lamda                = regs->enc_satd_lamda[cur_cu->com_cu_info.qp[0]];
    HLM_S32 bits                 = 0;
    HLM_S32 cost                 = 0;
    HLM_S32 best_bits            = HLM_MAX_S32;
    HLM_U08 merge_flag           = 0;
    HLM_U08 best_merge_flag      = 0;
    HLM_S32 best_cost            = HLM_MAX_S32;
    HLM_S32 best_satd            = HLM_MAX_S32;
    HLM_S32 satd_4x4             = 0;
    HLM_S32 satd_comp[3]         = { 0 };
    HLM_S32 best_satd_comp[3]    = { 0 };
    HLM_U08 num_comp             = (regs->image_format == HLM_IMG_YUV_400) ? 1 : 3;
    HLM_S08 hor_shift[3]         = { 0 };
    HLM_S08 ver_shift[3]         = { 0 };
    HLM_U16 cb_org[HLM_CU_SIZE]  = { 0 };
    HLM_U16 cb_pred[HLM_CU_SIZE] = { 0 };
    HLM_U32 pu_w                 = 0;
    HLM_U32 pu_h                 = 0;
    HLM_U32 pu_pos               = 0;
    HLM_S32 boundary_x           = 0;
    HLM_S32 boundary_y           = 0;
#if MIX_IBC || BVY_ZERO
    HLM_IBC_PU_INFO pb_info      = { 0 };  // 区分亮色度
    HLM_IBC_PU_INFO *pu_info     = HLM_NULL;
    HLM_U08 mix_ibc_flag         = 0;
    HLM_U08 best_mix_ibc_flag    = 0;
    HLM_MV *cur_bv               = HLM_NULL;
    HLM_U08 bvx_bits             = 0;
    HLM_U08 direct               = 0;
    HLM_U08 bvy_zero_flag        = 0;
    HLM_U08 sub_bvy_zero_flag[2] = { 0 };
    HLM_U08 best_bvy_zero_flag   = 0;
    HLM_U16 *up_rec[3]           = { 0 };  // 上一行的重建像素
#endif

    stride[0]  = regs->enc_input_luma_stride;
    stride[1]  = regs->enc_input_chroma_stride;
    stride[2]  = regs->enc_input_chroma_stride;
    ori_ptr[0] = regs->enc_input_y_base;
    ori_ptr[1] = regs->enc_input_cb_base;
    ori_ptr[2] = regs->enc_input_cr_base;
    rec_ptr[0] = regs->enc_recon_y_base;
    rec_ptr[1] = regs->enc_recon_cb_base;
    rec_ptr[2] = regs->enc_recon_cr_base;
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
        valid_len = cu_pixel_x % (1 << regs->segment_width_in_log2);
        org_len   = (valid_len == 0) ? 0 : HLM_WIDTH_SIZE;  // 左相邻用原始，更左侧用重建
        rec_len   = valid_len - org_len;
        pad_len   = HLM_IBC_BUFFER_WIDTH - valid_len;
    }
    boundary_x = regs->segment_enable_flag ? cu_pixel_x % (1 << regs->segment_width_in_log2) : cu_pixel_x;
    boundary_y = regs->segment_enable_flag ? cu_pixel_y % (1 << regs->segment_height_in_log2) : cu_pixel_y;

    // 填充搜索区域
    HLM_COM_GetFormatShift(regs->image_format, hor_shift, ver_shift);
    for (yuv_idx = 0; yuv_idx < num_comp; yuv_idx++)
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
        HLM_COM_SearchAreaCopy(cur_cu->com_cu_info.search_area[yuv_idx], ori_ptr[yuv_idx], stride[yuv_idx],
                               16 >> hor_shift[yuv_idx], HLM_IBC_SEARCH_AREA_HEIGHT >> ver_shift[yuv_idx],
                               cu_pixel_x >> hor_shift[yuv_idx], cu_pixel_y >> ver_shift[yuv_idx],
                               HLM_IBC_BUFFER_WIDTH >> hor_shift[yuv_idx]);  // 当前块内部填充原始
        if (org_len > 0)
        {
            HLM_COM_SearchAreaCopy(cur_cu->com_cu_info.search_area[yuv_idx], ori_ptr[yuv_idx], stride[yuv_idx],
                                   org_len >> hor_shift[yuv_idx], HLM_IBC_SEARCH_AREA_HEIGHT >> ver_shift[yuv_idx],
                                   (cu_pixel_x - org_len) >> hor_shift[yuv_idx], cu_pixel_y >> ver_shift[yuv_idx],
                                   (HLM_IBC_BUFFER_WIDTH - org_len) >> hor_shift[yuv_idx]);
        }
        if (rec_len)
        {
            HLM_COM_SearchAreaCopy(cur_cu->com_cu_info.search_area[yuv_idx], rec_ptr[yuv_idx], stride[yuv_idx],
                                   rec_len >> hor_shift[yuv_idx], HLM_IBC_SEARCH_AREA_HEIGHT >> ver_shift[yuv_idx],
                                   (cu_pixel_x - valid_len) >> hor_shift[yuv_idx], cu_pixel_y >> ver_shift[yuv_idx],
                                   (HLM_IBC_BUFFER_WIDTH - valid_len) >> hor_shift[yuv_idx]);
        }
        if (pad_len)
        {
            HLM_COM_SearchAreaPadding(cur_cu->com_cu_info.search_area[yuv_idx], rec_ptr[yuv_idx], regs->bitdepth, stride[yuv_idx],
                                      pad_len >> hor_shift[yuv_idx], HLM_IBC_SEARCH_AREA_HEIGHT >> ver_shift[yuv_idx],
                                      ver_shift[yuv_idx], boundary_x, boundary_y,
                                      cur_cu->com_cu_info.cu_x, cur_cu->com_cu_info.cu_y);
        }
    }

    // BV搜索
    for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
    {
        pu_x         = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
        pu_y         = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
        pu_x_in_pic  = cu_pixel_x + pu_x * 4;
        pu_y_in_pic  = cu_pixel_y + pu_y * 4;
        pu_x_in_area = HLM_IBC_BUFFER_WIDTH + pu_x * 4;
        pu_y_in_area = pu_y * 4;
        HLMC_SCC_ibc_search(ori_ptr, stride, &cur_cu->com_cu_info.search_area[0][0],
#if IBC_SEARCH_YUV
            (regs->image_format == HLM_IMG_YUV_444 || regs->image_format == HLM_IMG_RGB) ? 3 : 1,
#else
            1,
#endif
            pu_x_in_pic, pu_y_in_pic, pu_x_in_area, pu_y_in_area,
            cur_cu, lamda, zscan_idx, regs->segment_enable_flag, regs->segment_width_in_log2);
    }

    // 决策merge_flag
    for (merge_flag = 0; merge_flag < HLM_BV_MERGE_NUM; merge_flag++)
    {
        bits = 0;
#if MIX_IBC
        HLMC_COM_DeriveMixIbcInfo(&cur_cu->com_cu_info, merge_flag, &mix_ibc_flag, &bvy_zero_flag);
        bvx_bits = HLM_COM_GetBvxLen(&cur_cu->com_cu_info, regs->segment_enable_flag, regs->segment_width_in_log2);
        for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
        {
            direct = tbl_merge_type[merge_flag][zscan_idx];
            pu_info = &cur_cu->com_cu_info.ibc_pu_info[merge_flag][zscan_idx];
            if ((zscan_idx < 4 ? (mix_ibc_flag >> 1) : (mix_ibc_flag & 1))
#if FIRST_COLUMN_IBC
                && !cur_cu->com_cu_info.first_column_ibc_flag
#endif
                )
            {
                bits += HLMC_ECD_EncodePartType(pu_info->part_type, HLM_NULL, 1);
            }
            for (i = 0; i < pu_info->sub_pu_num; i++)
            {
                if (direct == 0 && i == 0)  // 定长码
                {
                    bits += bvx_bits;
                }
                else
                {
                    bits += HLMC_ECD_PutSeBits(HLM_NULL, pu_info->sub_bvd[i].mvx, 0, "bv_x");
                }
                bits += (zscan_idx < 4 ? (bvy_zero_flag >> 1) : (bvy_zero_flag & 1)) ? 0 : 2;  // y定长
            }
        }
#else
#if BVY_ZERO
        sub_bvy_zero_flag[0] = 1;
        sub_bvy_zero_flag[1] = 1;
        for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
        {
            if (cur_cu->bv_enc[merge_flag][zscan_idx].mvy != 0)
            {
#if BVY_ZERO == 3
                if (((zscan_idx >> 1) & 1) == 0)
#else
                if (zscan_idx < 4)
#endif
                {
                    sub_bvy_zero_flag[0] = 0;
                }
                else
                {
                    sub_bvy_zero_flag[1] = 0;
                }
            }
        }
#if BVY_ZERO == 1
        bvy_zero_flag = sub_bvy_zero_flag[0] & sub_bvy_zero_flag[1];
#else
        bvy_zero_flag = (sub_bvy_zero_flag[0] << 1) + sub_bvy_zero_flag[1];
#endif
#endif
        for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
        {
            bits += HLMC_ECD_EncodeBV(regs->segment_enable_flag ? valid_len >> HLM_LOG2_WIDTH_SIZE : cur_cu->com_cu_info.cu_x,
#if BVY_ZERO == 1
                                      bvy_zero_flag,
#elif BVY_ZERO == 2
                                      (zscan_idx < 4) ? (bvy_zero_flag >> 1) : (bvy_zero_flag & 1),
#elif BVY_ZERO == 3
                                      ((zscan_idx >> 1) & 1) == 0 ? (bvy_zero_flag >> 1) : (bvy_zero_flag & 1),
#endif
                                      merge_flag, zscan_idx, cur_cu->bv_enc[merge_flag], HLM_NULL, 1);
        }
#endif

        satd = 0;
        for (yuv_idx = 0; yuv_idx < num_comp; yuv_idx++)
        {
            // 先以PU为单位获取预测块
            for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
            {
                pu_x         = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
                pu_y         = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
                pu_w         = 4 >> hor_shift[yuv_idx];
                pu_h         = 4 >> ver_shift[yuv_idx];
                pu_pos       = (pu_y * pu_h) * HLM_WIDTH_SIZE + (pu_x * pu_w);
                pu_x_in_pic  = (cu_pixel_x >> hor_shift[yuv_idx]) + pu_x * pu_w;
                pu_y_in_pic  = (cu_pixel_y >> ver_shift[yuv_idx]) + pu_y * pu_h;
                pu_x_in_area = (HLM_IBC_BUFFER_WIDTH >> hor_shift[yuv_idx]) + pu_x * pu_w;
                pu_y_in_area = pu_y * pu_h;
                HLM_COM_GetBlock(ori_ptr[yuv_idx], stride[yuv_idx], cb_org + pu_pos, HLM_WIDTH_SIZE,
                    pu_x_in_pic, pu_y_in_pic, pu_w, pu_h);
#if MIX_IBC
                pu_info = &cur_cu->com_cu_info.ibc_pu_info[merge_flag][zscan_idx];
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
                HLM_COM_GetBlock(cur_cu->com_cu_info.search_area[yuv_idx], HLM_IBC_SEARCH_AREA_WIDTH, cb_pred + pu_pos, HLM_WIDTH_SIZE,
#if BVY_ZERO  // bugfix
                    pu_x_in_area + (cur_cu->bv_enc[merge_flag][zscan_idx].mvx >> hor_shift[yuv_idx]),
                    pu_y_in_area + (cur_cu->bv_enc[merge_flag][zscan_idx].mvy >> ver_shift[yuv_idx]),
#else
                    pu_x_in_area + (cur_cu->bv_enc[cur_cu->com_cu_info.merge_flag][zscan_idx].mvx >> hor_shift[yuv_idx]),
                    pu_y_in_area + (cur_cu->bv_enc[cur_cu->com_cu_info.merge_flag][zscan_idx].mvy >> ver_shift[yuv_idx]),
#endif
                    pu_w, pu_h);
#endif
            }

            // 再以TU为单位计算satd
            for (pu_x = 0; pu_x < (16 >> hor_shift[yuv_idx] >> 2); pu_x++)
            {
                for (pu_y = 0; pu_y < (8 >> ver_shift[yuv_idx] >> 2); pu_y++)
                {
                    // merge_flag决策用sad
                    cost = HLMC_COM_ComputeSad(cb_org + (pu_y * 4) * HLM_WIDTH_SIZE + (pu_x * 4),
                        cb_pred + (pu_y * 4) * HLM_WIDTH_SIZE + (pu_x * 4), 4, 4, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
                    satd += cost;

                    // 码控复杂度用satd
#if PK_WITH_SATD
                    satd_4x4 = HLMC_COM_ComputeSatd4x4(cb_org + (pu_y * 4) * HLM_WIDTH_SIZE + (pu_x * 4),
                        cb_pred + (pu_y * 4) * HLM_WIDTH_SIZE + (pu_x * 4), HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
#else
                    satd_4x4 = cost;
#endif
                    satd_comp[yuv_idx] += satd_4x4;
                }
            }
        }
#if SAD_SATD_TH_SCALE
        cost = satd + ((lamda * bits) >> (HLMC_LAMBDA_SHIFT));
#else
        cost = satd + ((lamda * bits) >> (HLMC_LAMBDA_SHIFT + 4));
#endif
        if (cost < best_cost)
        {
            best_cost         = cost;
            best_satd         = satd;
            best_satd_comp[0] = satd_comp[0];
            best_satd_comp[1] = satd_comp[1];
            best_satd_comp[2] = satd_comp[2];
            best_bits         = bits;
            best_merge_flag   = merge_flag;
#if MIX_IBC || BVY_ZERO
            best_mix_ibc_flag = mix_ibc_flag;
            best_bvy_zero_flag = bvy_zero_flag;
#endif
        }
    }

    bits = 1;  // cu_type
    bits += (HLM_BV_MERGE_NUM > 2 ? 2 : 1);  // merge_flag
    bits += 1;  // ts_flag
    bits += best_bits;  // bv
    satd = best_satd_comp[0] + best_satd_comp[1] + best_satd_comp[2];
    cur_cu->com_cu_info.merge_flag = best_merge_flag;
#if MIX_IBC || BVY_ZERO
    cur_cu->com_cu_info.mix_ibc_flag = best_mix_ibc_flag;
    cur_cu->com_cu_info.bvy_zero_flag = best_bvy_zero_flag;
#endif
    if (satd < cur_cu->intra_satd_cost)
    {
#if  IBC_SCALE
        cur_cu->best_cu_type = HLM_IBC_4x4;
#endif
        cur_cu->intra_satd_cost = satd;
        cur_cu->satd_comp[0] = best_satd_comp[0];
        cur_cu->satd_comp[1] = best_satd_comp[1];
        cur_cu->satd_comp[2] = best_satd_comp[2];
    }

    return satd;
}

/***************************************************************************************************
* 功  能：帧内预测主函数
* 参  数：*
*        raster_idx               -I    当前PU的光栅扫描索引
*        channel_size             -I    通道大小
*        regs                     -I    硬件寄存器
*        nbi_info                 -I    当前cu相邻块信息
*        cur_cu                   -O    当前cu信息
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTRA_PRED(HLM_U32                raster_idx,
                         HLM_U32                channel_size,
                         HLMC_REGS             *regs,
                         HLM_NEIGHBOR_INFO     *nbi_info,
                         HLMC_CU_INFO          *cur_cu)
{
    HLM_U08 intra_pred_mode                              = 0;
    HLM_U08 yuv_idx                                      = 0;
    HLM_U16 k                                            = 0;
    HLM_U16 default_value                                = 1 << (regs->bitdepth - 1);
    HLM_U32 pu_pos                                       = 0;
    HLM_U08 offset_w                                     = (channel_size == 8) ? 3 : 2;
    HLM_U08 offset_h                                     = (channel_size == 8) ? 3 : 2;
    HLM_U08 pu_size_w                                    = 1 << offset_w;
    HLM_U08 pu_size_h                                    = 1 << offset_h;
    HLM_U32 skip_chroma[2]                               = { 0 };
    HLM_U32 chroma_size                                  = channel_size;
    HLM_U32 chroma_size_offset                           = 0;
    HLM_S32 yuv_comp                                     = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_U32 zscan_idx                                    = HLM_INTRA_RASTER_TO_ZSCAN[raster_idx];
    HLM_U08 pu_pixel_x                                   = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx] << offset_w;
    HLM_U08 pu_pixel_y                                   = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx] << offset_h;
    HLM_U16 ref_pixel_16x8[HLM_INTRA_REF_PIXEL_NUM_16x8] = { 0 };
    HLM_U16 ref_pixel_4x4[HLM_INTRA_REF_PIXEL_NUM_4x4]   = { 0 };

    if (channel_size == 16)
    {
        intra_pred_mode = cur_cu->pu_info_enc[0].inter_pu_info.intra_pred_mode;
        if (intra_pred_mode == HLM_INTRA_16x8_DC)
        {
            if (cur_cu->com_cu_info.left_unavail  && cur_cu->com_cu_info.up_unavail)
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
            else
            {
                intra_pred_mode = HLM_INTRA_16x8_DC;
            }
        }

        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            for (k = 0; k < HLM_INTRA_REF_PIXEL_NUM_16x8; k++)
            {
                ref_pixel_16x8[k] = default_value;
            }
            memset(cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx], 0, 16 * 8 * sizeof(HLM_U16));
            HLM_COM_Intra16x8RefPel(nbi_info, &cur_cu->com_cu_info, intra_pred_mode, yuv_idx, ref_pixel_16x8);
            HLM_INTRA_PRED_16x8[intra_pred_mode](ref_pixel_16x8, cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx],
                regs->bitdepth, HLM_WIDTH_SIZE,
                1 << cur_cu->com_cu_info.cu_width[yuv_idx],
                1 << cur_cu->com_cu_info.cu_height[yuv_idx]);
        }
    }
    else if (channel_size == 4)
    {
        intra_pred_mode = cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].intra_pred_mode;
        if (intra_pred_mode == HLM_INTRA_4x4_DC)
        {
            if ((cur_cu->com_cu_info.left_unavail) && (cur_cu->com_cu_info.up_unavail) && (pu_pixel_x == 0) && (pu_pixel_y == 0))
            {
                intra_pred_mode = HLM_INTRA_4x4_DC_128;
            }
            else if ((cur_cu->com_cu_info.left_unavail) && (pu_pixel_x == 0))
            {
                intra_pred_mode = HLM_INTRA_4x4_DC_TOP;
            }
            else if ((cur_cu->com_cu_info.up_unavail) && (pu_pixel_y == 0))
            {
                intra_pred_mode = HLM_INTRA_4x4_DC_LEFT;
            }
            else
            {
                intra_pred_mode = HLM_INTRA_4x4_DC;
            }
        }

        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            for (k = 0; k < HLM_INTRA_REF_PIXEL_NUM_4x4; k++)
            {
                ref_pixel_4x4[k] = default_value;
            }
            pu_pixel_x = raster_idx % 4;
            pu_pixel_y = raster_idx >> 2;
            pu_pos = (pu_pixel_y << offset_h) * HLM_WIDTH_SIZE + (pu_pixel_x << offset_w);
            HLM_COM_Intra4x4RefPel(regs->enc_recon_y_base, regs->enc_recon_cb_base, regs->enc_recon_cr_base,
                regs->enc_input_luma_stride, regs->enc_input_chroma_stride, regs->enc_input_chroma_stride,
                regs->cur_patch_param->patch_coded_width[0] >> 4,  nbi_info, &cur_cu->com_cu_info,
                intra_pred_mode, yuv_idx, zscan_idx, offset_w, ref_pixel_4x4);
            HLM_INTRA_PRED_4x4[intra_pred_mode](ref_pixel_4x4, cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos,
                regs->bitdepth, HLM_WIDTH_SIZE,
                yuv_idx == 0 ? pu_size_w : pu_size_w >> cur_cu->com_cu_info.chroma_offset_x,
                yuv_idx == 0 ? pu_size_h : pu_size_h >> cur_cu->com_cu_info.chroma_offset_y);
        }
    }
    else if (channel_size == 8)
    {
        HLM_COM_Intra8x8ChromaPb(regs->image_format, channel_size, skip_chroma, &chroma_size, &chroma_size_offset);
        intra_pred_mode = cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].intra_pred_mode;
        if (intra_pred_mode == HLM_INTRA_4x4_DC)
        {
            if ((cur_cu->com_cu_info.left_unavail) && (cur_cu->com_cu_info.up_unavail) && (pu_pixel_x == 0) && (pu_pixel_y == 0))
            {
                intra_pred_mode = HLM_INTRA_4x4_DC_128;
            }
            else if ((cur_cu->com_cu_info.left_unavail) && (pu_pixel_x == 0))
            {
                intra_pred_mode = HLM_INTRA_4x4_DC_TOP;
            }
            else if ((cur_cu->com_cu_info.up_unavail) && (pu_pixel_y == 0))
            {
                intra_pred_mode = HLM_INTRA_4x4_DC_LEFT;
            }
            else
            {
                intra_pred_mode = HLM_INTRA_4x4_DC;
            }
        }

        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            if (yuv_idx && skip_chroma[raster_idx])
            {
                continue;
            }
            if (yuv_idx == 1)
            {
                offset_w = offset_w - chroma_size_offset;
                offset_h = offset_h - chroma_size_offset;
            }
            for (k = 0; k < HLM_INTRA_REF_PIXEL_NUM_4x4; k++)
            {
                ref_pixel_4x4[k] = default_value;
            }
            pu_pixel_x = raster_idx % 4;
            pu_pixel_y = raster_idx >> 2;
            pu_pos = (pu_pixel_y << offset_h) * HLM_WIDTH_SIZE + (pu_pixel_x << offset_w);
            HLM_COM_Intra4x4RefPel(regs->enc_recon_y_base, regs->enc_recon_cb_base, regs->enc_recon_cr_base,
                regs->enc_input_luma_stride, regs->enc_input_chroma_stride, regs->enc_input_chroma_stride,
                regs->cur_patch_param->patch_coded_width[0] >> 4, nbi_info, &cur_cu->com_cu_info,
                intra_pred_mode, yuv_idx, zscan_idx, offset_w, ref_pixel_4x4);
            HLM_INTRA_PRED_4x4[intra_pred_mode](ref_pixel_4x4, cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos,
                regs->bitdepth, HLM_WIDTH_SIZE,
                yuv_idx == 0 ? pu_size_w : pu_size_w >> chroma_size_offset,
                yuv_idx == 0 ? pu_size_h : pu_size_h >> chroma_size_offset);
        }
    }
}

/***************************************************************************************************
* 功  能：帧内RDO模块
* 参  数：*
*        raster_idx               -I    当前PU的光栅扫描索引
*        channel_size             -I    当前通道的大小
*        regs                     -I    当前数据硬件寄存器
*        nbi_info                 -I    相邻块信息
*        best_cu                  -O    最优cu
*        cur_cu                   -I    当前cu
* 返回值：无
***************************************************************************************************/
HLM_VOID HLMC_INTRA_RDO(HLM_U32                raster_idx,
                        HLM_U32                channel_size,
                        HLMC_REGS             *regs,
                        HLM_NEIGHBOR_INFO     *nbi_info,
                        HLMC_CU_INFO          *best_cu,
                        HLMC_CU_INFO          *cur_cu)
{
    HLM_U32 bits               = 0;
    HLM_U32 distortion         = 0;
    HLM_U32 sse                = 0;
    HLM_U32 lambda             = regs->enc_sse_lamda[cur_cu->com_cu_info.qp[0]];
    HLM_U08 mpm                = 0;
    HLM_U08 yuv_idx            = 0;
    HLM_U08 idx_4              = 0;
    HLM_U08 i                  = 0;
    HLM_U32 stride_yuv[3]      = { 0 };
    HLM_U16 *ori_pixel[3]      = { 0 };
    HLM_U16 *rec_pixel[3]      = { 0 };
    HLM_U08 zscan_idx          = HLM_INTRA_RASTER_TO_ZSCAN[raster_idx];
    HLM_U08 pu_x               = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
    HLM_U08 pu_y               = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
    HLM_U08 part_4x4_num       = 0;
    HLM_U08 x                  = 0;
    HLM_U08 y                  = 0;
    HLM_S32 yuv_comp           = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_U08 block_shape        = channel_size;
    HLM_U08 offset_w           = channel_size == 8 ? 3 : 2;
    HLM_U08 offset_h           = channel_size == 8 ? 3 : 2;
    HLM_U08 block_end          = channel_size == 8 ? 1 : 7;
    HLM_U32 pu_w               = 0;
    HLM_U32 pu_h               = 0;
    HLM_U32 pu_pos             = 0;
    HLM_COEFF *coef_src        = HLM_NULL;
    HLM_U32 pixel_x[3]         = { 0 };
    HLM_U32 pixel_y[3]         = { 0 };
    HLM_U32 cu_w[3]            = { 1 << cur_cu->com_cu_info.cu_width[0],
                                   1 << cur_cu->com_cu_info.cu_width[1],
                                   1 << cur_cu->com_cu_info.cu_width[2] };
    HLM_U32 cu_h[3]            = { 1 << cur_cu->com_cu_info.cu_height[0],
                                   1 << cur_cu->com_cu_info.cu_height[1],
                                   1 << cur_cu->com_cu_info.cu_height[2] };
    HLM_U32 skip_chroma[2]     = { 0 };
    HLM_U32 chroma_size        = channel_size;
    HLM_U32 chroma_size_offset = 0;
    HLM_U08 pu8[8]             = { 0,1,4,5,2,3,6,7 };
    HLM_U08 pu_num             = (block_shape == 8) ? 4 : 1;
    HLM_U08 real_idx           = 0;

    HLM_COM_Intra8x8ChromaPb(regs->image_format, channel_size, skip_chroma, &chroma_size, &chroma_size_offset);
    pu_pos        = (pu_y * block_shape) * HLM_WIDTH_SIZE + (pu_x * block_shape);
    pixel_x[0]    = (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[0]) + (pu_x << offset_w);
    pixel_y[0]    = (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[0]) + (pu_y << offset_h);
    pixel_x[1]    = (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[1]) + (pu_x << (offset_w - chroma_size_offset));
    pixel_y[1]    = (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[1]) + (pu_y << (offset_h - chroma_size_offset));
    pixel_x[2]    = (cur_cu->com_cu_info.cu_x << cur_cu->com_cu_info.cu_width[2]) + (pu_x << (offset_w - chroma_size_offset));
    pixel_y[2]    = (cur_cu->com_cu_info.cu_y << cur_cu->com_cu_info.cu_height[2]) + (pu_y << (offset_h - chroma_size_offset));
    stride_yuv[0] = regs->enc_input_luma_stride;
    stride_yuv[1] = regs->enc_input_chroma_stride;
    stride_yuv[2] = regs->enc_input_chroma_stride;
    ori_pixel[0]  = regs->enc_input_y_base + stride_yuv[0] * pixel_y[0] + pixel_x[0];
    ori_pixel[1]  = regs->enc_input_cb_base + stride_yuv[1] * pixel_y[1] + pixel_x[1];
    ori_pixel[2]  = regs->enc_input_cr_base + stride_yuv[2] * pixel_y[2] + pixel_x[2];
    rec_pixel[0]  = regs->enc_recon_y_base + stride_yuv[0] * pixel_y[0] + pixel_x[0];
    rec_pixel[1]  = regs->enc_recon_cb_base + stride_yuv[1] * pixel_y[1] + pixel_x[1];
    rec_pixel[2]  = regs->enc_recon_cr_base + stride_yuv[2] * pixel_y[2] + pixel_x[2];

    if (channel_size == 16)
    {
        cur_cu->com_cu_info.cu_type = HLM_I_16x8;
        cur_cu->intra_rd_cost = 0;
        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            HLMC_COM_ComputeRes(ori_pixel[yuv_idx],
                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx], cur_cu->com_cu_info.cu_pred_info.res[yuv_idx],
                cu_w[yuv_idx], cu_h[yuv_idx], stride_yuv[yuv_idx], HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
        }
        HLMC_TQ_Process(raster_idx, channel_size, regs->bitdepth, yuv_comp, cur_cu);

        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            part_4x4_num = (1 << (cur_cu->com_cu_info.cu_width[yuv_idx] + cur_cu->com_cu_info.cu_height[yuv_idx] - 4));
            for (idx_4 = 0; idx_4 < part_4x4_num; idx_4++)
            {
                HLM_COM_4x4_Block_To_Coeff_Pos(part_4x4_num, idx_4, &x, &y, &cur_cu->com_cu_info, yuv_idx);
            }
            HLM_COM_AddRes(cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx],
                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx], cur_cu->com_cu_info.cu_pred_info.res[yuv_idx],
                regs->bitdepth, cu_w[yuv_idx], cu_h[yuv_idx], HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
            sse = HLMC_COM_ComputeSse(ori_pixel[yuv_idx], cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx],
                cu_w[yuv_idx], cu_h[yuv_idx], stride_yuv[yuv_idx], HLM_WIDTH_SIZE);
            distortion += sse;
            for (i = 0; i < (cu_h[yuv_idx]); i++)
            {
                memcpy(rec_pixel[yuv_idx] + stride_yuv[yuv_idx] * i,
                    cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + (i*HLM_WIDTH_SIZE), (cu_w[yuv_idx]) * sizeof(HLM_U16));
            }
        }

        HLMC_COM_GetCbf(cur_cu->com_cu_info.cu_type, cur_cu->com_cu_info.cbf, cur_cu->com_cu_info.coeffs_num);
        bits += HLMC_ECD_EncodeCuType(HLM_I_16x8, regs->enc_frame_coding_type, 0, HLM_NULL,
            regs->enc_frame_coding_type == HLM_FRAME_TYPE_I ? regs->i_frame_enable_ibc : regs->p_frame_enable_ibc);
        HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, 0, &mpm, PROC_RDO);
        if (cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].intra_pred_mode == mpm)
        {
            bits += 1;
        }
        else if (cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].intra_pred_mode > mpm)
        {
            bits += 1 + 1 * (cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].intra_pred_mode - 1 != 0);
        }
        else
        {
            bits += 1 + 1 * (cur_cu->com_cu_info.cu_pred_info.pu_info[raster_idx].intra_pred_mode != 0);
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
        }
        bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 0);
        bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 1);
        bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 2);
        cur_cu->intra_rd_cost += distortion + ((lambda * bits) >> HLMC_LAMBDA_SHIFT);
        if (cur_cu->intra_rd_cost < best_cu->intra_rd_cost)
        {
            HLMC_COM_CopyCuInfo(cur_cu, best_cu, regs->image_format == HLM_IMG_YUV_400 ? 1 : 3);
        }
    }
    else  // channel_size == 8 || channel_size == 4
    {
        if (raster_idx == 0)
        {
            cur_cu->com_cu_info.cu_type = HLM_I_4x4;
            cur_cu->intra_rd_cost = 0;
        }
        pu_x = raster_idx % 4;
        pu_y = raster_idx >> 2;
        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            block_shape = yuv_idx == 0 ? channel_size : chroma_size;
            pu_pos = (pu_y * block_shape) * HLM_WIDTH_SIZE + (pu_x * block_shape);
            if (yuv_idx && skip_chroma[raster_idx]&& channel_size == 8)
            {
                continue;
            }
            HLMC_COM_ComputeRes(ori_pixel[yuv_idx],
                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos, cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
                block_shape, block_shape, stride_yuv[yuv_idx], HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
        }
        HLMC_TQ_Process(raster_idx, channel_size, regs->bitdepth,
            (skip_chroma[raster_idx] && channel_size == 8) ? 1 : yuv_comp, cur_cu);

        for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
        {
            block_shape = yuv_idx == 0 ? channel_size : chroma_size;
            pu_pos = (pu_y * block_shape) * HLM_WIDTH_SIZE + (pu_x * block_shape);
            if (yuv_idx && skip_chroma[raster_idx] && channel_size == 8)
            {
                continue;
            }
            real_idx = raster_idx;
            pu_num = (block_shape == 8) ? 4 : 1;
            part_4x4_num = (1 << (cur_cu->com_cu_info.cu_width[0] + cur_cu->com_cu_info.cu_height[0] - 4));  // 色度对齐亮度
            for (idx_4 = 0; idx_4 < pu_num; idx_4++)
            {
                if (block_shape == 8)
                {
                    real_idx = pu8[raster_idx * 4 + idx_4];
                }
                HLM_COM_4x4_Block_To_Coeff_Pos(part_4x4_num, real_idx, &x, &y, &cur_cu->com_cu_info, yuv_idx);
            }

            HLM_COM_AddRes(cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos,
                cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos, cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
                regs->bitdepth, block_shape, block_shape, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
            sse = HLMC_COM_ComputeSse(ori_pixel[yuv_idx], cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos,
                block_shape, block_shape, stride_yuv[yuv_idx], HLM_WIDTH_SIZE);
            distortion += sse;
            for (i = 0; i < block_shape; i++)
            {
                memcpy(rec_pixel[yuv_idx] + stride_yuv[yuv_idx] * i,
                    cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos + i * HLM_WIDTH_SIZE, block_shape * sizeof(HLM_U16));
            }
        }

        cur_cu->intra_rd_cost += distortion;  // 累加所有PU的失真
        if (raster_idx == block_end)
        {
            bits = 0;
            HLMC_COM_GetCbf(cur_cu->com_cu_info.cu_type, cur_cu->com_cu_info.cbf, cur_cu->com_cu_info.coeffs_num);
            bits += HLMC_ECD_EncodeCuType(HLM_I_4x4, regs->enc_frame_coding_type, 0, HLM_NULL,
                regs->enc_frame_coding_type == HLM_FRAME_TYPE_I ? regs->i_frame_enable_ibc : regs->p_frame_enable_ibc);
            for (zscan_idx = 0; zscan_idx < block_end + 1; zscan_idx++)
            {
                pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
                pu_y = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
                HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, zscan_idx, &mpm, PROC_RDO);
                bits += (cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode == mpm) ? 1 : 4;
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
                bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 1);
                bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 2);
            }
            cur_cu->intra_rd_cost += ((lambda * bits) >> HLMC_LAMBDA_SHIFT);
            if (cur_cu->intra_rd_cost < best_cu->intra_rd_cost)
            {
                HLMC_COM_CopyCuInfo(cur_cu, best_cu, regs->image_format == HLM_IMG_YUV_400 ? 1 : 3);
            }
        }
    }
}
#if LINE_BY_LINE
/***************************************************************************************************
* 功  能：逐行预测 RDO模块
* 参  数：*
*        regs                 -I    当前数据硬件寄存器
*        nbi_info             -I    相邻块信息
*        best_cu              -IO   最优ctu
*        cur_cu               -IO   当前ctu
* 返回值：无
***************************************************************************************************/
HLM_VOID HLMC_LINE_BY_LINE_RDO(HLMC_REGS              *regs,
                               HLM_NEIGHBOR_INFO     *nbi_info,
                               HLMC_CU_INFO           *best_cu,
                               HLMC_CU_INFO           *cur_cu)
{

    HLM_U08 line = 0;
#if  LINE_BY_LINE_4x1
    //HLM_U08 pred_mode = cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[0];
    HLM_U08 *pred_mode_4x1 = &(cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[0]);
#else
    HLM_U08 pred_mode = cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[0];
    HLM_U08 line_num = (pred_mode < 4) ? 8 : 16;
#endif
    
    HLM_U16 ref_pixel_16x1[19] = { 0 };
    HLM_U08 num_comp = (regs->image_format == HLM_IMG_YUV_400) ? 1 : 3;
    HLM_U08 yuv_idx = 0;
    HLM_U32 distortion = 0;
    cur_cu->intra_rd_cost = 0;
    cur_cu->com_cu_info.cu_type = HLM_I_LINE;
    cur_cu->com_cu_info.ts_flag = 1;
#if  LINE_BY_LINE_4x1
    HLM_U08 pred_lenght = 4;
#else
    if (pred_mode < 4)
#endif
    {
        for (line = 0; line < 8; line++)
        {
            HLMC_INTRA_MODE_get_ref_pixel_16x1(regs,
                cur_cu,
                ref_pixel_16x1,
                line,
                0,
#if  LINE_BY_LINE_4x1
                pred_lenght,
                pred_mode_4x1);
#else
                &pred_mode);
#endif
            distortion += HLMC_INTRA_16x1_TQ_Process(regs, cur_cu, line);
        }
    }
#if LINE_BY_LINE_4x1
    LINE_BY_LINE_RD_CAL(regs, distortion, best_cu, cur_cu);
    distortion = 0;
    cur_cu->intra_rd_cost = 0;
    cur_cu->com_cu_info.cu_type = HLM_I_ROW;
#else
    else
#endif
    {
#if LINE_BY_LINE_4x1
        pred_mode_4x1 = &(cur_cu->pu_info_enc[0].inter_pu_info.row_by_row_mode[0]);
#else
        pred_mode -= 4;
#endif
        for (line = 0; line < 16; line++)
        {
            HLMC_INTRA_MODE_get_ref_pixel_1x8(regs,
                cur_cu,
                ref_pixel_16x1,
                line,
                0,
#if  LINE_BY_LINE_4x1
                pred_lenght,
                pred_mode_4x1);
#else
                &pred_mode);
#endif
            distortion += HLMC_INTRA_1x8_TQ_Process(regs, cur_cu, line);
        }
    }
    LINE_BY_LINE_RD_CAL(regs, distortion, best_cu, cur_cu);
}
/***************************************************************************************************
* 功  能：逐行预测 RD计算模块
* 参  数：*
*        regs                 -I    当前数据硬件寄存器
*        distortion            -I   当前模式损失
*        best_mb              -IO   最优ctu
*        cur_mb               -IO   当前ctu
* 返回值：无
***************************************************************************************************/
HLM_VOID LINE_BY_LINE_RD_CAL(HLMC_REGS              *regs,
                             HLM_U32                 distortion,
                             HLMC_CU_INFO           *best_cu,
                             HLMC_CU_INFO           *cur_cu)
{
    HLM_U08 num_comp = (regs->image_format == HLM_IMG_YUV_400) ? 1 : 3;
    HLM_U08 yuv_idx = 0;
    HLM_U32 bits = 0;
    HLM_U08 i = 0;
    HLM_U08 j = 0;
    HLM_COEFF *coef_src = HLM_NULL;
    HLM_U32 lamda = regs->enc_sse_lamda[cur_cu->com_cu_info.qp[0]];
    for (yuv_idx = 0; yuv_idx < num_comp; yuv_idx++)
    {
        for (j = 0; j <(1 << cur_cu->com_cu_info.cu_height[yuv_idx]); j++)
        {
            coef_src = cur_cu->com_cu_info.cu_pred_info.coeff[yuv_idx] + j * HLM_WIDTH_SIZE;
            for (i = 0; i < (1 << cur_cu->com_cu_info.cu_width[yuv_idx]); i++)
            {
                if (coef_src[i] != 0)
                {
                    cur_cu->com_cu_info.coeffs_num[yuv_idx][0][0]++;
                }
            }
            coef_src += HLM_WIDTH_SIZE;
        }
    }
    HLMC_COM_GetCbf(cur_cu->com_cu_info.cu_type, cur_cu->com_cu_info.cbf, cur_cu->com_cu_info.coeffs_num);
    bits += HLMC_ECD_EncodeCuType(cur_cu->com_cu_info.cu_type, regs->enc_frame_coding_type, 0, HLM_NULL,
        regs->enc_frame_coding_type == HLM_FRAME_TYPE_I ? regs->i_frame_enable_ibc : regs->p_frame_enable_ibc);
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
#if LINE_BY_LINE_4x1_RESI
        HLMC_ECD_resi_pred(cur_cu, 0);
        HLMC_ECD_resi_pred(cur_cu, 1);
        HLMC_ECD_resi_pred(cur_cu, 2);
        bits += 6;
#endif
        bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, NULL, 0);
        bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, NULL, 1);
        bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, NULL, 2);
    }
    cur_cu->intra_rd_cost += distortion + ((lamda * bits) >> (HLMC_LAMBDA_SHIFT ));
    if (cur_cu->intra_rd_cost < best_cu->intra_rd_cost)
    {
        HLMC_COM_CopyCuInfo(cur_cu, best_cu, regs->image_format == HLM_IMG_YUV_400 ? 1 : 3);
    }
}

#endif
/***************************************************************************************************
* 功  能：SCC RDO模块
* 参  数：*
*        regs                 -I    当前数据硬件寄存器
*        nbi_info             -I    相邻块信息
*        best_cu              -O    最优cu
*        cur_cu               -I    当前cu
* 返回值：无
***************************************************************************************************/
HLM_VOID HLMC_SCC_RDO(HLMC_REGS              *regs,
                      HLM_NEIGHBOR_INFO      *nbi_info,
                      HLMC_CU_INFO           *best_cu,
                      HLMC_CU_INFO           *cur_cu)
{
    HLM_U32 zscan_idx     = 0;  // 光栅扫描顺序
    HLM_S32 pu_x          = 0;  // 4x4子宏块相对于宏块左上角的x/y坐标，以子宏块为单位
    HLM_S32 pu_y          = 0;
    HLM_S32 pu_x_in_pic   = 0;
    HLM_S32 pu_y_in_pic   = 0;
    HLM_S32 pu_x_in_area  = 0;
    HLM_S32 pu_y_in_area  = 0;
    HLM_S32 cu_pixel_x    = cur_cu->com_cu_info.cu_x << HLM_LOG2_WIDTH_SIZE;  // 当前CU的坐标，以像素为单位
    HLM_S32 cu_pixel_y    = cur_cu->com_cu_info.cu_y << HLM_LOG2_HEIGHT_SIZE;
    HLM_U16 *ori_ptr[3]   = { 0 };  // 原始图像指针，指向图像的左上角点
    HLM_U16 *rec_ptr[3]   = { 0 };  // 重建图像指针，指向图像的左上角点
    HLM_U32 stride[3]     = { 0 };
    HLM_U32 dist          = 0;
    HLM_U32 bits          = 0;
    HLM_U32 lambda        = regs->enc_sse_lamda[cur_cu->com_cu_info.qp[0]];
    HLM_U16 *dst          = 0;
    HLM_U16 *src          = 0;
    HLM_U08 ts_flag       = 0;
    HLM_U08 yuv_idx       = 0;
    HLM_U08 i             = 0;
    HLM_S32 rec_len       = cu_pixel_x - HLM_MAX(0, cu_pixel_x - HLM_IBC_BUFFER_WIDTH);
    HLM_S32 pad_len       = HLM_IBC_BUFFER_WIDTH - rec_len;
    HLM_S32 yuv_comp      = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_S08 hor_shift[3]  = { 0 };
    HLM_S08 ver_shift[3]  = { 0 };
    HLM_U08 ts_flag_start = 0;
    HLM_U32 pu_w          = 0;
    HLM_U32 pu_h          = 0;
    HLM_U32 pu_pos        = 0;
    HLM_U08 j             = 0;
    HLM_COEFF *coef_src   = HLM_NULL;
    HLM_COEFF *coef_dst   = HLM_NULL;
    HLM_S32 boundary_x    = 0;
    HLM_S32 boundary_y    = 0;
#if MIX_IBC
    HLM_IBC_PU_INFO pb_info  = { 0 };  // 区分亮色度
    HLM_IBC_PU_INFO *pu_info = HLM_NULL;
    HLM_U16 *cb_pred         = HLM_NULL;
    HLM_MV *cur_bv           = HLM_NULL;
    HLM_U08 bvx_bits         = 0;
    HLM_U08 direct           = 0;
    HLM_U16 *up_rec[3]       = { 0 };  // 上一行的重建像素
    HLM_U08 bvy_zero_flag    = cur_cu->com_cu_info.bvy_zero_flag;
    HLM_U08 mix_ibc_flag     = cur_cu->com_cu_info.mix_ibc_flag;
#endif

#if IBC_FORCE_TS
    ts_flag_start = 1;
#else
    ts_flag_start = 0;
#endif
    stride[0]  = regs->enc_input_luma_stride;
    stride[1]  = regs->enc_input_chroma_stride;
    stride[2]  = regs->enc_input_chroma_stride;
    ori_ptr[0] = regs->enc_input_y_base;
    ori_ptr[1] = regs->enc_input_cb_base;
    ori_ptr[2] = regs->enc_input_cr_base;
    rec_ptr[0] = regs->enc_recon_y_base;
    rec_ptr[1] = regs->enc_recon_cb_base;
    rec_ptr[2] = regs->enc_recon_cr_base;
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
            HLM_COM_SearchAreaPadding(cur_cu->com_cu_info.search_area[yuv_idx], rec_ptr[yuv_idx], regs->bitdepth, stride[yuv_idx],
                                      pad_len >> hor_shift[yuv_idx], HLM_IBC_SEARCH_AREA_HEIGHT >> ver_shift[yuv_idx],
                                      ver_shift[yuv_idx], boundary_x, boundary_y,
                                      cur_cu->com_cu_info.cu_x, cur_cu->com_cu_info.cu_y);
        }
    }

    // 决策ts_flag
    for (ts_flag = ts_flag_start; ts_flag < 2; ts_flag++)
    {
        cur_cu->com_cu_info.ts_flag = ts_flag;
        cur_cu->com_cu_info.cu_type = HLM_IBC_4x4;
        dist = 0;
        bits = 0;
        for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
        {
            pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
            pu_y = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
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
                    pu_x_in_area + (cur_cu->bv_enc[cur_cu->com_cu_info.merge_flag][zscan_idx].mvx >> hor_shift[yuv_idx]),
                    pu_y_in_area + (cur_cu->bv_enc[cur_cu->com_cu_info.merge_flag][zscan_idx].mvy >> ver_shift[yuv_idx]),
                    pu_w, pu_h);
#endif
                HLMC_COM_ComputeRes(ori_ptr[yuv_idx] + pu_y_in_pic * stride[yuv_idx] + pu_x_in_pic,
                    cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos, cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
                    pu_w, pu_h, stride[yuv_idx], HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
            }
            HLMC_TQ_Process((pu_y << 2) + pu_x, 4, regs->bitdepth, yuv_comp, cur_cu);

            for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
            {
                pu_w         = 4 >> hor_shift[yuv_idx];
                pu_h         = 4 >> ver_shift[yuv_idx];
                pu_pos       = (pu_y * pu_h) * HLM_WIDTH_SIZE + (pu_x * pu_w);
                pu_x_in_pic  = (cu_pixel_x >> hor_shift[yuv_idx]) + pu_x * pu_w;
                pu_y_in_pic  = (cu_pixel_y >> ver_shift[yuv_idx]) + pu_y * pu_h;
                pu_x_in_area = (HLM_IBC_BUFFER_WIDTH >> hor_shift[yuv_idx]) + pu_x * pu_w;
                pu_y_in_area = pu_y * pu_h;
                cur_cu->com_cu_info.coeffs_num[yuv_idx][pu_y][pu_x] = 0;
                coef_src = cur_cu->com_cu_info.cu_pred_info.coeff[yuv_idx] + pu_pos;
                for (j = 0; j < pu_h; j++)
                {
                    for (i = 0; i < pu_w; i++)
                    {
                        if (coef_src[i] != 0)
                        {
                            cur_cu->com_cu_info.coeffs_num[yuv_idx][pu_y][pu_x]++;
                        }
                    }
                    coef_src += HLM_WIDTH_SIZE;
                }

                HLM_COM_AddRes(cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos,
                    cur_cu->com_cu_info.cu_pred_info.pred[yuv_idx] + pu_pos, cur_cu->com_cu_info.cu_pred_info.res[yuv_idx] + pu_pos,
                    regs->bitdepth, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
                dist += HLMC_COM_ComputeSse(ori_ptr[yuv_idx] + pu_y_in_pic * stride[yuv_idx] + pu_x_in_pic,
                    cur_cu->com_cu_info.cu_pred_info.rec[yuv_idx] + pu_pos, pu_w, pu_h, stride[yuv_idx], HLM_WIDTH_SIZE);

                // 拷贝重建像素、更新搜索区域
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

        // 比特估计
        HLMC_COM_GetCbf(cur_cu->com_cu_info.cu_type, cur_cu->com_cu_info.cbf, cur_cu->com_cu_info.coeffs_num);
        bits += HLMC_ECD_EncodeCuType(cur_cu->com_cu_info.cu_type, regs->enc_frame_coding_type, 0, HLM_NULL,
            regs->enc_frame_coding_type == HLM_FRAME_TYPE_I ? regs->i_frame_enable_ibc : regs->p_frame_enable_ibc);
        bits += (HLM_BV_MERGE_NUM > 2 ? 2 : 1);  // merge_flag
        bits += 1;  // ts_flag
#if MIX_IBC
#if FIRST_COLUMN_IBC
        if (!cur_cu->com_cu_info.first_column_ibc_flag)
#else
        assert(cur_cu->com_cu_info.first_column_ibc_flag == 0);
#endif
        {
            bits += 2;  // bvy_zero_flag
            bits += 2;  // mix_ibc_flag
        }
        bvx_bits = HLM_COM_GetBvxLen(&cur_cu->com_cu_info, regs->segment_enable_flag, regs->segment_width_in_log2);
        for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
        {
            direct = tbl_merge_type[cur_cu->com_cu_info.merge_flag][zscan_idx];
            pu_info = &cur_cu->com_cu_info.ibc_pu_info[cur_cu->com_cu_info.merge_flag][zscan_idx];
            if ((zscan_idx < 4 ? (mix_ibc_flag >> 1) : (mix_ibc_flag & 1))
#if FIRST_COLUMN_IBC
                && !cur_cu->com_cu_info.first_column_ibc_flag
#endif
                )
            {
                bits += HLMC_ECD_EncodePartType(pu_info->part_type, HLM_NULL, 1);
            }
            for (i = 0; i < pu_info->sub_pu_num; i++)
            {
                if (direct == 0 && i == 0)  // 定长码
                {
                    bits += bvx_bits;
                }
                else
                {
                    bits += HLMC_ECD_PutSeBits(HLM_NULL, pu_info->sub_bvd[i].mvx, 0, "bv_x");
                }
                bits += (zscan_idx < 4 ? (bvy_zero_flag >> 1) : (bvy_zero_flag & 1)) ? 0 : 2;  // y定长
            }
        }
#else
#if BVY_ZERO
        bits += BVY_ZERO == 1 ? 1 : 2;  // bvy_zero_flag
#endif
        for (i = 0; i < 8; i++)
        {
            bits += HLMC_ECD_EncodeBV(regs->segment_enable_flag ? rec_len >> HLM_LOG2_WIDTH_SIZE : cur_cu->com_cu_info.cu_x,
#if BVY_ZERO == 1
                                      cur_cu->com_cu_info.bvy_zero_flag,
#elif BVY_ZERO == 2
                                      (i < 4) ? (cur_cu->com_cu_info.bvy_zero_flag >> 1) : (cur_cu->com_cu_info.bvy_zero_flag & 1),
#elif BVY_ZERO == 3
                                      ((i >> 1) & 1) == 0 ? (cur_cu->com_cu_info.bvy_zero_flag >> 1) : (cur_cu->com_cu_info.bvy_zero_flag & 1),
#endif
                                      cur_cu->com_cu_info.merge_flag, i, cur_cu->bv_enc[cur_cu->com_cu_info.merge_flag], HLM_NULL, 1);
        }
#endif
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
            bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 1);
            bits += HLMC_ECD_EstimateCoeff16x8(cur_cu, nbi_info, 2);
        }
        cur_cu->intra_rd_cost = dist + ((lambda * bits) >> HLMC_LAMBDA_SHIFT);
        if (cur_cu->intra_rd_cost < best_cu->intra_rd_cost)
        {
            HLMC_COM_CopyCuInfo(cur_cu, best_cu, regs->image_format == HLM_IMG_YUV_400 ? 1 : 3);
        }
    }
}
