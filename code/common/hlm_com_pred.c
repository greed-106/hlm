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
#include "hlm_com_pred.h"
#include <string.h>

/***************************************************************************************************
* 功  能：以Copy方式填充搜索区域
* 参  数：*
*        search_area        -O        搜索区域指针
*        src_ptr            -I        数据源指针
*        src_stride         -I        数据源stride
*        len                -I        拷贝数据的长度
*        height             -I        拷贝数据的高度
*        x_in_src           -I        拷贝像素在数据源上的横坐标
*        y_in_src           -I        拷贝像素在数据源上的纵坐标
*        x_in_area          -I        拷贝位置在搜索区域上的横坐标
* 返回值：无
***************************************************************************************************/
HLM_VOID HLM_COM_SearchAreaCopy(HLM_U16    *search_area,
                                HLM_U16    *src_ptr,
                                HLM_U32     src_stride,
                                HLM_S32     len,
                                HLM_S32     height,
                                HLM_S32     x_in_src,
                                HLM_S32     y_in_src,
                                HLM_S32     x_in_area)
{
    HLM_U16 *dst = 0;
    HLM_U16 *src = 0;
    HLM_U08 i    = 0;

    src = src_ptr + y_in_src * src_stride + x_in_src;
    dst = search_area + x_in_area;
    for (i = 0; i < height; i++)
    {
        memcpy(dst, src, len * sizeof(HLM_U16));
        src += src_stride;
        dst += HLM_IBC_SEARCH_AREA_WIDTH;
    }
}

/***************************************************************************************************
* 功  能：以Padding方式填充搜索区域
* 参  数：*
*        search_area        -O        搜索区域指针
*        src_ptr            -I        数据源指针，获取首列块的上一行像素
*        bit_depth          -I        比特位宽
*        src_stride         -I        数据源stride
*        len                -I        Padding数据的长度
*        height             -I        Padding数据的高度
*        ver_shift          -I        垂直方向的色度偏移
*        boundary_x         -I        宏块在隔断区域中的横坐标
*        boundary_y         -I        宏块在隔断区域中的纵坐标
*        cu_x               -I        宏块在patch中的横坐标，以CU为单位
*        cu_y               -I        宏块在patch中的纵坐标，以CU为单位
* 返回值：无
***************************************************************************************************/
HLM_VOID HLM_COM_SearchAreaPadding(HLM_U16    *search_area,
                                   HLM_U16    *src_ptr,
                                   HLM_U32     bit_depth,
                                   HLM_U32     src_stride,
                                   HLM_S32     len,
                                   HLM_S32     height,
                                   HLM_S08     ver_shift,
                                   HLM_S32     boundary_x,
                                   HLM_S32     boundary_y,
                                   HLM_S32     cu_x,
                                   HLM_S32     cu_y)
{
    HLM_U16 *dst = 0;
    HLM_U16 *src = 0;
    HLM_U16 val  = 0;
    HLM_U08 i    = 0;
    HLM_U16 j    = 0;

    if (boundary_x == 0)
    {
        if (boundary_y == 0)
        {
            val = 1 << (bit_depth - 1);
        }
        else
        {
            val = *(src_ptr + ((cu_y << 3 >> ver_shift) - 1) * src_stride);
        }
        dst = search_area;
        for (i = 0; i < height; i++)
        {
            for (j = 0; j < len; j++)
            {
                dst[j] = val;
            }
            dst += HLM_IBC_SEARCH_AREA_WIDTH;
        }
    }
    else
    {
        src = search_area + len;
        dst = search_area;
        for (i = 0; i < height; i++)
        {
            for (j = 0; j < len; j++)
            {
                dst[j] = src[0];
            }
            src += HLM_IBC_SEARCH_AREA_WIDTH;
            dst += HLM_IBC_SEARCH_AREA_WIDTH;
        }
    }
}

#if MIX_IBC
/***************************************************************************************************
* 功  能：以Padding方式填充搜索区域
* 参  数：*
*        search_area        -O        搜索区域指针
*        ref_ptr            -I        上一行的重建像素
*        hor_shift          -I        水平方向的色度偏移
*        segment_width      -I        隔断参考单元或Patch的宽度，以像素为单位
* 返回值：无
***************************************************************************************************/
HLM_VOID HLM_COM_CopyUpRefPixel(HLM_U16    *search_area,
                                HLM_U16    *ref_ptr,
                                HLM_S08     hor_shift,
                                HLM_S32     segment_width)
{
    HLM_S32 valid_len = HLM_MIN(HLM_IBC_BUFFER_WIDTH, segment_width) >> hor_shift;
    HLM_U16 *dst      = HLM_NULL;
    HLM_U16 stride    = HLM_IBC_SEARCH_AREA_WIDTH;
    HLM_U08 i         = 0;
    HLM_U08 j         = 0;

    for (i = 0; i < valid_len; i++)
    {
        search_area[i] = ref_ptr[i];
    }
    for (i = valid_len; i < HLM_IBC_SEARCH_AREA_WIDTH; i++)
    {
        search_area[i] = search_area[valid_len - 1];
    }
    dst = search_area + stride;
    for (j = 1; j < HLM_IBC_SEARCH_AREA_HEIGHT; j++)
    {
        memcpy(dst, search_area, HLM_IBC_SEARCH_AREA_WIDTH * sizeof(HLM_U16));
        dst += stride;
    }
}

