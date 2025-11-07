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
#include "hlmd_tq.h"

// 反变换量化
HLM_VOID HLMD_TQ_process_4x4(HLM_U32         raster_idx,
                             HLM_U08         bitdepth,
                             HLM_U32         comp,
                             HLMD_CU_INFO   *cur_cu)
{
    HLM_U32 bit_depth        = HLM_LUMA_Y == comp ? cur_cu->luma_bitdepth : cur_cu->chroma_bitdepth;
    HLM_U32 idx              = 0;
    HLM_U08 x                = HLM_B4_X_8[raster_idx];
    HLM_U08 y                = HLM_B4_Y_8[raster_idx];
    HLM_S32 dequant_v[6][16] = { 0 };
    HLM_U08 quant_shift      = bit_depth == 8 ? 0 : 1;
    HLM_U08 residual_shift   = bit_depth == 8 ? 6 : 5;
    HLM_U08 residual_offset  = 1 << (residual_shift - 1);
    HLM_U08 qp               = cur_cu->com_cu_info.qp[comp];
    HLM_U32 pu_x             = raster_idx % 4;
    HLM_U32 pu_y             = raster_idx >> 2;
    HLM_U32 pu_w             = (cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_cu->com_cu_info.ts_flag == 1)
                             ? 1 << cur_cu->com_cu_info.cu_width[comp] >> 2 : 4;   // 水平4个PU
    HLM_U32 pu_h             = (cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_cu->com_cu_info.ts_flag == 1)
                             ? 1 << cur_cu->com_cu_info.cu_height[comp] >> 1 : 4;  // 垂直2个PU
    HLM_U32 pu_pos           = (pu_y * pu_h) * HLM_WIDTH_SIZE + (pu_x * pu_w);
    HLM_COEFF *src           = cur_cu->com_cu_info.cu_pred_info.coeff[comp] + pu_pos;
    HLM_COEFF *dst           = cur_cu->com_cu_info.cu_pred_info.res[comp] + pu_pos;
    HLM_U08 cu_4x4_nums      = (cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_cu->com_cu_info.ts_flag == 1)
                             ? 8 : 1 << (cur_cu->com_cu_info.cu_height[comp] + cur_cu->com_cu_info.cu_width[comp] - 4);
    HLM_U32 j                = 0;
    HLM_U32 k                = 0;

    // 抽取4x4块的系数
    HLM_COM_4x4_Block_To_Coeff_Pos(cu_4x4_nums, raster_idx, &x, &y, &cur_cu->com_cu_info, comp);
    if (!(cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_cu->com_cu_info.ts_flag == 1))
    {
        pu_pos = (y * pu_h) * HLM_WIDTH_SIZE + (x * pu_w);
        src = cur_cu->com_cu_info.cu_pred_info.coeff[comp] + pu_pos;
        dst = cur_cu->com_cu_info.cu_pred_info.res[comp] + pu_pos;
    }

    for (k = 0; k < 6; k++)
    {
        for (j = 0; j < 16; j++)
        {
            idx = (j & 1) + ((j >> 2) & 1);
            dequant_v[k][j] = (HLM_DEQUANT_V[k][idx] << (4 - quant_shift));
        }
    }

    // 变换量化
    if (cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_cu->com_cu_info.ts_flag == 1)  // 像素域
    {
        if (cur_cu->com_cu_info.coeffs_num[comp][y][x])
        {
            HLM_COM_PixelDequant(src, dst, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, qp);
        }
        else
        {
            src = cur_cu->com_cu_info.cu_pred_info.res[comp] + pu_pos;
            for (j = 0; j < pu_h; j++)
            {
                memset(src, 0, sizeof(HLM_COEFF) * pu_w);
                src += HLM_WIDTH_SIZE;
            }
        }
        src = cur_cu->com_cu_info.cu_pred_info.coeff[comp] + pu_pos;
        for (j = 0; j < pu_h; j++)
        {
            memset(src, 0, sizeof(HLM_COEFF) * pu_w);
            src += HLM_WIDTH_SIZE;
        }
    }
    else  // 频域
    {
        if (cur_cu->com_cu_info.coeffs_num[comp][y][x])
        {
            HLM_COM_Dequant4x4(src, qp, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, dequant_v[qp % 6], dst);
            HLM_COM_Idct4x4(dst, dst, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, bit_depth);
            for (y = 0; y < pu_h; y++)
            {
                for (x = 0; x < pu_w; x++)
                {
                    dst[x] = (dst[x] + residual_offset) >> residual_shift;
                }
                dst += HLM_WIDTH_SIZE;
            }
        }
        else
        {
            for (j = 0; j < pu_h; j++)
            {
                memset(dst, 0, sizeof(HLM_COEFF) * pu_w);
                dst += HLM_WIDTH_SIZE;
            }
        }
    }

    // 重建
    HLM_COM_AddRes(cur_cu->com_cu_info.cu_pred_info.rec[comp] + pu_pos,
        cur_cu->com_cu_info.cu_pred_info.pred[comp] + pu_pos,
        cur_cu->com_cu_info.cu_pred_info.res[comp] + pu_pos,
        bit_depth, pu_w, pu_h, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE, HLM_WIDTH_SIZE);
}

