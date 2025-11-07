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
#ifndef _HLMD_DEFS_H_
#define _HLMD_DEFS_H_

#include "hlmd_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HLMD_MAX_REF_NUM           (1)         // 最大参考帧数

#if defined(_M_IX86)
#define HLMD_BSWAP(a)              __asm mov eax, a    __asm bswap eax     __asm  mov a, eax
#elif defined(_M_ARM)
#if defined(__ANDROID__)
#define HLMD_BSWAP(a)              __asm("rev %0, %1":"=r"(tmp):"r"(tmp));
#else
#define HLMD_BSWAP(a)              (a = HLMD_BSWAP32(a)) 
#endif
#elif defined(_M_X64)
#define HLMD_BSWAP(a)              (a = (((a & 0xFF) << 24)    | \
                                         ((a & 0xFF00) << 8)   | \
                                         ((a & 0xFF0000) >> 8) | \
                                         ((a & 0xFF000000) >> 24)))
#else
#define HLMD_BSWAP(a)              (a = HLMD_BSWAP32(a))
#endif

// NALU头信息
typedef struct _HLMD_NALU_HEADER
{
    HLM_U32  forbidden_zero_bit;
    HLM_U32  nal_ref_idc;
    HLM_U32  nal_unit_type;
} HLMD_NALU_HEADER;

// 含像素值信息的帧
typedef struct _HLMD_YUV_FRAME
{
    HLM_U16 *data[3];
    HLM_S32  step[3];
} HLMD_YUV_FRAME;

// 解码一帧所有图像属性
typedef struct _HLMD_PIC_DATA_
{
    HLMD_YUV_FRAME  yuv;                       // 图像地址和跨度
    HLM_S32         fram_num;                  // 当前帧的frame_num
    HLM_S32         dpb_lock_flag;             // DPB中的图像位置是否可用，0表示可用，1表示不可用
    HLM_U32         unref_set_id;              // 释放无效参考帧的线程标识
    HLM_U16        *cu_type_mem;               // 存储所有宏块类型的内存指针
    HLM_S16        *mv;                        // L0、L1两个方向，每个方向宏块数 * 16份(mv_x, mv_y)
    HLM_S08        *ref_idx;                   // L0、L1两个方向，每个方向宏块数 * 4个字节
    HLM_U16        *luma_ref_padding_y;        // 参考帧（或重建帧）padding后Y分量图像地址
    HLM_U16        *luma_ref_padding_cb;       // 参考帧（或重建帧）padding后Cb分量图像地址
    HLM_U16        *luma_ref_padding_cr;       // 参考帧（或重建帧）padding后Cr分量图像地址
    HLM_U16        *patch_padding_recon[3];    // padding后的patch重建数据，stride同yuv.step
} HLMD_PIC_DATA;

// 帧结构体
typedef struct _HLMD_FRAME
{
    HLMD_PIC_DATA *ref_pic_data;
} HLMD_FRAME;

// 参考帧列表结构体
typedef struct _HLMD_REF_LIST
{
    HLMD_PIC_DATA  ref_pic_data;
} HLMD_REF_LIST;

// 条带上下文信息结构体
typedef struct _HLMD_PATCH_CTX
{
    HLM_PATCH_HEADER   patch_header;           // 条带头信息
    HLM_U32            last_ref_count;
    HLM_S32            last_patch_type;        // 前一个patch类型
    HLM_S32            first_cu_in_patch;
    HLM_U32            nal_ref_idc;            // nal单元nal_ref_idc
    HLM_U32            idr_frame_flag;         // idr标志
    HLM_U32            ref_count;              // 参考列表中的参考帧数量
    HLM_U32            short_ref_cnt;          // 短期参考帧的数量
    HLMD_FRAME        *short_ref_list[HLMD_MAX_REF_NUM + 1];  // 短期参考帧列表
    HLMD_REF_LIST      ref_list[HLMD_MAX_REF_NUM];            // 前向参考帧列表
} HLMD_PATCH_CTX;

// 码流信息结构体
typedef struct _HLMD_BITSTREAM
{
    HLM_U32  max_bits_num;                     // 码流的长度, 单位bit
    HLM_U08 *init_buf;                         // initbuf指向每一帧的数据开始地址
    HLM_U32  bits_cnt;                         // bitcnt为在此基础上当前位置的偏移，单位为bit
} HLMD_BITSTREAM;

