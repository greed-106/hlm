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
#ifndef _HLMC_LIB_H_
#define _HLMC_LIB_H_

#include "hlm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// 输入信息结构体
typedef struct _HLMC_PROCESS_IN
{
    HLM_IMAGE  image_in;                    // 输入图像
    HLM_U08   *stream_buf;                  // 外部分配的码流缓冲区地址，不包含防竞争码
    HLM_U08   *out_stream_buf;              // 外部分配的码流缓冲区地址，包含防竞争码
    HLM_U32    stream_buf_size;             // 外部分配的码流缓冲区长度，至少4096字节
    HLM_U32    force_idr;                   // 强制IDR帧标志，0：编码为IDR帧，1：不编码为IDR帧
#if WRITE_PARAMETERS
    FILE      *fp_param;                    // log file pointer for "tool/BitstreamAnalyzer"
#endif
} HLMC_PROCESS_IN;

// 输出信息结构体
typedef struct _HLMC_PROCESS_OUT
{
    HLM_U32    stream_len;                  // 码流的长度
    HLM_U32    sei_len;                     // 做完防伪起始码后的SEI长度，这部分比特不统计在码率里
    HLM_U16   *recon_dbk_y;                 // 重建数据
    HLM_U16   *recon_dbk_u;
    HLM_U16   *recon_dbk_v;
    HLM_U32    rec_stride_y;
    HLM_U32    rec_stride_c;
} HLMC_PROCESS_OUT;

// 能力集参数
typedef struct _HLMC_ABILITY
{
    HLM_U32    max_width;                   // 能处理最大图像宽度（>= 64）的偶数
    HLM_U32    max_height;                  // 能处理最大图像高度（>= 64）的偶数
} HLMC_ABILITY;

// Profile类型
typedef enum _HLMC_PROFILE
{
    HLMC_MAIN_PROFILE = 64,                 // 0x40
    HLMC_HIGH_PROFILE = 80                  // 0x50
} HLMC_PROFILE;

// 序列级编码控制参数
typedef struct _HLMC_CODING_CTRL
{
    HLM_U32         width;                  // 编码宽度，不超过能力集max_width的偶数
    HLM_U32         height;                 // 编码高度，不超过能力集max_height的偶数
    HLM_U32         bitdepth;               // 比特深度
    HLM_U32         intra_8x8_enable_flag;
    HLM_S32         intra_16x1_2x8_enable_flag;
#if INTRA_CHROMA_MODE_SEPARATE
    HLM_U16         intra_chroma_mode_enable_flag;
    HLM_U16         intra_sub_chroma_mode_enable_flag;
#endif
    HLM_U32         img_format;             // 图像格式
    HLM_U32         frame_rate_num;         // 帧率分子
    HLM_U32         frame_rate_denom;       // 帧率分母
    HLMC_PROFILE    profile;                // Profile
    HLM_S32         uniform_patch_split;    // 是否均匀划分patch
    HLM_PATCH_INFO  patch_info;
    HLM_S32         i_frame_enable_ibc;
    HLM_S32         p_frame_enable_ibc;
    HLM_S32         sub_ibc_enable_flag;
    HLM_S32         mv_ref_cross_patch;
    HLM_S32         mv_search_width;
    HLM_S32         mv_search_height;
    HLM_S32         chroma_qp_offset;
    HLM_S32         segment_enable_flag;
    HLM_S32         segment_width_in_log2;
    HLM_S32         segment_height_in_log2;
#if FIX_CFG_ENC
    HLM_S32         patch_extra_params_present_flag;
#endif
} HLMC_CODING_CTRL;

// DPB配置参数
typedef struct _HLMC_DPB_REF_CTRL
{
    HLM_U32 intra_period;                   // I帧间隔
} HLMC_DPB_REF_CTRL;

// 码率控制模式
typedef enum _HLMC_RATECTRL_TYPE
{
    HLMC_RC_CBR = 0,                        // 定码率
    HLMC_RC_FIXQP = 1,                      // 定QP
} HLMC_RATECTRL_TYPE;

