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
#ifndef _HIM_COM_PRED_H_
#define _HIM_COM_PRED_H_

#include "hlm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HLM_INTER_MEDIAN(a, b, c)    ((a >= b) \
                                      ? ((c >= a) ? a : ((c <= b) ? b : c)) \
                                      : ((c <= a ) ? a : ((c >= b) ? b : c)));

typedef HLM_VOID(*HLM_INTRA_PRED_FUNC)  (HLM_U16    *ref_pixel,
                                         HLM_U16    *pred_pixel,
                                         HLM_S32     bitdepth,
                                         HLM_U32     pred_stride,
                                         HLM_U08     width,
                                         HLM_U08     height);

typedef enum _HLM_INTRA_NEIGHBORS
{
    HLM_INTRA_LEFT                      = 0x01,
    HLM_INTRA_TOP                       = 0x02,
    HLM_INTRA_TOP_RIGHT                 = 0x04,
    HLM_INTRA_TOP_LEFT                  = 0x08,
    HLM_INTRA_ALL_NEIGHBORS             = 0x0f
}HLM_INTRA_NEIGHBORS;

// 16x8帧内预测模式
typedef enum _HLM_INTRA_MODE_16x8
{
    HLM_INTRA_16x8_V                    = 0,
    HLM_INTRA_16x8_H                    = 1,
    HLM_INTRA_16x8_DC                   = 2,
    HLM_INTRA_16x8_P                    = 3,
    HLM_INTRA_PRED_MODE_NUM_16x8        = 4,  //不包含扩展类型一共4个
    HLM_INTRA_16x8_DC_LEFT              = 4,
    HLM_INTRA_16x8_DC_TOP               = 5,
    HLM_INTRA_16x8_DC_128               = 6,
    HLM_INTRA_MODE_NUM_16x8             = 7,  //包含扩展类型一共7个
}HLM_INTRA_MODE_16x8;

// 4x4和8x8帧内预测模式
typedef enum _HLM_INTRA_MODE_4x4
{
    HLM_INTRA_4x4_V                     = 0,
    HLM_INTRA_4x4_H                     = 1,
    HLM_INTRA_4x4_DC                    = 2,
    HLM_INTRA_4x4_DDL                   = 3,
    HLM_INTRA_4x4_DDR                   = 4,
    HLM_INTRA_4x4_VR                    = 5,
    HLM_INTRA_4x4_HD                    = 6,
    HLM_INTRA_4x4_VL                    = 7,
    HLM_INTRA_4x4_HU                    = 8,
    HLM_INTRA_PRED_MODE_NUM_4x4         = 9,   //不包含扩展类型一共9个
    HLM_INTRA_4x4_DC_LEFT               = 9,
    HLM_INTRA_4x4_DC_TOP                = 10,
    HLM_INTRA_4x4_DC_128                = 11,
    HLM_INTRA_MODE_NUM_4x4              = 12,  //包含扩展类型一共12个
}HLM_INTRA_MODE_4x4;

// 参考像素存储位置索引
typedef enum _HLM_INTRA_REF_IDX
{
    HLM_INTRA_LEFT_REF_IDX              = 0,
    HLM_INTRA_TOP_LEFT_REF_IDX          = 8,
    HLM_INTRA_TOP_REF_IDX               = 9,
    HLM_INTRA_TOP_RIGHT_REF_IDX         = 17,
}HLM_INTRA_REF_IDX;

typedef enum _HLM_INTER_PART_TYPE
{
    PART_16x8 = 0,
    PART_8x8_0,
    PART_8x8_1,
    PART_NUM,
} HLM_INTER_PART_TYPE;

#if MIX_IBC || BVY_ZERO
// 混合ibc的划分类型
typedef enum _HLM_IBC_PART_TYPE
{
    HLM_IBC_NO_SPLIT,                                    // 不划分
    HLM_IBC_VER_SYM4,                                    // 垂直四等分
    HLM_IBC_HOR_SYM4,                                    // 水平四等分
    HLM_IBC_QT,                                          // 四叉树划分
    HLM_IBC_PART_NUM,
}HLM_IBC_PART_TYPE;

// IBC子PU的信息，以4x4块为单位
typedef struct _HLM_IBC_PU_INFO
{
    HLM_IBC_PART_TYPE   part_type;                       // 当前4x4块的划分类型
    HLM_U08             sub_pu_num;
    HLM_U08             sub_x[4];                        // 相对于4x4块左上角点的坐标，取值0~3
    HLM_U08             sub_y[4];
    HLM_U08             sub_w[4];                        // 子PU的宽高，取值1~4
    HLM_U08             sub_h[4];
    HLM_MV              sub_bv[4];                       // 子pu的bv信息
    HLM_MV              sub_bvd[4];
} HLM_IBC_PU_INFO;
#endif

