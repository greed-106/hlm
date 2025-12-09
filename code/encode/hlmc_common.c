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
#include "hlmc_common.h"
#if SIMD
#include <smmintrin.h>
#include <immintrin.h>

#define SSE_SAD_16B_4PEL(src1, src2, s0, s1, ot) \
    s0 = _mm_loadl_epi64((__m128i*)(src1)); \
    s1 = _mm_loadl_epi64((__m128i*)(src2));\
    ot = _mm_sub_epi16(s0, s1); \
    ot = _mm_abs_epi16(ot); \
    ot = _mm_cvtepi16_epi32(ot);
#endif

// 将cur_cu_info的信息复制给best_cu_info
HLM_VOID HLMC_COM_copy_com_cu(HLM_CU_INFO    *cur_cu,
                              HLM_CU_INFO    *best_cu,
                              HLM_U32         yuv_comp)
{
    HLM_U32 i                      = 0;
    HLM_CU_PRED_INFO *cur_cu_info  = &cur_cu->cu_pred_info;
    HLM_CU_PRED_INFO *best_cu_info = &best_cu->cu_pred_info;

    best_cu->cu_x            = cur_cu->cu_x;
    best_cu->cu_y            = cur_cu->cu_y;
    best_cu->cu_type         = cur_cu->cu_type;
    best_cu->last_code_qp    = cur_cu->last_code_qp;
    best_cu->merge_flag      = cur_cu->merge_flag;
    best_cu->left_unavail    = cur_cu->left_unavail;
    best_cu->up_unavail      = cur_cu->up_unavail;
    best_cu->chroma_offset_x = cur_cu->chroma_offset_x;
    best_cu->chroma_offset_y = cur_cu->chroma_offset_y;
    if (cur_cu->cu_type == HLM_IBC_4x4 || cur_cu->cu_type == HLM_I_LINE || cur_cu->cu_type == HLM_I_ROW)
    {
        best_cu->ts_flag = cur_cu->ts_flag;
    }
    else
    {
        best_cu->ts_flag = 0;  // intra和inter强制变换
    }
    memcpy(best_cu->cu_width, cur_cu->cu_width, sizeof(HLM_U08) * 3);
    memcpy(best_cu->cu_height, cur_cu->cu_height, sizeof(HLM_U08) * 3);
    memcpy(best_cu->qp, cur_cu->qp, sizeof(HLM_U08) * 3);
    memcpy(best_cu->cbf, cur_cu->cbf, sizeof(HLM_U08) * 3);
    memcpy(best_cu->coeffs_num, cur_cu->coeffs_num, sizeof(HLM_U08) * 3 * 2 * 4);

    best_cu->bvy_zero_flag = cur_cu->bvy_zero_flag;
    best_cu->mix_ibc_flag = cur_cu->mix_ibc_flag;
    memcpy(best_cu->inner_bv_left, cur_cu->inner_bv_left, sizeof(HLM_MV) * HLM_BV_MERGE_NUM * 8);
    memcpy(best_cu->inner_bv_up, cur_cu->inner_bv_up, sizeof(HLM_MV) * HLM_BV_MERGE_NUM * 16);
    memcpy(best_cu->ibc_pu_info, cur_cu->ibc_pu_info, sizeof(HLM_IBC_PU_INFO) * HLM_BV_MERGE_NUM * 8);

    // 复制cu_pred_info
    best_cu_info->part_type    = cur_cu_info->part_type;
    best_cu_info->skip_mvp.mvx = cur_cu_info->skip_mvp.mvx;
    best_cu_info->skip_mvp.mvy = cur_cu_info->skip_mvp.mvy;
    memcpy(best_cu_info->pu_info, cur_cu_info->pu_info, HLM_TU_4x4_NUMS * sizeof(HLM_PU_INFO));

    for (i = 0; i < yuv_comp; i++)
    {
        memcpy(best_cu_info->skip_pred[i],  cur_cu_info->skip_pred[i],  sizeof(HLM_U16)   * HLM_CU_SIZE);
        memcpy(best_cu_info->pred[i],       cur_cu_info->pred[i],       sizeof(HLM_U16)   * HLM_CU_SIZE);
        memcpy(best_cu_info->inter_pred[i], cur_cu_info->inter_pred[i], sizeof(HLM_U16)   * HLM_CU_SIZE);
        memcpy(best_cu_info->rec[i],        cur_cu_info->rec[i],        sizeof(HLM_U16)   * HLM_CU_SIZE);
        memcpy(best_cu_info->res[i],        cur_cu_info->res[i],        sizeof(HLM_COEFF) * HLM_CU_SIZE);
        memcpy(best_cu_info->coeff[i],      cur_cu_info->coeff[i],      sizeof(HLM_COEFF) * HLM_CU_SIZE);
    }
}