// 算法模型内cu_info公共参数
typedef struct _HLMD_CU_INFO
{
    HLM_CU_INFO com_cu_info;                   // 编解码公共参数
    HLM_MV    mvd_l0[2];                       // 当前宏块的mvd值
    HLM_U16   luma_bitdepth;
    HLM_U16   chroma_bitdepth;
    HLM_COEFF dc_coeffs[3][HLM_TU_4x4_NUMS];   // 解码的DC系数
    HLM_U08   dc_coeffs_num[3];                // DC非零系数个数
#if !MIX_IBC
    HLM_MV    bv[8];
#endif
} HLMD_CU_INFO;

// 硬件解码层配置信息结构体
typedef struct _HLMD_REGS
{
    HLMD_PATCH_CTX  *patch_ctx;
    HLMD_FRAME      *cur_frame;                // 当前工作帧缓存
    HLM_U32   dec_frame_coding_type;           // 解码帧类型
    HLM_S32   segment_enable_flag;
    HLM_S32   segment_width_in_log2;
    HLM_S32   segment_height_in_log2;
    HLMD_BITSTREAM  *bs;
    HLM_S32   image_format;
    HLM_U32   intra_8x8_enable_flag;
    HLM_U32   cu_delta_qp_enable_flag;

    // 输出
    HLM_U32   dec_pic_width;                   // 解码图像宽度
    HLM_U32   dec_pic_height;                  // 解码图像高度
    HLM_U32   dec_bitdepth;
    HLM_U32   dec_pic_luma_bitdepth;           // 解码图像亮度位宽
    HLM_U32   dec_pic_chroma_bitdepth;         // 解码图像色度位宽
    HLM_U16  *dec_recon_y_base;                // 重构图像地址
    HLM_U16  *dec_recon_cb_base;
    HLM_U16  *dec_recon_cr_base;

    // stride
    HLM_U32   dec_output_luma_stride;          // 重构图像跨度
    HLM_U32   dec_output_chroma_stride;
    HLM_U32   dec_ref_frame_luma_stride;       // 参考帧跨度，padding后大小
    HLM_U32   dec_ref_frame_chroma_stride;
    HLM_U16  *dec_ref_y_padding_base;          // 参考帧padding后图像Y分量基地址
    HLM_U16  *dec_ref_cb_padding_base;         // 参考帧padding后图像cb分量基地址
    HLM_U16  *dec_ref_cr_padding_base;         // 参考帧padding后图像cr分量基地址

    HLM_PATCH_PARAM *cur_patch_param;          // 当前patch的参数
    HLMD_CU_INFO    *cur_cu;
    HLM_NEIGHBOR_INFO *nbi_info;
    HLM_S32   mv_ref_cross_patch;
    HLM_S32   inter_pad_w_left;
    HLM_S32   inter_pad_w_right;
    HLM_S32   inter_pad_h_up;
    HLM_S32   inter_pad_h_down;
} HLMD_REGS;

// 算法模型内公共参数结构体，等效算法模型句柄
typedef struct _HLMD_SW_SPEC
{
    HLMD_ABILITY       ability;
    HLM_VOID          *dpb_handle;             // DPB模块指针
    HLMD_FRAME        *dec_pic_buf;
    HLMD_FRAME        *cur_frame;              // 当前工作帧缓存
    HLM_S32            dpb_free_index;         // DPB空闲帧索引
    HLM_PARAM_SPS      sps;                    // SPS
    HLM_PARAM_PPS      pps;                    // PPS
    HLMD_PATCH_CTX     patch_ctx;              // patch_ctx
    HLM_U32            poc;                    // POC值
    HLM_U08           *ram_buf;                // RAM缓存地址
    HLM_SZT            ram_size;
    HLM_U32            coded_patch_cnt;        // 当前帧的已解码patch个数
    HLMD_CU_INFO       cur_cu;                 // 在软件层申请内存，然后传给寄存器和硬件层
    HLM_NEIGHBOR_INFO  nbi_info;
} HLMD_SW_SPEC;

// 算法模型内公共参数结构体，等效算法模型句柄
typedef struct _HLMD_HW_SPEC
{
    HLMD_REGS         *regs;
    HLMD_CU_INFO      *cur_cu;
    HLM_NEIGHBOR_INFO *nbi_info;
    HLMD_BITSTREAM     bs;
    HLM_U32            cu_rows;                // CU行数
    HLM_U32            cu_cols;                // CU列数
    HLM_S32            bit_depth_luma;
    HLM_S32            bit_depth_chroma;
} HLMD_HW_SPEC;

#ifdef __cplusplus
}
#endif

#endif // _HLMD_DEFS_H_