typedef struct _HLM_PU_INFO
{
    HLM_U32             pu_x;                            // 当前pu x轴坐标，像素级，相对于cu左上角
    HLM_U32             pu_y;                            // 当前pu y轴坐标
    HLM_U08             intra_pred_mode;                 // intra预测模式 ，yuv444下亮色度模式一致
    HLM_MV              inter_mv;                        // inter mv,yuv444下亮色度一致
    HLM_MV              inter_mvp;                       // inter mvp,为相邻块经过RDO之后的mvp
#if LINE_BY_LINE
    HLM_U08             line_by_line_mode[4];            // 逐行4x1预测模式
    HLM_U08             row_by_row_mode[4];              // 逐列1x4预测模式
#endif
} HLM_PU_INFO;

typedef struct _HLM_CU_PRED_INFO
{
    HLM_CU_TYPE         part_type;                       // 划分方式
    HLM_PU_INFO         pu_info[HLM_TU_4x4_NUMS];        // pu信息，按照raster存储
    HLM_U16            *skip_pred[3];                    // skip模式下预测值(编码端指针可移动到公共区域）
    HLM_U16            *inter_pred[3];                   // inter模式下预测值(编码端指针可移动到公共区域）
    HLM_U16            *pred[3];                         // 预测值
    HLM_U16            *rec[3];                          // 重构值
    HLM_COEFF          *res[3];                          // 预测残差
    HLM_COEFF          *coeff[3];                        // 残差系数
    HLM_MV              skip_mvp;                        // skip模式的mvp
    HLM_MV              inter_mvp[3];                    // inter模式三种子块的mvp
} HLM_CU_PRED_INFO;

typedef struct _HLM_CU_INFO
{
    HLM_U32             cu_x;                            // 当前mb在patch内的 x轴坐标，单位是mb
    HLM_U32             cu_y;                            // 当前mb在patch内的 y轴坐标
    HLM_CU_TYPE         cu_type;                         // 当前cu的类型
    HLM_U08             cbf[3];                          // 非零系数标志
    HLM_U08             qp[3];                           // 亮色度的qp值
    HLM_U08             last_code_qp;                    // 上一次编码的qp
    HLM_U08             cu_width[3];                     // 亮色度宽
    HLM_U08             cu_height[3];                    // 亮色度高
    HLM_U08             chroma_offset_x;
    HLM_U08             chroma_offset_y;
    HLM_CU_PRED_INFO    cu_pred_info;
    HLM_U08             coeffs_num[3][2][4];             // 4x4块非零系数个数
#if LINE_BY_LINE
    HLM_CU_TYPE         line_row_pread_mode;                         // 当前cu的类型
#if LINE_BY_LINE_4x1_RESI
    HLM_U32             cu_line_resi[3];
#endif
#endif
    HLM_U08             ts_flag;
    HLM_U08             merge_flag;
    HLM_U08             up_unavail;
    HLM_U08             left_unavail;
    HLM_U16             intra_8x8_enable_flag;
    HLM_U16             cu_delta_qp_enable_flag;
    HLM_U16             search_area[3][HLM_IBC_SEARCH_AREA_WIDTH * HLM_IBC_SEARCH_AREA_HEIGHT];
#if MIX_IBC || BVY_ZERO
    HLM_U08             first_column_ibc_flag;  // 是否为首列ibc
    HLM_U08             bvy_zero_flag;
    HLM_U08             mix_ibc_flag;
    HLM_MV              inner_bv_left[HLM_BV_MERGE_NUM][8];  // CU内部每个4x4块的相邻bv，按1x1大小保存
    HLM_MV              inner_bv_up[HLM_BV_MERGE_NUM][16];
    HLM_IBC_PU_INFO     ibc_pu_info[HLM_BV_MERGE_NUM][8];    // 每个4x4块的PU信息，按照Z字存储
#endif
} HLM_CU_INFO;

typedef struct _HLM_NEIGHBOR_INFO
{
    HLM_U16       intra_rec_left_y[16];             // 左侧cu的一列重构像素值
    HLM_U16       intra_rec_left_u[16];
    HLM_U16       intra_rec_left_v[16];
    HLM_U16      *intra_rec_up_y;                   // 上方cu的一行重构像素值
    HLM_U16      *intra_rec_up_u;
    HLM_U16      *intra_rec_up_v;
    HLM_U16       up_left_y;                        // 左上方像素点
    HLM_U16       up_left_u;
    HLM_U16       up_left_v;

    HLM_CU_TYPE   pred_type_upleft;                 // 编码类型信息，按照4x4大小保存
    HLM_CU_TYPE   pred_type_left[4];
    HLM_CU_TYPE  *pred_type_up;
    HLM_U08       intra_pred_mode_left[4];          // 帧内预测模式值，按4x4大小保存
    HLM_U08      *intra_pred_mode_up;

    HLM_MV        inter_mv_upleft;                  // inter最终mv，按4x4大小保存
    HLM_MV        inter_mv_left[4];
    HLM_MV       *inter_mv_up;
} HLM_NEIGHBOR_INFO;