/***************************************************************************************************
* 功  能：将cur_cu_info的信息复制给best_cu_info
* 参  数：*
*         cur_cu           -I    cur_cu的起始地址
*         best_cu          -O    best_cu的起始地址
*         yuv_comp         -I    分量个数
* 返回值：
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_COM_CopyCuInfo(HLMC_CU_INFO       *cur_cu,
                             HLMC_CU_INFO       *best_cu,
                             HLM_U32             yuv_comp)
{
    HLM_U32 i = 0;
    HLM_U32 j = 0;

    HLMC_COM_copy_com_cu(&cur_cu->com_cu_info, &best_cu->com_cu_info, yuv_comp);

    for (i = 0; i < yuv_comp; i++)
    {
        best_cu->satd_comp[i] = cur_cu->satd_comp[i];
        best_cu->quant_params.bias_f[i] = cur_cu->quant_params.bias_f[i];
    }
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 16; j++)
        {
            best_cu->quant_params.dequant_v[i][j] = cur_cu->quant_params.dequant_v[i][j];
        }
    }
    for (j = 0; j < 2; j++)
    {
        memcpy(&best_cu->pu_info_enc[j].inter_pu_info, &cur_cu->pu_info_enc[j].inter_pu_info, sizeof(HLM_PU_INFO));
        best_cu->pu_info_enc[j].mv_16x8 = cur_cu->pu_info_enc[j].mv_16x8;
        best_cu->pu_info_enc[j].mv_8x8 = cur_cu->pu_info_enc[j].mv_8x8;
    }
    best_cu->intra_rd_cost   = cur_cu->intra_rd_cost;
    best_cu->intra_satd_cost = cur_cu->intra_satd_cost;
    best_cu->inter_satd_cost = cur_cu->inter_satd_cost;
    best_cu->mix_flag        = cur_cu->mix_flag;
}

/***************************************************************************************************
* 功  能：计算两块block的SSE
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         block_width     -I    块宽度
*         block_height    -I    块高度
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
* 返回值：两块block的SSE值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_COM_ComputeSse(HLM_U16 *block0,
                            HLM_U16 *block1,
                            HLM_U32  block_width,
                            HLM_U32  block_height,
                            HLM_U32  block0_stride,
                            HLM_U32  block1_stride)
{
    HLM_U32  i          = 0;
    HLM_U32  j          = 0;
    HLM_U32  tmp        = 0;
    HLM_U32  sse        = 0;
    HLM_U16 *block0_tmp = block0;
    HLM_U16 *block1_tmp = block1;

    for (i = 0; i < block_height; i++)
    {
        for (j = 0; j < block_width; j++)
        {
            tmp = HLM_ABS(block0_tmp[j] - block1_tmp[j]);
            sse += (tmp * tmp);
        }
        block0_tmp += block0_stride;
        block1_tmp += block1_stride;
    }

    return sse;
}

/***************************************************************************************************
* 功  能：计算两块block的残差
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         res             -O    残差的起始地址
*         block_width     -I    块宽度
*         block_height    -I    块高度
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
*         res_stride      -I    残差的跨度
* 返回值：
* 备  注：res = block0 - block1
***************************************************************************************************/
HLM_VOID HLMC_COM_ComputeRes(HLM_U16   *block0,
                             HLM_U16   *block1,
                             HLM_COEFF *res,
                             HLM_U32    block_width,
                             HLM_U32    block_height,
                             HLM_U32    block0_stride,
                             HLM_U32    block1_stride,
                             HLM_U32    res_stride)
{
    HLM_U32  i          = 0;
    HLM_U32  j          = 0;
    HLM_COEFF *res_tmp  = res;
    HLM_U16 *block0_tmp = block0;
    HLM_U16 *block1_tmp = block1;

    for (i = 0; i < block_height; i++)
    {
        for (j = 0; j < block_width; j++)
        {
            res_tmp[j] = (HLM_COEFF)(block0_tmp[j] - block1_tmp[j]);
        }
        res_tmp += res_stride;
        block0_tmp += block0_stride;
        block1_tmp += block1_stride;
    }
}