/***************************************************************************************************
* 功  能：根据4x4块的bv信息更新内部bv
* 参  数：*
*        com_cu_info            -IO        当前CU信息
*        zscan_idx              -I         当前4x4块的索引
*        merge_flag             -I         merge方式
*        part_type              -I         当前4x4块的划分类型
*        bv                     -I         当前4x4块的bv或子PU的bv
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_UpdateInnerBv(HLM_CU_INFO         *com_cu_info,
                               HLM_U32              zscan_idx,
                               HLM_U08              merge_flag,
                               HLM_IBC_PART_TYPE    part_type,
                               HLM_MV              *bv)
{
    HLM_U08 pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
    HLM_U08 pu_y = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
    HLM_MV *le   = com_cu_info->inner_bv_left[merge_flag] + (pu_y << 2);
    HLM_MV *up   = com_cu_info->inner_bv_up[merge_flag] + (pu_x << 2);

    if (part_type == HLM_IBC_NO_SPLIT)
    {
        le[0] = le[1] = le[2] = le[3] = bv[0];
        up[0] = up[1] = up[2] = up[3] = bv[0];
    }
    else if (part_type == HLM_IBC_HOR_SYM4)
    {
        le[0] = bv[0];
        le[1] = bv[1];
        le[2] = bv[2];
        le[3] = bv[3];
        up[0] = up[1] = up[2] = up[3] = bv[3];
    }
    else if (part_type == HLM_IBC_VER_SYM4)
    {
        up[0] = bv[0];
        up[1] = bv[1];
        up[2] = bv[2];
        up[3] = bv[3];
        le[0] = le[1] = le[2] = le[3] = bv[3];
    }
    else  // QT
    {
        le[0] = le[1] = bv[1];
        le[2] = le[3] = bv[3];
        up[0] = up[1] = bv[2];
        up[2] = up[3] = bv[3];
    }
}

/***************************************************************************************************
* 功  能：根据4x4块的划分类型获取子pu信息，包括pu_num、x/y/w/h
* 参  数：*
*        part_type              -I         当前4x4块的划分类型
*        pu_info                -O         当前4x4块的子PU信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_GetIbcPuInfo(HLM_IBC_PART_TYPE    part_type,
                              HLM_IBC_PU_INFO     *pu_info)
{
    HLM_U08 *x = pu_info->sub_x;
    HLM_U08 *y = pu_info->sub_y;
    HLM_U08 *w = pu_info->sub_w;
    HLM_U08 *h = pu_info->sub_h;
    HLM_U08 i  = 0;

    pu_info->part_type = part_type;
    pu_info->sub_pu_num = (part_type == HLM_IBC_NO_SPLIT) ? 1 : 4;
    if (part_type == HLM_IBC_NO_SPLIT)
    {
        w[0] = 4;
        h[0] = 4;
        x[0] = 0;
        y[0] = 0;
    }
    else if (part_type == HLM_IBC_VER_SYM4)
    {
        for (i = 0; i < pu_info->sub_pu_num; i++)
        {
            w[i] = 1;
            h[i] = 4;
            x[i] = i;
            y[i] = 0;
        }
    }
    else if (part_type == HLM_IBC_HOR_SYM4)
    {
        for (i = 0; i < pu_info->sub_pu_num; i++)
        {
            w[i] = 4;
            h[i] = 1;
            x[i] = 0;
            y[i] = i;
        }
    }
    else  // QT
    {
        for (i = 0; i < pu_info->sub_pu_num; i++)
        {
            w[i] = 2;
            h[i] = 2;
            x[i] = (i & 1) * 2;
            y[i] = (i >> 1) * 2;
        }
    }
}

/***************************************************************************************************
* 功  能：获取420/422格式的色度子pb信息，包括pu_num、x/y/w/h
* 参  数：*
*        pu_info                -IO        当前4x4块的子PU信息
*        hor_shift              -I         水平方向的采样移位
*        ver_shift              -I         垂直方向的采样移位
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_GetChromaPbInfo(HLM_IBC_PU_INFO     *pu_info,
                                 HLM_U08              hor_shift,
                                 HLM_U08              ver_shift)
{
    HLM_U08 pu_w = 4 >> hor_shift;  // 色度4x4块的尺寸
    HLM_U08 pu_h = 4 >> ver_shift;
    HLM_U08 *x   = pu_info->sub_x;
    HLM_U08 *y   = pu_info->sub_y;
    HLM_U08 *w   = pu_info->sub_w;
    HLM_U08 *h   = pu_info->sub_h;
    HLM_U08 i    = 0;

    if (pu_info->part_type == HLM_IBC_NO_SPLIT)
    {
        pu_info->sub_pu_num = 1;
        w[0] = pu_w;
        h[0] = pu_h;
        x[0] = 0;
        y[0] = 0;
    }
    else if (pu_info->part_type == HLM_IBC_VER_SYM4)
    {
        pu_info->sub_pu_num = pu_w;
        for (i = 0; i < pu_info->sub_pu_num; i++)
        {
            w[i] = 1;
            h[i] = pu_h;
            x[i] = i;
            y[i] = 0;
        }
    }
    else if (pu_info->part_type == HLM_IBC_HOR_SYM4)
    {
        pu_info->sub_pu_num = pu_h;
        for (i = 0; i < pu_info->sub_pu_num; i++)
        {
            w[i] = pu_w;
            h[i] = 1;
            x[i] = 0;
            y[i] = i;
        }
    }
    else
    {
        pu_info->sub_pu_num = 4;
        for (i = 0; i < pu_info->sub_pu_num; i++)
        {
            w[i] = pu_w >> 1;
            h[i] = pu_h >> 1;
            x[i] = (i & 1) * (pu_w >> 1);
            y[i] = (i >> 1) * (pu_h >> 1);
        }
    }
}

/***************************************************************************************************
* 功  能：获取bvp
* 参  数：*
*        com_cu_info            -I         当前CU信息
*        merge_flag             -I         merge方式
*        zscan_idx              -I         4x4块的索引
*        pu_idx                 -I         子PU的索引
*        bvp                    -O         当前4x4块的bvp或子PU的bvp
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_GetBvp(HLM_CU_INFO         *com_cu_info,
                        HLM_U08              merge_flag,
                        HLM_U08              zscan_idx,
                        HLM_U08              pu_idx,
                        HLM_MV              *bvp)
{
    HLM_S32 pu_x             = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
    HLM_S32 pu_y             = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
    HLM_U08 direct           = tbl_merge_type[merge_flag][zscan_idx];
    HLM_IBC_PU_INFO *pu_info = &com_cu_info->ibc_pu_info[merge_flag][zscan_idx];
    HLM_S08 ref_pu_idx       = 0;

    if (pu_idx == 0)  // 第一个子pu
    {
        if (direct == 1)  // 向左
        {
            *bvp = com_cu_info->inner_bv_left[merge_flag][(pu_y << 2) + pu_info->sub_y[pu_idx]];
        }
        else if (direct == 2)  // 向上
        {
            *bvp = com_cu_info->inner_bv_up[merge_flag][pu_x];
            bvp->mvy = 0;
        }
        else  // 不merge，定长码
        {
            bvp->mvx = 0;
            bvp->mvy = 0;
        }
    }
    else
    {
        ref_pu_idx = pu_idx - 1;  // 默认参考前一个子pu
        if (pu_info->part_type == HLM_IBC_QT && (pu_idx == 2 || (pu_idx == 3 && direct == 2)))
        {
            ref_pu_idx--;
        }
        *bvp = pu_info->sub_bv[ref_pu_idx];
    }
}

/***************************************************************************************************
* 功  能：获取定长码编码bvx的码长
* 参  数：*
*        com_cu_info            -I         当前CU的信息
*        segment_enable_flag    -I         隔断参考是否开启
*        segment_width_in_log2  -I         隔断区域宽度（以像素为单位）的log
* 返回值：bvx的码长
* 备  注：
***************************************************************************************************/
HLM_U08 HLM_COM_GetBvxLen(HLM_CU_INFO *com_cu_info,
                          HLM_U08      segment_enable_flag,
                          HLM_U32      segment_width_in_log2)
{
    HLM_U32 bound_x = com_cu_info->cu_x;  // 以CU为单位

    if (segment_enable_flag)
    {
        bound_x = com_cu_info->cu_x % (1 << (segment_width_in_log2 - HLM_LOG2_WIDTH_SIZE));
    }
#if FIRST_COLUMN_IBC
    if (com_cu_info->first_column_ibc_flag)
    {
        return HLM_IBC_HOR_SEARCH_LOG;
    }
    else if (bound_x >= 8)
#else
    if (bound_x >= 8)
#endif
    {
        return HLM_IBC_HOR_SEARCH_LOG;
    }
    else
    {
        return tbl_bvx_bits[bound_x];
    }
}
#endif

/****************************************************************************************
* 功  能：获取不同块大小下的inter块mvp
* 参  数：*
*        available          -I      当前块相邻块可用性
*        cu_x               -I      当前块相对cu的x起始点
*        cu_y               -I      当前块相对cu的y起始点
*        nbi_mv             -I      当前块相邻位置(经过计算和处理后)mv
*        ref_frame          -I      当前块相邻位置(经过计算和处理后)参考帧id
*        mvp                -O      当前块mvp
* 返回值：无
* 备  注：
****************************************************************************************/
HLM_VOID HLM_COM_GetMvp(HLM_U32   *available,
                        HLM_U32    cu_x,
                        HLM_U32    cu_y,
                        HLM_MV    *nbi_mv,
                        HLM_S32   *ref_frame,
                        HLM_MV    *mvp)
{
    HLM_MV  zero_mv       = { 0 };
    HLM_MV *mv_a          = HLM_NULL;
    HLM_MV *mv_b          = HLM_NULL;
    HLM_MV *mv_c          = HLM_NULL;
    HLM_U32 mvPredType    = 0;  // median
    HLM_S32 ref_frame_cur = 1;
    HLM_S32 rFrameL       = available[0] ? ref_frame[0] : -1;
    HLM_S32 rFrameU       = available[1] ? ref_frame[1] : -1;
    HLM_S32 rFrameUR      = available[2] ? ref_frame[2] : -1;

    if (rFrameL == ref_frame_cur && rFrameU != ref_frame_cur && rFrameUR != ref_frame_cur)
    {
        mvPredType = 1;  // left
    }
    else if (rFrameL != ref_frame_cur && rFrameU == ref_frame_cur && rFrameUR != ref_frame_cur)
    {
        mvPredType = 2;  // up
    }
    else if (rFrameL != ref_frame_cur && rFrameU != ref_frame_cur && rFrameUR == ref_frame_cur)
    {
        mvPredType = 3;  // right-up
    }

    switch (mvPredType)
    {
    case 0:  // median
        if (!(available[1] || available[2]))
        {
            *mvp = available[0] ? nbi_mv[0] : zero_mv;
        }
        else
        {
            mv_a = available[0] ? &nbi_mv[0] : &zero_mv; //left
            mv_b = available[1] ? &nbi_mv[1] : &zero_mv; //up
            mv_c = available[2] ? &nbi_mv[2] : &zero_mv; //upright

            mvp->mvx = HLM_INTER_MEDIAN(mv_a->mvx, mv_b->mvx, mv_c->mvx);
            mvp->mvy = HLM_INTER_MEDIAN(mv_a->mvy, mv_b->mvy, mv_c->mvy);
        }
        break;
    case 1:  // left
        *mvp = available[0] ? nbi_mv[0] : zero_mv;
        break;
    case 2:  // up
        *mvp = available[1] ? nbi_mv[1] : zero_mv;
        break;
    case 3:  // right-up
        *mvp = available[2] ? nbi_mv[2] : zero_mv;
        break;
    default:
        break;
    }
}