/***************************************************************************************************
* 常量表定义
***************************************************************************************************/
static const HLM_U08 HLM_INTRA_ZSCAN_TO_PELX[16] =
{
    0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3
};

static const HLM_U08 HLM_INTRA_ZSCAN_TO_PELY[16] =
{
    0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3
};

static const HLM_U08 HLM_INTRA_RASTER_TO_ZSCAN[16] =
{
    0, 1, 4, 5,
    2, 3, 6, 7,
    8, 9, 12,13,
    10,11,14,15
};

static const HLM_U08 HLM_INTRA_MODE_AVAIL_16x8[HLM_INTRA_MODE_NUM_16x8] =
{
    HLM_INTRA_TOP,
    HLM_INTRA_LEFT,
    HLM_INTRA_TOP | HLM_INTRA_LEFT,
    HLM_INTRA_LEFT | HLM_INTRA_TOP | HLM_INTRA_TOP_LEFT,
    HLM_INTRA_LEFT,
    HLM_INTRA_TOP,
    0
};

static const HLM_U08 HLM_INTRA_MODE_AVAIL_4x4[HLM_INTRA_MODE_NUM_4x4] =
{
    HLM_INTRA_TOP,
    HLM_INTRA_LEFT,
    HLM_INTRA_LEFT | HLM_INTRA_TOP,
    HLM_INTRA_TOP | HLM_INTRA_TOP_RIGHT,
    HLM_INTRA_LEFT | HLM_INTRA_TOP | HLM_INTRA_TOP_LEFT,
    HLM_INTRA_LEFT | HLM_INTRA_TOP | HLM_INTRA_TOP_LEFT,
    HLM_INTRA_LEFT | HLM_INTRA_TOP | HLM_INTRA_TOP_LEFT,
    HLM_INTRA_TOP | HLM_INTRA_TOP_RIGHT,
    HLM_INTRA_LEFT,
    HLM_INTRA_LEFT,
    HLM_INTRA_TOP,
    0
};

#if MIX_IBC
static const HLM_U08 tbl_bvx_bits[8] = { 3, 4, 5, 5, 6, 6, 7, 7 };  // 索引为cu_x，值为bv_x的比特数

// zscan扫描顺序下BV Merge方式，0:不merge，1:向左，2:向上
static const HLM_U08 tbl_merge_type[HLM_BV_MERGE_NUM][8] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0 },  // 不merge
    { 0, 1, 2, 1, 1, 1, 1, 1 },  // 水平
    { 0, 0, 2, 2, 0, 0, 2, 2 },  // 垂直
    { 0, 1, 2, 2, 1, 1, 2, 2 }   // 混合
};
#endif

extern const HLM_INTRA_PRED_FUNC HLM_INTRA_PRED_16x8[HLM_INTRA_MODE_NUM_16x8];
extern const HLM_INTRA_PRED_FUNC HLM_INTRA_PRED_4x4[HLM_INTRA_MODE_NUM_4x4];

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
                                HLM_S32     x_in_area);

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
                                   HLM_S32     cu_y);

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
                                HLM_S32     segment_width);

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
                               HLM_MV              *bv);

/***************************************************************************************************
* 功  能：根据4x4块的划分类型获取子pu信息，包括pu_num、x/y/w/h
* 参  数：*
*        part_type              -I         当前4x4块的划分类型
*        pu_info                -O         当前4x4块的子PU信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_GetIbcPuInfo(HLM_IBC_PART_TYPE    part_type,
                              HLM_IBC_PU_INFO     *pu_info);

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
                                 HLM_U08              ver_shift);

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
                        HLM_MV              *bvp);

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
                          HLM_U32      segment_width_in_log2);
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
                        HLM_MV    *mvp);

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
                             HLM_S32              ref_frame[][3]);

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
                              HLM_MV             *mvp);

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
                        HLM_PROC             proc);

/***************************************************************************************************
* 功  能：获取intra8x8划分下，色度PB的信息
* 参  数：*
*        image_format           -I         图像格式
*        luma_size              -I         亮度PB的宽高，默认为8
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
                                  HLM_U32 *chroma_size_offset);

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
                                 HLM_U16             *ref_pixel);

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
                                HLM_U16             *ref_pixel);

#ifdef __cplusplus
}
#endif

#endif //_HIM_COM_PRED_H_