/***************************************************************************************************
* 功  能：计算两块block的SAD
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         block_width     -I    两块block的宽度
*         block_height    -I    两块block的高度
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
* 返回值：两块block的SAD值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_COM_ComputeSad(HLM_U16 *block0,
                            HLM_U16 *block1,
                            HLM_U32  block_width,
                            HLM_U32  block_height,
                            HLM_U32  block0_stride,
                            HLM_U32  block1_stride)
{
    HLM_U32  i          = 0;
    HLM_U32  j          = 0;
    HLM_U32  sad        = 0;
    HLM_U16 *block0_tmp = block0;
    HLM_U16 *block1_tmp = block1;
#if SIMD
    __m128i s0, s1, sum, tmp;

    if (block_width == 4)
    {
        SSE_SAD_16B_4PEL(block0_tmp, block1_tmp, s0, s1, sum);
        for (i = 1; i < block_height; i++)
        {
            block0_tmp += block0_stride;
            block1_tmp += block1_stride;
            SSE_SAD_16B_4PEL(block0_tmp, block1_tmp, s0, s1, tmp);
            sum = _mm_add_epi32(sum, tmp);
        }
        sum = _mm_hadd_epi32(sum, _mm_setzero_si128());
        sum = _mm_hadd_epi32(sum, _mm_setzero_si128());
        sad = _mm_extract_epi32(sum, 0);
    }
    else
#endif
    for (i = 0; i < block_height; i++)
    {
        for (j = 0; j < block_width; j++)
        {
            sad += HLM_ABS(block0_tmp[j] - block1_tmp[j]);
        }
        block0_tmp += block0_stride;
        block1_tmp += block1_stride;
    }

    return sad;
}

/***************************************************************************************************
* 功  能：计算两块ibc4x4块的SAD
* 参  数：*
*         block0                 -I      第一块block的起始地址
*         block1                 -I      第二块block的起始地址
*         sad                    -O      sad
*         sub_ibc_enable_flag    -I      子块ibc使能标记
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_COM_ComputeIbc4x4Sad(HLM_U16 *block0,
                                   HLM_U16 *block1,
                                   HLM_U32  sad[HLM_IBC_PART_NUM][4],
                                   HLM_U08  sub_ibc_enable_flag)
{
#if SIMD
    __m128i row0, row1, row2, row3;
    __m128i s0, s1, tmp;

    // 计算绝对差
    SSE_SAD_16B_4PEL(block0,     block1,       s0, s1, row0);
    SSE_SAD_16B_4PEL(block0 + 4, block1 + 4,   s0, s1, row1);
    SSE_SAD_16B_4PEL(block0 + 8, block1 + 8,   s0, s1, row2);
    SSE_SAD_16B_4PEL(block0 + 12, block1 + 12, s0, s1, row3);

    // 计算行和
    s0 = _mm_hadd_epi32(row0, row1);
    s1 = _mm_hadd_epi32(row2, row3);
    tmp = _mm_hadd_epi32(s0, s1);
    _mm_store_si128((__m128i *)sad[HLM_IBC_HOR_SYM4], tmp);

    // 不划分
    sad[HLM_IBC_NO_SPLIT][0] = sad[HLM_IBC_HOR_SYM4][0] + sad[HLM_IBC_HOR_SYM4][1]
                             + sad[HLM_IBC_HOR_SYM4][2] + sad[HLM_IBC_HOR_SYM4][3];

    if (!sub_ibc_enable_flag)
    {
        return;
    }

    // 计算列和
    s0 = _mm_add_epi32(row0, row1);
    s1 = _mm_add_epi32(row2, row3);
    tmp = _mm_add_epi32(s0, s1);
    _mm_store_si128((__m128i *)sad[HLM_IBC_VER_SYM4], tmp);

    // 四叉树划分
    tmp = _mm_hadd_epi32(s0, s1);
    _mm_store_si128((__m128i *)sad[HLM_IBC_QT], tmp);
#else
    HLM_U16 *block0_tmp = block0;
    HLM_U16 *block1_tmp = block1;
    HLM_U08 i           = 0;
    HLM_U08 j           = 0;
    HLM_U32 df[4][4]    = { { 0 } };
    HLM_U32 r[4]        = { 0 };  // 行和
    HLM_U32 c[2][4]     = { 0 };  // 列和

    for (i = 0; i < 4; i++)  // 行
    {
        for (j = 0; j < 4; j++)  // 列
        {
            df[i][j] = HLM_ABS(block0_tmp[j] - block1_tmp[j]);
        }
        block0_tmp += 4;
        block1_tmp += 4;
    }

    // 计算行和
    r[0] = df[0][0] + df[0][1] + df[0][2] + df[0][3];
    r[1] = df[1][0] + df[1][1] + df[1][2] + df[1][3];
    r[2] = df[2][0] + df[2][1] + df[2][2] + df[2][3];
    r[3] = df[3][0] + df[3][1] + df[3][2] + df[3][3];

    // 不划分
    sad[HLM_IBC_NO_SPLIT][0] = r[0] + r[1] + r[2] + r[3];

    if (!sub_ibc_enable_flag)
    {
        return;
    }

    // 水平划分
    sad[HLM_IBC_HOR_SYM4][0] = r[0];
    sad[HLM_IBC_HOR_SYM4][1] = r[1];
    sad[HLM_IBC_HOR_SYM4][2] = r[2];
    sad[HLM_IBC_HOR_SYM4][3] = r[3];

    // 计算列和
    c[0][0] = df[0][0] + df[1][0];
    c[0][1] = df[0][1] + df[1][1];
    c[0][2] = df[0][2] + df[1][2];
    c[0][3] = df[0][3] + df[1][3];
    c[1][0] = df[2][0] + df[3][0];
    c[1][1] = df[2][1] + df[3][1];
    c[1][2] = df[2][2] + df[3][2];
    c[1][3] = df[2][3] + df[3][3];

    // 垂直划分
    sad[HLM_IBC_VER_SYM4][0] = c[0][0] + c[1][0];
    sad[HLM_IBC_VER_SYM4][1] = c[0][1] + c[1][1];
    sad[HLM_IBC_VER_SYM4][2] = c[0][2] + c[1][2];
    sad[HLM_IBC_VER_SYM4][3] = c[0][3] + c[1][3];

    // 四叉树划分
    sad[HLM_IBC_QT][0] = c[0][0] + c[0][1];
    sad[HLM_IBC_QT][1] = c[0][2] + c[0][3];
    sad[HLM_IBC_QT][2] = c[1][0] + c[1][1];
    sad[HLM_IBC_QT][3] = c[1][2] + c[1][3];
#endif
}

/***************************************************************************************************
* 功  能：导出mix_ibc_flag和bvy_zero_flag
* 参  数：*
*        com_cu_info            -I         当前CU信息
*        merge_flag             -I         merge方式
*        mix_ibc_flag           -O         是否为混合ibc
*        bvy_zero_flag          -O         bvy是否全零
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_COM_DeriveMixIbcInfo(HLM_CU_INFO  *com_cu_info,
                                   HLM_U08       merge_flag,
                                   HLM_U08      *mix_ibc_flag,
                                   HLM_U08      *bvy_zero_flag)
{
    HLM_U08 sub_bvy_zero_flag[2] = { 0 };
    HLM_U08 sub_mix_ibc_flag[2]  = { 0 };
    HLM_IBC_PU_INFO *pu_info     = HLM_NULL;
    HLM_U08 zscan_idx            = 0;
    HLM_U08 i                    = 0;

    sub_bvy_zero_flag[0] = 1;
    sub_bvy_zero_flag[1] = 1;
    sub_mix_ibc_flag[0]  = 0;
    sub_mix_ibc_flag[1]  = 0;
    for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
    {
        pu_info = &com_cu_info->ibc_pu_info[merge_flag][zscan_idx];
        if (pu_info->part_type != HLM_IBC_NO_SPLIT)
        {
            sub_mix_ibc_flag[zscan_idx >> 2] = 1;
        }
        for (i = 0; i < pu_info->sub_pu_num; i++)
        {
            if (pu_info->sub_bv[i].mvy != 0)
            {
                sub_bvy_zero_flag[zscan_idx >> 2] = 0;
            }
        }
    }
    *bvy_zero_flag = (sub_bvy_zero_flag[0] << 1) + sub_bvy_zero_flag[1];
    *mix_ibc_flag = (sub_mix_ibc_flag[0] << 1) + sub_mix_ibc_flag[1];
}

/***************************************************************************************************
* 功  能：计算两块4x4block的satd
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
* 返回值：两块block的satd值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_COM_ComputeSatd4x4(HLM_U16 *block0,
                                HLM_U16 *block1,
                                HLM_U32  block0_stride,
                                HLM_U32  block1_stride)
{
    HLM_U32 k           = 0;
    HLM_U32 satd        = 0;
    HLM_S32 diff[16]    = { 0 };
    HLM_S32 m[16]       = { 0 };
    HLM_S32 d[16]       = { 0 };
    HLM_U16 *block0_tmp = block0;
    HLM_U16 *block1_tmp = block1;

    for (k = 0; k < 16; k += 4)
    {
        diff[k + 0] = block0_tmp[0] - block1_tmp[0];
        diff[k + 1] = block0_tmp[1] - block1_tmp[1];
        diff[k + 2] = block0_tmp[2] - block1_tmp[2];
        diff[k + 3] = block0_tmp[3] - block1_tmp[3];
        block0_tmp += block0_stride;
        block1_tmp += block1_stride;
    }

    /*===== hadamard transform =====*/
    m[0] = diff[0] + diff[12];
    m[1] = diff[1] + diff[13];
    m[2] = diff[2] + diff[14];
    m[3] = diff[3] + diff[15];
    m[4] = diff[4] + diff[8];
    m[5] = diff[5] + diff[9];
    m[6] = diff[6] + diff[10];
    m[7] = diff[7] + diff[11];
    m[8] = diff[4] - diff[8];
    m[9] = diff[5] - diff[9];
    m[10] = diff[6] - diff[10];
    m[11] = diff[7] - diff[11];
    m[12] = diff[0] - diff[12];
    m[13] = diff[1] - diff[13];
    m[14] = diff[2] - diff[14];
    m[15] = diff[3] - diff[15];

    d[0] = m[0] + m[4];
    d[1] = m[1] + m[5];
    d[2] = m[2] + m[6];
    d[3] = m[3] + m[7];
    d[4] = m[8] + m[12];
    d[5] = m[9] + m[13];
    d[6] = m[10] + m[14];
    d[7] = m[11] + m[15];
    d[8] = m[0] - m[4];
    d[9] = m[1] - m[5];
    d[10] = m[2] - m[6];
    d[11] = m[3] - m[7];
    d[12] = m[12] - m[8];
    d[13] = m[13] - m[9];
    d[14] = m[14] - m[10];
    d[15] = m[15] - m[11];

    m[0] = d[0] + d[3];
    m[1] = d[1] + d[2];
    m[2] = d[1] - d[2];
    m[3] = d[0] - d[3];
    m[4] = d[4] + d[7];
    m[5] = d[5] + d[6];
    m[6] = d[5] - d[6];
    m[7] = d[4] - d[7];
    m[8] = d[8] + d[11];
    m[9] = d[9] + d[10];
    m[10] = d[9] - d[10];
    m[11] = d[8] - d[11];
    m[12] = d[12] + d[15];
    m[13] = d[13] + d[14];
    m[14] = d[13] - d[14];
    m[15] = d[12] - d[15];

    d[0] = m[0] + m[1];
    d[1] = m[2] + m[3];
    d[2] = m[0] - m[1];
    d[3] = m[3] - m[2];
    d[4] = m[4] + m[5];
    d[5] = m[6] + m[7];
    d[6] = m[4] - m[5];
    d[7] = m[7] - m[6];
    d[8] = m[8] + m[9];
    d[9] = m[10] + m[11];
    d[10] = m[8] - m[9];
    d[11] = m[11] - m[10];
    d[12] = m[12] + m[13];
    d[13] = m[14] + m[15];
    d[14] = m[12] - m[13];
    d[15] = m[15] - m[14];

    satd += HLM_ABS(d[0]);
    for (k = 1; k < 16; ++k)
    {
        satd += HLM_ABS(d[k] << 2);
    }
    satd = ((satd + 32) >> 6);

    return satd;
}