// CBR码率控制参数
typedef struct _HLMC_RC_CBR_CFG
{
    HLM_S32   bpp_i;                        // 用户配置目标BPP * 16
    HLM_S32   bpp_p;                        // 用户配置目标BPP * 16
    HLM_S32   rc_buffer_size_log2;          // 值为0表示采用默认参数
    HLM_S32   init_qp;                      // 第一帧的起始Qp值
    HLM_S32   stat_time;                    // 统计时间（滑动窗口大小）,[1,60]
    HLM_S32   qp_min;                       // P帧qp最小值
    HLM_S32   qp_max;                       // P帧qp最大值
    HLM_S32   qp_min_i;                     // I帧qp最小值
    HLM_S32   qp_max_i;                     // I帧qp最大值
} HLMC_RC_CBR_CFG;

//FIX_QP参数
typedef struct _HLMC_RC_FIXQP_CFG
{
    HLM_U32 qp_i;                           // I帧QP
    HLM_U32 qp_p;                           // P帧QP
} HLMC_RC_FIXQP_CFG;

// 码率控制参数
typedef struct _HLMC_RATE_CTRL
{
    HLMC_RATECTRL_TYPE rate_ctrl_mode;
    HLMC_RC_CBR_CFG    rc_cbr_ctrl;
    HLMC_RC_FIXQP_CFG  rc_fixqp_ctrl;
} HLMC_RATE_CTRL;

/***************************************************************************************************
* 功  能：获取编码算法模型所需存储信息
* 参  数：*
*         ability          -I    能力集参数指针
*         mem_tab          -O    存储空间参数结构体
*         coding_ctrl      -I    编码控制结构体
* 返回值：状态码
* 备  注：如果mtab[i].size为0，则不需要分配该块内存
***************************************************************************************************/
HLM_STATUS HLMC_LIB_GetMemSize(HLMC_ABILITY      *ability,
                               HLM_MEM_TAB        mem_tab[HLM_MEM_TAB_NUM],
                               HLMC_CODING_CTRL  *coding_ctrl);

/***************************************************************************************************
* 功  能：创建编码算法模型实例,内存初始化
* 参  数：*
*         ability          -I    能力集参数指针
*         mem_tab          -O    存储空间参数结构体
*         handle           -O    编码实例句柄指针
*         coding_ctrl      -I    编码控制结构体
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_LIB_Create(HLMC_ABILITY     *ability,
                           HLM_MEM_TAB       mem_tab[HLM_MEM_TAB_NUM],
                           HLM_VOID        **handle,
                           HLMC_CODING_CTRL *coding_ctrl);

/***************************************************************************************************
* 功  能：设置序列级编码控制参数
* 参  数：*
*         handle       -O  编码实例句柄指针
*         code_params  -I  编码模型配置参数
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_LIB_SetCodingCtrl(HLM_VOID          *handle,
                                  HLMC_CODING_CTRL  *code_params);

/***************************************************************************************************
* 功  能：设置DPB及参考结构参数
* 参  数：*
*         handle        -O  编码实例句柄指针
*         dpb_params    -I  DPB结构和参考关系控制参数
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_LIB_SetDpbRefCtrl(HLM_VOID           *handle,
                                  HLMC_DPB_REF_CTRL  *dpb_params);

/***************************************************************************************************
* 功  能：编码一帧图像
* 参  数：*
*         handle    -I  编码实例句柄指针
*         in_buf    -I  编码模型输入参数地址
*         in_size   -I  编码模型输入参数大小
*         out_buf   -O  编码模型输出参数地址
*         out_size  -I  编码模型输出参数大小
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_LIB_EncodeFrame(HLM_VOID *handle,
                                HLM_VOID *in_buf,
                                HLM_SZT   in_size,
                                HLM_VOID *out_buf,
                                HLM_SZT   out_size);

/***************************************************************************************************
* 功  能：码率控制模块初始化
* 参  数：*
*         lib_handle         -IO       算法模型句柄
*         pic_width          -I        图像宽
*         pic_height         -I        图像高
*         rate_ctrl          -I        码控参数
* 返回值：无
* 备  注：
***************************************************************************************************/
#if FIX_3
HLM_STATUS HLMC_LIB_SetRateCtrl(HLM_VOID *lib_handle,
#else
HLM_VOID HLMC_LIB_InitRc(HLM_VOID        *lib_handle,
#endif
                         HLM_U32          pic_width,
                         HLM_U32          pic_height,
                         HLMC_RATE_CTRL  *rate_ctrl);

#ifdef __cplusplus
}
#endif

#endif // _HLMC_LIB_H_