/***************************************************************************************************
* 功  能：反量化反变换
* 参  数：*
*        cur_cu               -I/O   当前CU
*        raster_idx           -I     当处理I4*4时，传入其编号，其他则传入0
*        yuv_comp             -I     分量个数
* 备  注：
***************************************************************************************************/
HLM_VOID HLMD_TQ_Process(HLMD_CU_INFO  *cur_cu,
                         HLM_U32        raster_idx,
                         HLM_U32        yuv_comp)
{
    HLM_U08 cur_ts_flag      = cur_cu->com_cu_info.ts_flag;
    HLM_U08 i                = 0;
    HLM_U08 part_4x4         = 0;
    HLM_U08 cu_4x4_nums      = HLM_TU_4x4_NUMS;
    HLM_U08 pu8[8]           = { 0,1,4,5,2,3,6,7 };
    HLM_U08 chroma_4x4_block = cur_cu->com_cu_info.chroma_offset_y ? 1 : 4;
    HLM_U08 part_4x4_num     = 0;
    HLM_U08 tmp_idx          = 0;
    HLM_U08 bit_depth        = 0;

    if (HLM_I_4x4 == cur_cu->com_cu_info.cu_type || HLM_IBC_4x4 == cur_cu->com_cu_info.cu_type)
    {
        if (cur_cu->com_cu_info.intra_8x8_enable_flag && HLM_I_4x4 == cur_cu->com_cu_info.cu_type)
        {
            for (i = HLM_LUMA_Y; i < yuv_comp; i++)
            {
                part_4x4_num = i == HLM_LUMA_Y ? 4 : chroma_4x4_block;
                for (part_4x4 = 0; part_4x4 < part_4x4_num; part_4x4++)
                {
                    if (cur_cu->com_cu_info.chroma_offset_x + cur_cu->com_cu_info.chroma_offset_y != 0 && i != HLM_LUMA_Y)
                    {
                        tmp_idx = part_4x4 + raster_idx;
                    }
                    else
                    {
                        tmp_idx = pu8[part_4x4 + raster_idx * part_4x4_num];
                    }
                    bit_depth = i == HLM_LUMA_Y ? cur_cu->luma_bitdepth : cur_cu->chroma_bitdepth;
                    HLMD_TQ_process_4x4(tmp_idx, bit_depth, i, cur_cu);
                }
            }
        }
        else
        for (i = HLM_LUMA_Y; i < yuv_comp; i++)
        {
            if (i > 0 && cur_cu->com_cu_info.cu_width[1] != cur_cu->com_cu_info.cu_width[0]  // 420/422的色度
                && cur_cu->com_cu_info.cu_type == HLM_IBC_4x4 && cur_ts_flag == 0)
            {
                cur_cu->com_cu_info.ts_flag = 1;  // IBC强制变换跳过
            }
            bit_depth = i == HLM_LUMA_Y ? cur_cu->luma_bitdepth : cur_cu->chroma_bitdepth;
            HLMD_TQ_process_4x4(raster_idx, bit_depth, i, cur_cu);
            cur_cu->com_cu_info.ts_flag = cur_ts_flag;
        }
    }
    else
    {
        for (i = HLM_LUMA_Y; i < yuv_comp; i++)
        {
            cu_4x4_nums = 1 << (cur_cu->com_cu_info.cu_height[i] + cur_cu->com_cu_info.cu_width[i] - 4);
            for (part_4x4 = 0; part_4x4 < cu_4x4_nums; part_4x4++)
            {
                bit_depth = i == HLM_LUMA_Y ? cur_cu->luma_bitdepth : cur_cu->chroma_bitdepth;
                HLMD_TQ_process_4x4(part_4x4, bit_depth, i, cur_cu);
            }
        }
    }

    return;
}