/***************************************************************************************************
* 功  能：计算两个块的satd或sad，使用4x4为基本单元
* 参  数：*
*         block0          -I    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         satd_record_4x4 -I    记录4x4子块的satd值
*         block_w         -I    两块block的宽度
*         block_h         -I    两块block的高度
*         stride_0        -I    第一块block的跨度
*         stride_1        -I    第二块block的跨度
* 返回值：两块block的satd值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_COM_ComputeDist16x8(HLM_U16  *block_0,
                                 HLM_U16  *block_1,
                                 HLM_U32  *satd_record_4x4,
                                 HLM_S32   block_w,
                                 HLM_S32   block_h,
                                 HLM_U32   stride_0,
                                 HLM_U32   stride_1)
{
    HLM_U32 satd      = 0;
    HLM_S32 row       = 0;
    HLM_S32 col       = 0;
    HLM_U32 satd_temp = 0;
    HLM_S32 i         = 0;

    for (row = 0; row < block_h; row += 4)
    {
        for (col = 0; col < block_w; col += 4)
        {
            satd_temp = HLMC_COM_ComputeSatd4x4(block_0 + row * stride_0 + col,
                block_1 + row * stride_1 + col, stride_0, stride_1);
            satd += satd_temp;
            satd_record_4x4[i++] += satd_temp;
        }
    }

    return satd;
}

/***************************************************************************************************
* 功  能：计算cbf
* 参  数：*
*         cu_type          -I    当前宏块类型
*         cbf              -O    三分量cbf
*         coeffs_num       -I    当前宏块4x4非零系数个数
* 返回值：
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_COM_GetCbf(HLM_CU_TYPE   cu_type,
                         HLM_U08       cbf[3],
                         HLM_U08       coeffs_num[3][2][4])
{
    HLM_U08 yuv_idx = 0;
    HLM_U08 x       = 0;
    HLM_U08 y       = 0;
    HLM_U08 nnz_4x4 = 0;

    for (yuv_idx = 0; yuv_idx < 3; yuv_idx++)
    {
        nnz_4x4 = 0;
        for (y = 0; y < 2; y += 1)
        {
            for (x = 0; x < 4; x += 1)
            {
                nnz_4x4 |= (coeffs_num[yuv_idx][y][x]);
            }
        }
        cbf[yuv_idx] = (nnz_4x4 != 0);
    }
}