/***************************************************************************************************
* 功  能：获取inter块mvp前处理
* 参  数：*
*        com_cu_info        -I      当前CU信息
*        blk8_pu0_mv        -I      8x8划分下第一个PU的mv
*        nbi_info           -I      当前CU相邻块信息
*        is_right_cu        -I      当前cu是否为图像最右侧cu，mv的右上块不存在用左上块代替
*        proc               -I      当前CU处理流程阶段
*        available          -O      当前CU相邻块可用性
*        nbi_mv             -O      当前CU相邻位置(经过计算和处理后)mv
*        ref_frame          -O      当前CU相邻位置(经过计算和处理后)参考帧id
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_InterMvpPre(HLM_CU_INFO         *com_cu_info,
                             HLM_MV              *blk8_pu0_mv,
                             HLM_NEIGHBOR_INFO   *nbi_info,
                             HLM_S32              is_right_cu,
                             HLM_PROC             proc,
                             HLM_U32              available[][3],
                             HLM_MV               nbi_mv[][3],
                             HLM_S32              ref_frame[][3])
{
    HLM_U32 cur_cu_pos = com_cu_info->cu_x << 2;

    // 16x8
    if (com_cu_info->cu_x != 0)
    {
        available[PART_16x8][0] = 1;
        nbi_mv[PART_16x8][0] = nbi_info->inter_mv_left[0];
        if (proc == PROC_PRE)
        {
            ref_frame[PART_16x8][0] = 1;
        }
        else
        {
            ref_frame[PART_16x8][0] = (nbi_info->pred_type_left[0] >= HLM_P_8x8) ? 1 : -1;
        }
    }
    if (com_cu_info->cu_y != 0)
    {
        available[PART_16x8][1] = 1;
        nbi_mv[PART_16x8][1] = nbi_info->inter_mv_up[cur_cu_pos];
        ref_frame[PART_16x8][1] = (nbi_info->pred_type_up[cur_cu_pos] >= HLM_P_8x8) ? 1 : -1;

        if (!is_right_cu)
        {
            available[PART_16x8][2] = 1;
            nbi_mv[PART_16x8][2] = nbi_info->inter_mv_up[cur_cu_pos + 4];
            ref_frame[PART_16x8][2] = (nbi_info->pred_type_up[cur_cu_pos + 4] >= HLM_P_8x8) ? 1 : -1;
        }
        else if (com_cu_info->cu_x != 0)
        {
            available[PART_16x8][2] = 1;
            nbi_mv[PART_16x8][2] = nbi_info->inter_mv_upleft;
            ref_frame[PART_16x8][2] = (nbi_info->pred_type_upleft >= HLM_P_8x8) ? 1 : -1;
        }
    }

    // 8x8，PU0
    if (com_cu_info->cu_x != 0)
    {
        available[PART_8x8_0][0] = 1;
        nbi_mv[PART_8x8_0][0] = nbi_info->inter_mv_left[0];
        if (proc == PROC_PRE)
        {
            ref_frame[PART_8x8_0][0] = 1;
        }
        else
        {
            ref_frame[PART_8x8_0][0] = (nbi_info->pred_type_left[0] >= HLM_P_8x8) ? 1 : -1;
        }
    }
    if (com_cu_info->cu_y != 0)
    {
        available[PART_8x8_0][1] = 1;
        nbi_mv[PART_8x8_0][1] = nbi_info->inter_mv_up[cur_cu_pos];
        ref_frame[PART_8x8_0][1] = (nbi_info->pred_type_up[cur_cu_pos] >= HLM_P_8x8) ? 1 : -1;

        available[PART_8x8_0][2] = 1;
        nbi_mv[PART_8x8_0][2] = nbi_info->inter_mv_up[cur_cu_pos + 2];
        ref_frame[PART_8x8_0][2] = (nbi_info->pred_type_up[cur_cu_pos + 2] >= HLM_P_8x8) ? 1 : -1;
    }

    // 8x8，PU1
    if (proc == PROC_RDO || proc == PROC_BS)
    {
        available[PART_8x8_1][0] = 1;
        nbi_mv[PART_8x8_1][0] = *blk8_pu0_mv;
        ref_frame[PART_8x8_1][0] = 1;
    }
    else if (com_cu_info->cu_x != 0)  // 此时左边无法确定,使用左侧相邻cu块mv
    {
        available[PART_8x8_1][0] = 1;
        nbi_mv[PART_8x8_1][0] = nbi_info->inter_mv_left[0];
        ref_frame[PART_8x8_1][0] = 1;
    }
    if (com_cu_info->cu_y != 0)
    {
        available[PART_8x8_1][1] = 1;
        nbi_mv[PART_8x8_1][1] = nbi_info->inter_mv_up[cur_cu_pos + 2];
        ref_frame[PART_8x8_1][1] = (nbi_info->pred_type_up[cur_cu_pos + 2] >= HLM_P_8x8) ? 1 : -1;

        if (!is_right_cu)
        {
            available[PART_8x8_1][2] = 1;
            nbi_mv[PART_8x8_1][2] = nbi_info->inter_mv_up[cur_cu_pos + 4];
            ref_frame[PART_8x8_1][2] = (nbi_info->pred_type_up[cur_cu_pos + 4] >= HLM_P_8x8) ? 1 : -1;
        }
        else
        {
            available[PART_8x8_1][2] = 1;
            nbi_mv[PART_8x8_1][2] = nbi_info->inter_mv_up[cur_cu_pos + 1];
            ref_frame[PART_8x8_1][2] = (nbi_info->pred_type_up[cur_cu_pos + 1] >= HLM_P_8x8) ? 1 : -1;
        }
    }
}

/***************************************************************************************************
* 功  能：获取skip块mvp
* 参  数：*
*        com_cu_info        -I      当前CU信息
*        nbi_info           -I      当前CU相邻块信息
*        is_right_cu        -I      当前cu是否为图像最右侧cu，mv的右上块不存在用左上块代替
*        mvp                -O      内部子块的mvp
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_InterSkipMvp(HLM_CU_INFO        *com_cu_info,
                              HLM_NEIGHBOR_INFO  *nbi_info,
                              HLM_S32             is_right_cu,
                              HLM_MV             *mvp)
{
    HLM_MV  zero_mv = { 0 };
    HLM_S32 zeroMotionAbove = 0;
    HLM_S32 zeroMotionLeft = 0;
    HLM_U32 available[3] = { 0 };
    HLM_MV  nbi_mv[3] = { 0 };
    HLM_U32 cur_cu_pos = com_cu_info->cu_x << 2;
    HLM_S32 ref_idx[3] = { 0 };

    if (com_cu_info->cu_x != 0)
    {
        available[0] = 1;
        nbi_mv[0] = nbi_info->inter_mv_left[0];
        if (com_cu_info->cu_y == 0)
        {
            nbi_mv[1] = nbi_mv[2] = nbi_mv[0];
        }
        ref_idx[0] = (nbi_info->pred_type_left[0] >= HLM_P_8x8) ? 1 : -1;
    }
    if (com_cu_info->cu_y != 0)
    {
        available[1] = 1;
        nbi_mv[1] = nbi_info->inter_mv_up[cur_cu_pos];
        ref_idx[1] = (nbi_info->pred_type_up[cur_cu_pos] >= HLM_P_8x8) ? 1 : -1;

        if (!is_right_cu)
        {
            available[2] = 1;
            nbi_mv[2] = nbi_info->inter_mv_up[cur_cu_pos + 4];
            ref_idx[2] = (nbi_info->pred_type_up[cur_cu_pos + 4] >= HLM_P_8x8) ? 1 : -1;
        }
        else
        {
            available[2] = 1;
            nbi_mv[2] = nbi_info->inter_mv_upleft;
            ref_idx[2] = (nbi_info->pred_type_upleft >= HLM_P_8x8) ? 1 : -1;
        }
    }

    zeroMotionLeft = !available[0] ? 1 : (ref_idx[0] == 1 && nbi_mv[0].mvx == 0 && nbi_mv[0].mvy == 0) ? 1 : 0;
    zeroMotionAbove = !available[1] ? 1 : (ref_idx[1] == 1 && nbi_mv[1].mvx == 0 && nbi_mv[1].mvy == 0) ? 1 : 0;
    if (zeroMotionAbove || zeroMotionLeft)
    {
        *mvp = zero_mv;
    }
    else
    {
        HLM_COM_GetMvp(available, 0, 0, nbi_mv, ref_idx, mvp);
    }
}

/***************************************************************************************************
* 功  能：生成MPM
* 参  数：*
*        nbi_info               -I         当前cu相邻块信息
*        com_cu_info            -I         当前cu信息
*        zscan_idx              -I         当前cu的位置
*        mpm                    -O         预测的帧内预测模式
*        proc                   -I         处理阶段
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_GetMpm(HLM_NEIGHBOR_INFO   *nbi_info,
                        HLM_CU_INFO         *com_cu_info,
                        HLM_U32              zscan_idx,
                        HLM_U08             *mpm,
                        HLM_PROC             proc)
{
    HLM_U08 pu_x                 = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
    HLM_U08 pu_y                 = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
    HLM_U08 cu_type_left         = 0;
    HLM_U08 cu_type_top          = 0;
    HLM_U08 intra_pred_mode_left = 0;
    HLM_U08 intra_pred_mode_top  = 0;
    HLM_CU_TYPE target_type      = (proc == PROC_PRE) ? HLM_I_4x4 : com_cu_info->cu_type;

    if ((com_cu_info->left_unavail && 0 == pu_x) || (com_cu_info->up_unavail && 0 == pu_y))
    {
        *mpm = HLM_INTRA_4x4_DC;
        return;
    }

    if (0 == pu_x)  //左侧cu_type
    {
        cu_type_left = nbi_info->pred_type_left[pu_y];
    }
    else
    {
        cu_type_left = target_type;
    }
    if (0 == pu_y)  // 上侧cu_type
    {
        cu_type_top = nbi_info->pred_type_up[(com_cu_info->cu_x << 2) + pu_x];
    }
    else
    {
        cu_type_top = target_type;
    }

    if (cu_type_left == target_type)  // 左侧pred_mode
    {
        if (0 == pu_x)
        {
            intra_pred_mode_left = nbi_info->intra_pred_mode_left[pu_y];
        }
        else
        {
            intra_pred_mode_left = com_cu_info->cu_pred_info.pu_info[(pu_y << 2) + pu_x - 1].intra_pred_mode;
        }
    }
    else
    {
        intra_pred_mode_left = HLM_INTRA_4x4_DC;
    }
    if (cu_type_top == target_type)  // 上侧pred_mode
    {
        if (0 == pu_y)
        {
            intra_pred_mode_top = nbi_info->intra_pred_mode_up[(com_cu_info->cu_x << 2) + pu_x];
        }
        else
        {
            intra_pred_mode_top = com_cu_info->cu_pred_info.pu_info[((pu_y - 1) << 2) + pu_x].intra_pred_mode;
        }
    }
    else
    {
        intra_pred_mode_top = HLM_INTRA_4x4_DC;
    }

    *mpm = HLM_MIN(intra_pred_mode_top, intra_pred_mode_left);
}

/***************************************************************************************************
* 功  能：获取intra8x8划分下，色度PB的信息
* 参  数：*
*        image_format           -I         图像格式
*        luma_size              -I         亮度PB的宽高，应为8
*        skip_chroma            -O         两个8x8块是否包含色度PB
*        chroma_size            -O         色度PB的宽高
*        chroma_size_offset     -O         色度PB的宽高与亮度PB的倍数
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_Intra8x8ChromaPb(HLM_U32  image_format,
                                  HLM_U32  luma_size,
                                  HLM_U32  skip_chroma[2],
                                  HLM_U32 *chroma_size,
                                  HLM_U32 *chroma_size_offset)
{
    if (HLM_IMG_YUV_422 == image_format)
    {
        // 色度只有一个PB，宽高为8x8
        *chroma_size = luma_size;
        *chroma_size_offset = 0;
        skip_chroma[0] = 0;  // 第一个8x8有色度PB
        skip_chroma[1] = 1;  // 第二个8x8没有色度PB
    }
    else if (HLM_IMG_YUV_420 == image_format)
    {
        // 色度有两个PB，宽高为4x4
        *chroma_size = luma_size >> 1;
        *chroma_size_offset = 1;
        skip_chroma[0] = 0;
        skip_chroma[1] = 0;
    }
    else if (HLM_IMG_YUV_400 == image_format)
    {
        // 没有色度信息
        *chroma_size = luma_size;
        *chroma_size_offset = 0;
        skip_chroma[0] = 1;
        skip_chroma[1] = 1;
    }
    else
    {
        // 色度信息同亮度
        *chroma_size = luma_size;
        *chroma_size_offset = 0;
        skip_chroma[0] = 0;
        skip_chroma[1] = 0;
    }
}

/***************************************************************************************************
* 功  能：获取16x8的参考像素
* 参  数：*
*        nbi_info                 -I       当前cu相邻块信息
*        com_cu_info              -I       当前cu信息
*        pred_mode                -I       预测模式
*        yuv_idx                  -I       YUV通道
*        ref_pixel                -O       参考像素
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_Intra16x8RefPel(HLM_NEIGHBOR_INFO   *nbi_info,
                                 HLM_CU_INFO         *com_cu_info,
                                 HLM_U08              pred_mode,
                                 HLM_U08              yuv_idx,
                                 HLM_U16             *ref_pixel)
{
    HLM_U08 ref_flag      = HLM_INTRA_ALL_NEIGHBORS;
    HLM_U32 luma_width    = 1 << com_cu_info->cu_width[0];
    HLM_U32 chroma_width  = 1 << com_cu_info->cu_width[1];
    HLM_U32 luma_height   = 1 << com_cu_info->cu_height[0];
    HLM_U32 chroma_height = 1 << com_cu_info->cu_height[1];
    HLM_U08 left_unavail  = com_cu_info->left_unavail;
    HLM_U08 up_unavail    = com_cu_info->up_unavail;

    if (left_unavail)
    {
        ref_flag &= (HLM_INTRA_TOP | HLM_INTRA_TOP_RIGHT);
    }
    if (up_unavail)
    {
        ref_flag &= (HLM_INTRA_LEFT);
    }
    if (yuv_idx == 0)
    {
        if (ref_flag & HLM_INTRA_TOP)
        {
            memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX,
                nbi_info->intra_rec_up_y + (com_cu_info->cu_x << com_cu_info->cu_width[0]), luma_width * sizeof(HLM_U16));
        }
        if (ref_flag & HLM_INTRA_LEFT)
        {
            memcpy(ref_pixel + HLM_INTRA_LEFT_REF_IDX, nbi_info->intra_rec_left_y, luma_height * sizeof(HLM_U16));
        }
        if (ref_flag & HLM_INTRA_TOP_LEFT)
        {
            ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = nbi_info->up_left_y;
        }
    }
    if (yuv_idx == 1)
    {
        if (ref_flag & HLM_INTRA_TOP)
        {
            memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX,
                nbi_info->intra_rec_up_u + (com_cu_info->cu_x << com_cu_info->cu_width[1]), chroma_width * sizeof(HLM_U16));
        }
        if (ref_flag & HLM_INTRA_LEFT)
        {
            memcpy(ref_pixel + HLM_INTRA_LEFT_REF_IDX, nbi_info->intra_rec_left_u, chroma_height * sizeof(HLM_U16));
        }
        if (ref_flag & HLM_INTRA_TOP_LEFT)
        {
            ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = nbi_info->up_left_u;
        }
    }
    if (yuv_idx == 2)
    {
        if (ref_flag & HLM_INTRA_TOP)
        {
            memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX,
                nbi_info->intra_rec_up_v + (com_cu_info->cu_x << com_cu_info->cu_width[2]), chroma_width * sizeof(HLM_U16));
        }
        if (ref_flag & HLM_INTRA_LEFT)
        {
            memcpy(ref_pixel + HLM_INTRA_LEFT_REF_IDX, nbi_info->intra_rec_left_v, chroma_height * sizeof(HLM_U16));
        }
        if (ref_flag & HLM_INTRA_TOP_LEFT)
        {
            ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = nbi_info->up_left_v;
        }
    }
}

/***************************************************************************************************
* 功  能：获取4x4的参考像素
* 参  数：*
*        recon_y_base             -I       重建像素Y分量的地址
*        recon_u_base             -I       重建像素U分量的地址
*        recon_v_base             -I       重建像素V分量的地址
*        stride_y                 -I       Y分量跨度
*        stride_u                 -I       U分量跨度
*        stride_v                 -I       V分量跨度
*        patch_width_in_cu        -I       patch宽度，以CU为单位
*        nbi_info                 -I       当前cu相邻块信息
*        com_cu_info              -I       当前cu信息
*        pred_mode                -I       预测模式
*        yuv_idx                  -I       YUV通道
*        zscan_idx                -I       当前PU的位置
*        pu_log_size              -I       log2的PU尺寸
*        ref_pixel                -O       参考像素
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_Intra4x4RefPel(HLM_U16             *recon_y_base,
                                HLM_U16             *recon_u_base,
                                HLM_U16             *recon_v_base,
                                HLM_U32              stride_y,
                                HLM_U32              stride_u,
                                HLM_U32              stride_v,
                                HLM_U32              patch_width_in_cu,
                                HLM_NEIGHBOR_INFO   *nbi_info,
                                HLM_CU_INFO         *com_cu_info,
                                HLM_U08              pred_mode,
                                HLM_U08              yuv_idx,
                                HLM_U32              zscan_idx,
                                HLM_U32              pu_log_size,
                                HLM_U16             *ref_pixel)
{
    HLM_U08 ref_flag       = HLM_INTRA_ALL_NEIGHBORS;
    HLM_U08 pu_pixel_x     = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx] << pu_log_size;
    HLM_U08 pu_pixel_y     = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx] << pu_log_size;
    HLM_U32 cu_pixel_x     = com_cu_info->cu_x << ((yuv_idx == 0) ? 4 : 4 - com_cu_info->chroma_offset_x);
    HLM_U32 cu_pixel_y     = com_cu_info->cu_y << ((yuv_idx == 0) ? 3 : 3 - com_cu_info->chroma_offset_y);
    HLM_U16 *rec_cu_left_y = nbi_info->intra_rec_left_y;
    HLM_U16 *rec_cu_left_u = nbi_info->intra_rec_left_u;
    HLM_U16 *rec_cu_left_v = nbi_info->intra_rec_left_v;
    HLM_U16 *rec_cu_top_y  = nbi_info->intra_rec_up_y + cu_pixel_x;
    HLM_U16 *rec_cu_top_u  = nbi_info->intra_rec_up_u + cu_pixel_x;
    HLM_U16 *rec_cu_top_v  = nbi_info->intra_rec_up_v + cu_pixel_x;
    HLM_U16 *rec_pu_y      = recon_y_base + (cu_pixel_y + pu_pixel_y) * stride_y + cu_pixel_x + pu_pixel_x;
    HLM_U16 *rec_pu_u      = recon_u_base + (cu_pixel_y + pu_pixel_y) * stride_u + cu_pixel_x + pu_pixel_x;
    HLM_U16 *rec_pu_v      = recon_v_base + (cu_pixel_y + pu_pixel_y) * stride_v + cu_pixel_x + pu_pixel_x;
    HLM_U32 i              = 0;
    HLM_U32 pu_size        = 1 << pu_log_size;
    HLM_U08 left_unavail   = com_cu_info->left_unavail;
    HLM_U08 up_unavail     = com_cu_info->up_unavail;

    if (left_unavail && pu_pixel_x == 0)
    {
        ref_flag &= (HLM_INTRA_TOP | HLM_INTRA_TOP_RIGHT);   // HLM_INTRA_TOP || HLM_INTRA_TOP_RIGHT
    }
    if (up_unavail && pu_pixel_y == 0)
    {
        ref_flag &= (HLM_INTRA_LEFT);  // HLM_INTRA_LEFT
    }
    if (yuv_idx == 0)
    {
        if (ref_flag & HLM_INTRA_TOP)
        {
            if (pu_pixel_y == 0)
            {
                memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX, rec_cu_top_y + pu_pixel_x, pu_size * sizeof(HLM_U16));
            }
            else
            {
                memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX, rec_pu_y - stride_y, pu_size * sizeof(HLM_U16));
            }
        }
        if (ref_flag & HLM_INTRA_TOP_RIGHT)
        {
            if (((zscan_idx == (com_cu_info->intra_8x8_enable_flag ? 1 : 5)) && (com_cu_info->cu_x == (patch_width_in_cu - 1)))
                || (zscan_idx == 3) || (zscan_idx == 7) || (zscan_idx == 11) || (zscan_idx == 13) || (zscan_idx == 15))
            {
                for (i = 0; i < pu_size; i++)
                {
                    ref_pixel[HLM_INTRA_TOP_REF_IDX + pu_size + i] = ref_pixel[HLM_INTRA_TOP_REF_IDX + pu_size - 1];
                }
            }
            else
            {
                if (pu_pixel_y == 0)
                {
                    memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX + pu_size, rec_cu_top_y + pu_pixel_x + pu_size, pu_size * sizeof(HLM_U16));
                }
                else
                {
                    memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX + pu_size, rec_pu_y - stride_y + pu_size, pu_size * sizeof(HLM_U16));
                }
            }
        }
        if (ref_flag & HLM_INTRA_LEFT)
        {
            if (pu_pixel_x == 0)
            {
                memcpy(ref_pixel + HLM_INTRA_LEFT_REF_IDX, rec_cu_left_y + pu_pixel_y, pu_size * sizeof(HLM_U16));
            }
            else
            {
                for (i = 0; i < pu_size; i++)
                {
                    ref_pixel[HLM_INTRA_LEFT_REF_IDX + i] = *(rec_pu_y - 1 + i * stride_y);
                }
            }
        }
        if (ref_flag & HLM_INTRA_TOP_LEFT)
        {
            if (pu_pixel_x == 0 && pu_pixel_y == 0)
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = nbi_info->up_left_y;
            }
            else if (pu_pixel_x == 0)
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = *(rec_cu_left_y - 1 + pu_pixel_y);
            }
            else if (pu_pixel_y == 0)
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = *(rec_cu_top_y - 1 + pu_pixel_x);
            }
            else
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = *(rec_pu_y - 1 - stride_y);
            }
        }
    }
    if (yuv_idx == 1)
    {
        if (ref_flag & HLM_INTRA_TOP)
        {
            if (pu_pixel_y == 0)
            {
                memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX, rec_cu_top_u + pu_pixel_x, pu_size * sizeof(HLM_U16));
            }
            else
            {
                memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX, rec_pu_u - stride_u, pu_size * sizeof(HLM_U16));
            }
        }
        if (ref_flag & HLM_INTRA_TOP_RIGHT)
        {
            if (((zscan_idx == (com_cu_info->intra_8x8_enable_flag ? 1 : 5)) && (com_cu_info->cu_x == (patch_width_in_cu - 1)))
                || (zscan_idx == 3) || (zscan_idx == 7) || (zscan_idx == 11) || (zscan_idx == 13) || (zscan_idx == 15))
            {
                for (i = 0; i < pu_size; i++)
                {
                    ref_pixel[HLM_INTRA_TOP_REF_IDX + pu_size + i] = ref_pixel[HLM_INTRA_TOP_REF_IDX + pu_size - 1];
                }
            }
            else
            {
                if (pu_pixel_y == 0)
                {
                    memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX + pu_size, rec_cu_top_u + pu_pixel_x + pu_size, pu_size * sizeof(HLM_U16));
                }
                else
                {
                    memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX + pu_size, rec_pu_u - stride_u + pu_size, pu_size * sizeof(HLM_U16));
                }
            }
        }
        if (ref_flag & HLM_INTRA_LEFT)
        {
            if (pu_pixel_x == 0)
            {
                memcpy(ref_pixel + HLM_INTRA_LEFT_REF_IDX, rec_cu_left_u + pu_pixel_y, pu_size * sizeof(HLM_U16));
            }
            else
            {
                for (i = 0; i < pu_size; i++)
                {
                    ref_pixel[HLM_INTRA_LEFT_REF_IDX + i] = *(rec_pu_u - 1 + i * stride_u);
                }
            }
        }
        if (ref_flag & HLM_INTRA_TOP_LEFT)
        {
            if (pu_pixel_x == 0 && pu_pixel_y == 0)
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = nbi_info->up_left_u;
            }
            else if (pu_pixel_x == 0)
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = *(rec_cu_left_u - 1 + pu_pixel_y);
            }
            else if (pu_pixel_y == 0)
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = *(rec_cu_top_u - 1 + pu_pixel_x);
            }
            else
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = *(rec_pu_u - 1 - stride_u);
            }
        }
    }
    if (yuv_idx == 2)
    {
        if (ref_flag & HLM_INTRA_TOP)
        {
            if (pu_pixel_y == 0)
            {
                memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX, rec_cu_top_v + pu_pixel_x, pu_size * sizeof(HLM_U16));
            }
            else
            {
                memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX, rec_pu_v - stride_v, pu_size * sizeof(HLM_U16));
            }
        }
        if (ref_flag & HLM_INTRA_TOP_RIGHT)
        {
            if (((zscan_idx == (com_cu_info->intra_8x8_enable_flag ? 1 : 5)) && (com_cu_info->cu_x == (patch_width_in_cu - 1)))
                || (zscan_idx == 3) || (zscan_idx == 7) || (zscan_idx == 11) || (zscan_idx == 13) || (zscan_idx == 15))
            {
                for (i = 0; i < pu_size; i++)
                {
                    ref_pixel[HLM_INTRA_TOP_REF_IDX + pu_size + i] = ref_pixel[HLM_INTRA_TOP_REF_IDX + pu_size - 1];
                }
            }
            else
            {
                if (pu_pixel_y == 0)
                {
                    memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX + pu_size, rec_cu_top_v + pu_pixel_x + pu_size, pu_size * sizeof(HLM_U16));
                }
                else
                {
                    memcpy(ref_pixel + HLM_INTRA_TOP_REF_IDX + pu_size, rec_pu_v - stride_v + pu_size, pu_size * sizeof(HLM_U16));
                }
            }
        }
        if (ref_flag & HLM_INTRA_LEFT)
        {
            if (pu_pixel_x == 0)
            {
                memcpy(ref_pixel + HLM_INTRA_LEFT_REF_IDX, rec_cu_left_v + pu_pixel_y, pu_size * sizeof(HLM_U16));
            }
            else
            {
                for (i = 0; i < pu_size; i++)
                {
                    ref_pixel[HLM_INTRA_LEFT_REF_IDX + i] = *(rec_pu_v - 1 + i * stride_v);
                }
            }
        }
        if (ref_flag & HLM_INTRA_TOP_LEFT)
        {
            if (pu_pixel_x == 0 && pu_pixel_y == 0)
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = nbi_info->up_left_v;
            }
            else if (pu_pixel_x == 0)
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = *(rec_cu_left_v - 1 + pu_pixel_y);
            }
            else if (pu_pixel_y == 0)
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = *(rec_cu_top_v - 1 + pu_pixel_x);
            }
            else
            {
                ref_pixel[HLM_INTRA_TOP_LEFT_REF_IDX] = *(rec_pu_v - 1 - stride_v);
            }
        }
    }
}

// plane模式
HLM_VOID HLM_INTRA_pred_plane(HLM_U16    *ref_pixel,
                              HLM_U16    *pred_pixel,
                              HLM_S32     bitdepth,
                              HLM_U32     pred_stride,
                              HLM_U08     width,
                              HLM_U08     height)
{
    HLM_U16 *src_le      = ref_pixel;
    HLM_U16 *src_up      = ref_pixel + HLM_INTRA_TOP_REF_IDX;
    HLM_S32  w           = width;
    HLM_S32  h           = height;
    HLM_U16 *rsrc        = HLM_NULL;
    HLM_S32  coef_h      = 0;
    HLM_S32  coef_v      = 0;
    HLM_S32  a           = 0;
    HLM_S32  b           = 0;
    HLM_S32  c           = 0;
    HLM_S32  x           = 0;
    HLM_S32  y           = 0;
    HLM_S32  w2          = w >> 1;
    HLM_S32  h2          = h >> 1;
    HLM_S32  ib_mult[5]  = { 13, 17, 5, 11, 23 };
    HLM_S32  ib_shift[5] = { 7, 10, 11, 15, 19 };
    HLM_S32  idx_w       = width == 16 ? 2 : width == 8 ? 1 : 0;   // log2(16)-2 = 2
    HLM_S32  idx_h       = height == 8 ? 1 : 0;   // log2(8)-2 =1;
    HLM_S32  im_h        = ib_mult[idx_w];
    HLM_S32  is_h        = ib_shift[idx_w];
    HLM_S32  im_v        = ib_mult[idx_h];
    HLM_S32  is_v        = ib_shift[idx_h];
    HLM_S32  temp        = 0;
    HLM_S32  temp2       = 0;

    rsrc = src_up + (w2 - 1);
    for (x = 1; x < w2 + 1; x++)
    {
        coef_h += x * (rsrc[x] - rsrc[-x]);
    }
    rsrc = src_le + (h2 - 1);
    for (y = 1; y < h2; y++)
    {
        coef_v += y * (rsrc[y] - rsrc[-y]);
    }
    coef_v += h2 * (rsrc[h2] - src_up[-1]);

    a = (src_le[h - 1] + src_up[w - 1]) << 4;
    b = ((coef_h << 5) * im_h + (1 << (is_h - 1))) >> is_h;
    c = ((coef_v << 5) * im_v + (1 << (is_v - 1))) >> is_v;
    temp = a - (h2 - 1) * c - (w2 - 1) * b + 16;
    for (y = 0; y < h; y++)
    {
        temp2 = temp;
        for (x = 0; x < w; x++)
        {
            pred_pixel[x] = HLM_CLIP((HLM_S32)(temp2 >> 5), 0, ((1 << bitdepth) - 1));
            temp2 += b;
        }
        temp += c;
        pred_pixel += pred_stride;
    }
}

// 正常dc模式
HLM_VOID HLM_INTRA_pred_dc(HLM_U16    *ref_pixel,
                           HLM_U16    *pred_pixel,
                           HLM_S32     bitdepth,
                           HLM_U32     pred_stride,
                           HLM_U08     width,
                           HLM_U08     height)
{
    HLM_S32 i           = 0;
    HLM_S32 j           = 0;
    HLM_U32 dc_value    = 0;
    HLM_U16 *left_pixel = ref_pixel;
    HLM_U16 *top_pixel  = ref_pixel + HLM_INTRA_TOP_REF_IDX;

    for (i = 0; i < height; i++)
    {
        dc_value += left_pixel[i];
    }
    for (i = 0; i < height; i++)
    {
        dc_value += top_pixel[i];
    }
    dc_value = (dc_value + height) / (2 * height);

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            pred_pixel[i] = dc_value;
        }
        pred_pixel += pred_stride;
    }
}

// 左侧dc模式
HLM_VOID HLM_INTRA_pred_dc_left(HLM_U16    *ref_pixel,
                                HLM_U16    *pred_pixel,
                                HLM_S32     bitdepth,
                                HLM_U32     pred_stride,
                                HLM_U08     width,
                                HLM_U08     height)
{
    HLM_S32 i           = 0;
    HLM_S32 j           = 0;
    HLM_U32 dc_value    = 0;
    HLM_U16 *left_pixel = ref_pixel;

    for (i = 0; i < height; i++)
    {
        dc_value += left_pixel[i];
    }
    dc_value = (dc_value + (height >> 1)) / height;

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            pred_pixel[i] = dc_value;
        }
        pred_pixel += pred_stride;
    }
}

// 上侧dc模式
HLM_VOID HLM_INTRA_pred_dc_top(HLM_U16    *ref_pixel,
                               HLM_U16    *pred_pixel,
                               HLM_S32     bitdepth,
                               HLM_U32     pred_stride,
                               HLM_U08     width,
                               HLM_U08     height)
{
    HLM_S32 i          = 0;
    HLM_S32 j          = 0;
    HLM_U32 dc_value   = 0;
    HLM_U16 *top_pixel = ref_pixel + HLM_INTRA_TOP_REF_IDX;

    for (i = 0; i < width; i++)
    {
        dc_value += top_pixel[i];
    }
    dc_value = (dc_value + (width >> 1)) / width;

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            pred_pixel[i] = dc_value;
        }
        pred_pixel += pred_stride;
    }
}

// 默认值dc模式
HLM_VOID HLM_INTRA_pred_dc_default(HLM_U16    *ref_pixel,
                                   HLM_U16    *pred_pixel,
                                   HLM_S32     bitdepth,
                                   HLM_U32     pred_stride,
                                   HLM_U08     width,
                                   HLM_U08     height)
{
    HLM_U32 dc_value = 1 << (bitdepth - 1);
    HLM_S32 i        = 0;
    HLM_S32 j        = 0;

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            pred_pixel[i] = dc_value;
        }
        pred_pixel += pred_stride;
    }
}

// 垂直模式
HLM_VOID HLM_INTRA_pred_vertical(HLM_U16    *ref_pixel,
                                 HLM_U16    *pred_pixel,
                                 HLM_S32     bitdepth,
                                 HLM_U32     pred_stride,
                                 HLM_U08     width,
                                 HLM_U08     height)
{
    HLM_U32 i          = 0;
    HLM_U32 j          = 0;
    HLM_U16 *top_pixel = ref_pixel + HLM_INTRA_TOP_REF_IDX;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            pred_pixel[j] = top_pixel[j];
        }
        pred_pixel += pred_stride;
    }
}

// 水平模式
HLM_VOID HLM_INTRA_pred_horizontal(HLM_U16    *ref_pixel,
                                   HLM_U16    *pred_pixel,
                                   HLM_S32     bitdepth,
                                   HLM_U32     pred_stride,
                                   HLM_U08     width,
                                   HLM_U08     height)
{
    HLM_U32 i           = 0;
    HLM_U32 j           = 0;
    HLM_U16 *left_pixel = ref_pixel;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            pred_pixel[j] = left_pixel[i];
        }
        pred_pixel += pred_stride;
    }
}

// 对角向左模式
HLM_VOID HLM_INTRA_pred_diag_down_left(HLM_U16    *ref_pixel,
                                       HLM_U16    *pred_pixel,
                                       HLM_S32     bitdepth,
                                       HLM_U32     pred_stride,
                                       HLM_U08     width,
                                       HLM_U08     height)
{
    HLM_U16 *top_pixel = ref_pixel + HLM_INTRA_TOP_REF_IDX - 1;
    HLM_U08 y          = 0;
    HLM_U08 x          = 0;
    HLM_S16 idx        = 0;
    HLM_S16 pos        = 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            idx = x + y + 1;
            if (idx <2 * width - 1)
            {
                pred_pixel[y * pred_stride + x] = (top_pixel[idx] + 2 * top_pixel[idx + 1] + top_pixel[idx + 2] + 2) >> 2;
            }
            else
            {
                pred_pixel[y * pred_stride + x] = (top_pixel[idx] + 3 * top_pixel[idx + 1] + 2) >> 2;
            }
        }
    }
}

// 对角向右模式
HLM_VOID HLM_INTRA_pred_diag_down_right(HLM_U16    *ref_pixel,
                                        HLM_U16    *pred_pixel,
                                        HLM_S32     bitdepth,
                                        HLM_U32     pred_stride,
                                        HLM_U08     width,
                                        HLM_U08     height)
{
    HLM_U16 *left_pixel = ref_pixel;
    HLM_U16 *top_pixel  = ref_pixel + HLM_INTRA_TOP_REF_IDX;
    top_pixel           = ref_pixel + HLM_INTRA_TOP_REF_IDX - 1;
    HLM_U08 y           = 0;
    HLM_U08 x           = 0;
    HLM_S16 idx         = 0;
    HLM_S16 pos         = 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            idx = x - y - 1;
            if (idx >= 0)
            {
                pred_pixel[y * pred_stride + x] = (top_pixel[idx] + 2 * top_pixel[idx + 1] + top_pixel[idx + 2] + 2) >> 2;
            }
            else if (idx == -1)
            {
                pred_pixel[y * pred_stride + x] = (2 * top_pixel[0] + left_pixel[0] + top_pixel[1] + 2) >> 2;
            }
            else
            {
                pos = -idx - 2;
                if (pos == 0)
                {
                    pred_pixel[y * pred_stride + x] = (2 * left_pixel[pos] + left_pixel[pos + 1] + top_pixel[0] + 2) >> 2;
                }
                else
                {
                    pred_pixel[y * pred_stride + x] = (left_pixel[pos - 1] + 2 * left_pixel[pos] + left_pixel[pos + 1] + 2) >> 2;
                }
            }
        }
    }
}

// 垂直偏右模式
HLM_VOID HLM_INTRA_pred_vertical_right(HLM_U16    *ref_pixel,
                                       HLM_U16    *pred_pixel,
                                       HLM_S32     bitdepth,
                                       HLM_U32     pred_stride,
                                       HLM_U08     width,
                                       HLM_U08     height)
{
    HLM_U16 *left_pixel = ref_pixel;
    HLM_U16 *top_pixel  = ref_pixel + HLM_INTRA_TOP_REF_IDX - 1;
    HLM_U08 y           = 0;
    HLM_U08 x           = 0;
    HLM_S16 idx         = 0;
    HLM_U16 w           = 0;
    HLM_U16 i           = 0;
    HLM_S16 weight      = 0;
    HLM_S16 pos         = 0;
    
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            idx = 2 * x - y ;
            if (idx >= 0 && (idx %2 ==0))
            {
                pred_pixel[y * pred_stride + x] = (top_pixel[x - (y >> 1)] + top_pixel[x - (y >> 1) + 1] + 1) >> 1;
            }
            else if (idx >= 0)
            {
                pred_pixel[y * pred_stride + x] = (top_pixel[x - (y >> 1) - 1] + 2 * top_pixel[x - (y >> 1)] + top_pixel[x - (y >> 1) + 1] + 2) >> 2;
            }
            else
            {
                if (idx == -1) 
                {
                    pred_pixel[y * pred_stride + x] = (2 * top_pixel[0] + left_pixel[0] + top_pixel[1] + 2) >> 2;
                }
                else if (idx == -2)
                {
                    pred_pixel[y * pred_stride + x] = (top_pixel[0] + 2 * left_pixel[0] + left_pixel[1] + 2) >> 2;
                }
                else if (width == 4)
                {
                    pred_pixel[y * pred_stride + x] = (left_pixel[y - 1] + 2 * left_pixel[y - 2] + left_pixel[y - 3] + 2) >> 2;
                }
                else 
                {
                    pred_pixel[y * pred_stride + x] = (left_pixel[y - 2 * x - 1] + 2 * left_pixel[y - 2 * x - 2] + left_pixel[y - 2 * x - 3] + 2) >> 2;
                }
            }
        }
    }
}

// 水平偏下模式
HLM_VOID HLM_INTRA_pred_horizontal_down(HLM_U16    *ref_pixel,
                                        HLM_U16    *pred_pixel,
                                        HLM_S32     bitdepth,
                                        HLM_U32     pred_stride,
                                        HLM_U08     width,
                                        HLM_U08     height)
{
    HLM_U16 *left_pixel = ref_pixel;
    HLM_U16 *top_pixel  = ref_pixel + HLM_INTRA_TOP_REF_IDX - 1;
    HLM_U08 y           = 0;
    HLM_U08 x           = 0;
    HLM_S16 idx         = 0;
    HLM_U16 w           = 0;
    HLM_U16 i           = 0;
    HLM_S16 pos         = 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            idx = 2 * y - x;
            if (idx == 0)
            {
                pred_pixel[y * pred_stride + x] = (left_pixel[0] + top_pixel[0] + 1) >> 1;
            }
            else if (idx > 0 && (idx %2==0))
            {
                pred_pixel[y * pred_stride + x] = (left_pixel[y - (x >> 1)-1] + left_pixel[y - (x >> 1)] + 1) >> 1;
            }
            else if (idx > 0)
            {
                if (idx == 1)
                    pred_pixel[y * pred_stride + x] = (top_pixel[0] + 2 * left_pixel[y - (x >> 1) - 1] + left_pixel[y - (x >> 1)] + 2) >> 2;
                else
                    pred_pixel[y * pred_stride + x] = (left_pixel[y - (x >> 1) - 2] + 2 * left_pixel[y - (x >> 1) -1] + left_pixel[y - (x >> 1)] + 2) >> 2;
            }
            else
            {
                if (idx == -1)
                {
                    pred_pixel[y * pred_stride + x] = (2 * top_pixel[0] + left_pixel[0] + top_pixel[1] + 2) >> 2;
                }
                else if (width == 4)
                {
                    pred_pixel[y * pred_stride + x] = (top_pixel[x] + 2 * top_pixel[x - 1] + top_pixel[x - 2] + 2) >> 2;
                }
                else
                {
                    pred_pixel[y * pred_stride + x] = (top_pixel[x - 2 * y] + 2 * top_pixel[x - 2 * y - 1] + top_pixel[x - 2 * y - 2] + 2) >> 2;
                }
            }
          
        }
    }
}

// 垂直偏左模式
HLM_VOID HLM_INTRA_pred_vertical_left(HLM_U16    *ref_pixel,
                                      HLM_U16    *pred_pixel,
                                      HLM_S32     bitdepth,
                                      HLM_U32     pred_stride,
                                      HLM_U08     width,
                                      HLM_U08     height)
{
    HLM_U16 *left_pixel = ref_pixel;
    HLM_U16 *top_pixel  = ref_pixel + HLM_INTRA_TOP_REF_IDX;
    HLM_U08 y           = 0;
    HLM_U08 x           = 0;
    HLM_S16 idx         = 0;
    HLM_U16 w           = 0;
    HLM_U16 i           = 0;
    HLM_S16 pos         = 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            idx = y + 2 * x;
            if (idx & 1)
            {
                pos = (idx + 1) >> 1;
                if (pos >= 2 * width - 1)
                {
                    pos = 2 * width - 1;
                    pred_pixel[y * pred_stride + x] = (top_pixel[pos - 1] + top_pixel[pos] * 2 + top_pixel[pos] + 2) >> 2;
                }
                else
                {
                    pred_pixel[y * pred_stride + x] = (top_pixel[pos - 1] + top_pixel[pos] * 2 + top_pixel[pos + 1] + 2) >> 2;
                }
            }
            else
            {
                pos = idx >> 1;
                if (pos >= 2 * width - 1)
                {
                    pos = 2 * width - 1;
                    pred_pixel[y * pred_stride + x] = (top_pixel[pos] + top_pixel[pos] + 1) >> 1;
                }
                else
                {
                    pred_pixel[y * pred_stride + x] = (top_pixel[pos] + top_pixel[pos + 1] + 1) >> 1;
                }
            }
        }
    }
}

// 水平偏上模式
HLM_VOID HLM_INTRA_pred_horizontal_up(HLM_U16    *ref_pixel,
                                      HLM_U16    *pred_pixel,
                                      HLM_S32     bitdepth,
                                      HLM_U32     pred_stride,
                                      HLM_U08     width,
                                      HLM_U08     height)
{
    HLM_U16 *left_pixel = ref_pixel;
    HLM_U08 y           = 0;
    HLM_U08 x           = 0;
    HLM_S16 idx         = 0;
    HLM_U16 w           = 0;
    HLM_U16 i           = 0;
    HLM_S16 pos         = 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            idx = y * 2 + x;
            if (idx >= 2 * (height - 1))
            {
                pred_pixel[y * pred_stride + x] = ref_pixel[height - 1];
            }
            else if (idx & 1)
            {
                pos = ((idx + 1) >> 1) >= height - 1 ? height - 1 : ((idx + 1) >> 1);
                pred_pixel[y * pred_stride + x] = (left_pixel[pos - 1] + left_pixel[pos] * 2 + left_pixel[(pos + 1) >= height ? height - 1 : pos + 1] + 2) >> 2;
            }
            else
            {
                pos = idx >> 1;
                pred_pixel[y * pred_stride + x] = (left_pixel[pos] + left_pixel[pos + 1] + 1) >> 1;
            }
        }
    }
}

const HLM_INTRA_PRED_FUNC HLM_INTRA_PRED_16x8[HLM_INTRA_MODE_NUM_16x8] =
{
    HLM_INTRA_pred_vertical,
    HLM_INTRA_pred_horizontal,
    HLM_INTRA_pred_dc,
    HLM_INTRA_pred_plane,
    HLM_INTRA_pred_dc_left,
    HLM_INTRA_pred_dc_top,
    HLM_INTRA_pred_dc_default
};

const HLM_INTRA_PRED_FUNC HLM_INTRA_PRED_4x4[HLM_INTRA_MODE_NUM_4x4] =
{
    HLM_INTRA_pred_vertical,
    HLM_INTRA_pred_horizontal,
    HLM_INTRA_pred_dc,
    HLM_INTRA_pred_diag_down_left,
    HLM_INTRA_pred_diag_down_right,
    HLM_INTRA_pred_vertical_right,
    HLM_INTRA_pred_horizontal_down,
    HLM_INTRA_pred_vertical_left,
    HLM_INTRA_pred_horizontal_up,
    HLM_INTRA_pred_dc_left,
    HLM_INTRA_pred_dc_top,
    HLM_INTRA_pred_dc_default
};
