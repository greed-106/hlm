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
#ifndef _HLM_COMMON_H_
#define _HLM_COMMON_H_

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "hlm_defs.h"
#include "hlm_mem.h"
#include "hlm_nbi.h"
#include "hlm_com_iqt.h"
#include "hlm_com_pred.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WRITE_PARAMETERS                        (0)       // output data for "tool/BitstreamAnalyzer"
#define COLOR_TYPE                              (1)       // 1: BT.601   2: BT.709   3: BT.2020

#define MD5FUNC(f, w, x, y, z, msg1, s,msg2 )   (w += f(x, y, z) + msg1 + msg2, \
                                                 w = w<<s | w>>(32-s), w += x)
#define FF(x, y, z)                             (z ^ (x & (y ^ z)))
#define GG(x, y, z)                             (y ^ (z & (x ^ y)))
#define HH(x, y, z)                             (x ^ y ^ z)
#define II(x, y, z)                             (y ^ (x | ~z))

#define PATCH_HOR_OVERLAP(x, w, xx, ww)         ((x + w <= xx) || (xx + ww <= x))  // 无水平重叠
#define PATCH_VER_OVERLAP(y, h, yy, hh)         ((y + h <= yy) || (yy + hh <= y))  // 无垂直重叠

#define HLM_MAX_QP(bitdepth)                    (51 + 6 * (bitdepth - 8))          // 最大QP

typedef struct _HLM_MD5
{
    HLM_U32  h[4];                                  // hash state ABCD
    HLM_U08  msg[64];                               // input buffer (chunk message)
    HLM_U32  bits[2];                               // number of bits, modulo 2^64 (lsb first)
} HLM_MD5;

// patch参数
typedef struct _HLM_PATCH_PARAM
{
    HLM_U32  patch_x;                               // patch左上角像素的横坐标
    HLM_U32  patch_y;                               // patch左上角像素的纵坐标
    HLM_U32  patch_width[2];                        // patch的原始宽度
    HLM_U32  patch_height[2];                       // patch的原始高度
    HLM_U32  patch_coded_width[2];                  // patch的padding后的宽度，16的倍数
    HLM_U32  patch_coded_height[2];                 // patch的padding后的高度，8的倍数
} HLM_PATCH_PARAM;

// patch划分信息
typedef struct _HLM_PATCH_INFO
{
    HLM_S32  patch_num;                             // 划分的patch个数
    HLM_PATCH_PARAM patch_param[HLM_MAX_PATCH_NUM]; // 每个patch的参数
} HLM_PATCH_INFO;

// 序列头参数
typedef struct _HLM_PARAM_SPS
{
    HLM_S32  pic_height_in_map_units_minus1;
    HLM_S32  pic_width_in_cus_minus1;
    HLM_S32  bitdepth;
    HLM_S32  bit_depth_luma_minus8;
    HLM_S32  bit_depth_chroma_minus8;
    HLM_S32  format;
    HLM_S32  frame_cropping_flag;
    HLM_S32  frame_crop_left_offset;
    HLM_S32  frame_crop_right_offset;
    HLM_S32  frame_crop_top_offset;
    HLM_S32  frame_crop_bottom_offset;
    HLM_S32  profile;
    HLM_S32  intra_8x8_enable_flag;
    HLM_S32  cu_delta_qp_enable_flag;
    HLM_S32  mv_limit_enable_flag;
    HLM_S32  bpp_i;
    HLM_S32  bpp_p;
    HLM_S32  i_frame_enable_ibc;
    HLM_S32  p_frame_enable_ibc;
    HLM_S32  mv_ref_cross_patch;
    HLM_S32  mv_search_width;
    HLM_S32  mv_search_height;
    HLM_PATCH_INFO patch_info;
} HLM_PARAM_SPS;

// 图像头参数
typedef struct _HLM_PARAM_PPS
{
    HLM_S32  pic_type;
    HLM_S32  poc;
    HLM_S32  pic_luma_qp;
    HLM_S32  pic_chroma_delta_qp;
} HLM_PARAM_PPS;

//条带头参数
typedef struct _HLM_PATCH_HEADER
{
    HLM_S32  patch_idx;
    HLM_S32  patch_bpp;
    HLM_S32  segment_enable_flag;
    HLM_S32  segment_width_in_log2;
    HLM_S32  segment_height_in_log2;
    HLM_S32  patch_extra_params_present_flag;
    HLM_PARAM_SPS  sps;  // patch级重传的序列头语法
    HLM_PARAM_PPS  pps;  // patch级重传的图像头语法
} HLM_PATCH_HEADER;

// SEI控制参数
typedef struct _HLM_SEI_INFO
{
    SEI_PAYLOAD_TYPE  payload_type;
    HLM_U32  payload_size;  // 以字节为单位
    HLM_U08 *payload_data;
} HLM_PARAM_SEI;

/***************************************************************************************************
* 常量表定义
***************************************************************************************************/
static const HLM_U08 HLM_B4_X_8[8]   = { 0,1,2,3,0,1,2,3 };  // 4x4块坐标转换
static const HLM_U08 HLM_B4_Y_8[8]   = { 0,0,0,0,1,1,1,1 };
static const HLM_U08 HLM_B4_X_4[4]   = { 0,1,0,1 };
static const HLM_U08 HLM_B4_Y_4[4]   = { 0,0,1,1 };
static const HLM_U08 HLM_B4_X_2[2]   = { 0,1 };
static const HLM_U08 HLM_B4_Y_2[2]   = { 0,0 };
#if !MIX_IBC
static const HLM_U08 tbl_bvx_bits[8] = { 3, 4, 5, 5, 6, 6, 7, 7 };  // 索引为cu_x，值为bv_x的比特数

