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
#ifndef _HLMD_LIB_H_
#define _HLMD_LIB_H_

#include "hlm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// 输入解码库的码流信息结构体
typedef struct _HLMD_STREAM_IN
{
    HLM_U08 *stream_buf;                    // 码流缓冲区指针
    HLM_S32  stream_len;                    // 码流总长度
} HLMD_STREAM_IN;

// 解码库输出信息结构体
typedef struct _HLMD_PROCESS_OUT
{
    HLM_IMAGE  image_out;                   // 输出图像
    HLM_S32    image_poc;                   // 输出图像显示顺序
    HLM_U08    is_mismatch;                 // 是否编解码匹配
    HLM_U08    finish_one_frame;            // 是否解完一帧
} HLMD_PROCESS_OUT;

// 预解析的码流结构体
typedef struct _HLMD_PRE_BITSTREAM
{
    HLM_U08 *data;                          // 码流指针
    HLM_S32 bytes_remaining;                // 这段码流剩余字节数
    HLM_U64 next_bits;                      // 接下来要处理的8个字节存入
    HLM_S32 next_bits_cnt;                  // 8个字节中剩余比特数
    HLM_S32 zero_succession_cnt;            // 连续0x00出现次数
} HLMD_PRE_BITSTREAM;

// 预解析的图像信息
typedef struct _HLMD_VIDEO_INFO
{
    HLM_S32 code_width[3];
    HLM_S32 code_height[3];
    HLM_S32 ref_frm_num;
    HLM_U32 profile_idc;
    HLM_U32 crop_left;                      // 左边的裁剪像素值
    HLM_U32 crop_right;                     // 右边的裁剪像素值
    HLM_U32 crop_top;                       // 顶部的裁剪像素值
    HLM_U32 crop_bottom;                    // 底部的裁剪像素值
    HLM_U32 bit_depth_luma;                 // 亮度位宽
    HLM_U32 bit_depth_chroma;               // 色度位宽
    HLM_U32 format;                         // 图像格式
    HLM_S32 mv_search_width;                // mv搜索区域宽度，影响图像padding
    HLM_S32 mv_search_height;               // mv搜索区域高度，影响图像padding
} HLMD_VIDEO_INFO;

// 能力集参数
typedef struct _HLMD_ABILITY
{
    HLM_S32 code_width[3];                  // 能处理最大图像宽度（>= 64），要求16倍对齐
    HLM_S32 code_height[3];                 // 能处理最大图像高度（>= 64），要求16倍对齐
    HLM_S32 ref_frm_num;                    // 能处理最大图像参考帧数（<=16）
    HLM_S32 bit_depth_luma;
    HLM_S32 bit_depth_chroma;
} HLMD_ABILITY;

/***************************************************************************************************
* 功  能：预解析序列头
* 参  数：*
*         in_buf       -I       SPS包数据指针
*         in_size      -O       SPS包的大小
*         video_info   -I       图像序列信息结构体
* 返回值：状态码
* 备  注：如果mtab[i].size为0，则不需要分配该块内存
***************************************************************************************************/
HLM_STATUS HLMD_LIB_PreParseSeqHeader(HLM_VOID           *in_buf,
                                      HLM_SZT             in_size,
                                      HLMD_VIDEO_INFO    *video_info);

/***************************************************************************************************
* 功  能：获取解码算法模型所需存储信息
* 参  数：*
*         ability         -I    能力集参数指针
*         mem_tab         -O    存储空间参数结构体
*         video_info      -I    视频信息
* 返回值：状态码
* 备  注：如果mtab[i].size为0，则不需要分配该块内存
***************************************************************************************************/
HLM_STATUS HLMD_LIB_GetMemSize(HLMD_ABILITY     *ability,
                               HLM_MEM_TAB       mem_tab[HLM_MEM_TAB_NUM],
                               HLMD_VIDEO_INFO  *video_info);

/***************************************************************************************************
* 功  能：创建解码算法模型实例,内存初始化
* 参  数：*
*         ability         -I    能力集参数指针
*         mem_tab         -O    存储空间参数结构体
*         handle          -O    编码实例句柄指针
*         video_info      -I    视频信息
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_LIB_Create(HLMD_ABILITY     *ability,
                           HLM_MEM_TAB       mem_tab[HLM_MEM_TAB_NUM],
                           HLM_VOID        **handle,
                           HLMD_VIDEO_INFO  *video_info);

/***************************************************************************************************
* 功  能：解码一帧码流
* 参  数：*
*         handle    -I  解码实例句柄指针
*         in_buf    -I  解码模型输入参数地址
*         in_size   -I  解码模型输入参数大小
*         out_buf   -O  解码模型输出参数地址
*         out_size  -I  解码模型输出参数大小
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_LIB_DecodeFrame(HLM_VOID *handle,
                                HLM_VOID *in_buf,
                                HLM_SZT   in_size,
                                HLM_VOID *out_buf,
                                HLM_SZT   out_size);

#ifdef __cplusplus
}
#endif

#endif // _HLMD_LIB_H_