// zscan扫描顺序下BV Merge方式，0:不merge，1:向左，2:向上
static const HLM_U08 tbl_merge_type[HLM_BV_MERGE_NUM][8] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0 },        // HLM_BV_NO_MERGE
    { 0, 1, 2, 1, 1, 1, 1, 1 },        // HLM_BV_MERGE_LEFT
    { 0, 0, 2, 2, 0, 0, 2, 2 },        // HLM_BV_MERGE_UP
    { 0, 1, 2, 2, 1, 1, 2, 2 }         // HLM_BV_MERGE_MIX
};
#endif

/***************************************************************************************************
* 功  能：计算对数
* 参  数：
*        v                        -I       待求对数的值
* 返回值：计算对数后向下取整的结果
* 备  注：
***************************************************************************************************/
HLM_S32 HLM_COM_Log2(HLM_U32 v);

/***************************************************************************************************
* 功  能：计算MD5
* 参  数：*
*        yuv_buf                  -I       图像数据
*        digest                   -O       MD5值
*        luma_width               -I       图像亮度宽度
*        luma_height              -I       图像亮度高度
*        chroma_width             -I       图像色度宽度
*        chroma_height            -I       图像色度高度
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_GetMd5(HLM_U16 *y_buf,
                        HLM_U16 *cb_buf,
                        HLM_U16 *cr_buf,
                        HLM_U08  digest[16],
                        HLM_U32  luma_width,
                        HLM_U32  luma_height,
                        HLM_U32  chroma_width,
                        HLM_U32  chroma_height);

/***************************************************************************************************
* 功  能：CSC 颜色空间转换（RGB -> YCbCr）
* 参  数：*
*        src0                     -IO      第一分量数据
*        src1                     -IO      第二分量数据
*        src2                     -IO      第三分量数据
*        width                    -I       宽度
*        height                   -I       高度
*        bitdepth                 -I       比特深度
* 返回值：无
* 备  注：
*      Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
*      Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + 128
*      Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + 128
***************************************************************************************************/
HLM_VOID HLM_COM_Rgb_To_YCbCr(HLM_U16 *src0,
                              HLM_U16 *src1,
                              HLM_U16 *src2,
                              HLM_S32  width,
                              HLM_S32  height,
                              HLM_U08  bitdepth);

/***************************************************************************************************
* 功  能：CSC 颜色空间转换（YCbCr -> RGB）
* 参  数：*
*        src0                     -IO      第一分量数据
*        src1                     -IO      第二分量数据
*        src2                     -IO      第三分量数据
*        width                    -I       宽度
*        height                   -I       高度
*        bitdepth                 -I       比特深度
* 返回值：无
* 备  注：
*      R  =  Y + 1.402 * (Cr - 128)
*      G  =  Y - 0.344 * (Cb - 128) - 0.714 *( Cr - 128)
*      B  =  Y + 1.772 * (Cb - 128)
***************************************************************************************************/
HLM_VOID HLM_COM_YCbCr_To_Rgb(HLM_U16 *src0,
                              HLM_U16 *src1,
                              HLM_U16 *src2,
                              HLM_S32  width,
                              HLM_S32  height,
                              HLM_U08  bitdepth);

/***************************************************************************************************
* 功  能：获取每个4x4块对应的系数位置
* 参  数：*
*        num_4x4                  -I       4x4块个数
*        idx_4                    -I       4x4块顺序
*        pos_x                    -IO      横坐标
*        pos_y                    -IO      纵坐标
*        com_cu_info              -IO      当前CU的信息
*        yuv_idx                  -I       分量索引
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_4x4_Block_To_Coeff_Pos(HLM_U08          num_4x4,
                                        HLM_S32          idx_4,
                                        HLM_U08         *pos_x,
                                        HLM_U08         *pos_y,
                                        HLM_CU_INFO     *com_cu_info,
                                        HLM_U08          yuv_idx);

/***************************************************************************************************
* 功  能：获取不同格式下、不同分量的水平方向和垂直方向的采样率
* 参  数：*
*        image_format             -I       图像格式
*        hor_shift                -O       水平采样率
*        ver_shift                -O       垂直采样率
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_GetFormatShift(HLM_U32  image_format,
                                HLM_S08  hor_shift[3],
                                HLM_S08  ver_shift[3]);

/***************************************************************************************************
* 功  能：获取src上的(blk_x,blk_y)位置处的宽高为blk_w和blk_h的像素块
* 参  数：*
*        src                      -I       数据源指针
*        src_stride               -I       数据源stride
*        blk                      -O       像素块指针
*        blk_stride               -I       像素块stride
*        blk_x                    -I       像素块左上角点的横坐标
*        blk_y                    -I       像素块左上角点的纵坐标
*        blk_w                    -I       像素块宽度
*        blk_h                    -I       像素块高度
* 返回值：无
***************************************************************************************************/
HLM_VOID HLM_COM_GetBlock(HLM_U16  *src,
                          HLM_U32   src_stride,
                          HLM_U16  *blk,
                          HLM_U32   blk_stride,
                          HLM_U32   blk_x,
                          HLM_U32   blk_y,
                          HLM_U32   blk_w,
                          HLM_U32   blk_h);

/***************************************************************************************************
* 功  能：校验Patch划分能否拼成完整图像
* 参  数：*
*        pic_width                -I       图像宽度
*        pic_height               -I       图像高度
*        patch_info               -I       patch信息
* 返回值：校验是否通过(0/1)
* 备  注：
***************************************************************************************************/
HLM_S32 HLM_COM_CheckPatchSplit(HLM_S32            pic_width,
                                HLM_S32            pic_height,
                                HLM_PATCH_INFO    *patch_info);

#ifdef __cplusplus
}
#endif

#endif //_HLM_COMMON_H_
