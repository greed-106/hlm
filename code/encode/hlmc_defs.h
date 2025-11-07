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
#ifndef _HLMC_DEFS_H_
#define _HLMC_DEFS_H_

#include "hlmc_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HLMC_STREAM_BUF_RSV        (4096)      // 每个CU编码结束才检查缓冲区是否已满，预留4096字节，防止越界
#define HLMC_MAX_DPB_NUM           (2)         // 参考帧数+线程数+1，线程数为1
#define HLMC_LAMBDA_SHIFT          (6)         // lambda放大因子的移位

// Patch帧类型及参考关系
typedef enum _HLMC_PATCH_REF_TYPE
{
    HLMC_BASE_IDRPATCH = 0,                 //base层中的IDR帧
    HLMC_BASE_PPATCH_REFTOIDR,              //base层中的P帧，用于base层中其他帧的参考且只参考IDR帧
    HLMC_BASE_PPATCH_REFBYBASE,             //base层中的P帧，用于base层中其他帧的参考
    HLMC_BASE_PPATCH_REFBYENHANCE,          //base层中的P帧，用于enhance层中的帧的参考
    HLMC_ENHANCE_PPATCH_REFBYENHANCE,       //enhance层中的P帧，用于enhance层中其他帧的参考
    HLMC_ENHANCE_PPATCH_NOTFORREF,          //enhance层中的P帧，不用于参考
    HLMC_ENHANCE_PPATCH_BUTT,
    HLMC_VIRTUAL_I                          //虚拟I帧
} HLMC_PATCH_REF_TYPE;

// 复杂度等级
typedef enum _HLMC_RC_COMPLEX_LEVEL
{
    VERY_FLAT = 0,
    FLAT,
    MIDDLE,
    COMPLEX,
    VERY_COMPLEX,
    NUMS_COMPLEX
}HLMC_RC_COMPLEX_LEVEL;

typedef struct _VENC_RATE_CTRL_OUT_REGS
{
    HLM_U08  reg_patch_qp;                  // 帧级码控输出帧级QP
    HLM_U08  reg_qpmax;                     // 用户配置QPmax
    HLM_U08  reg_qpmin;                     // 用户配置QPmin
    HLM_U32  enc_pic_target_bits;           // 帧级目标比特数
}VENC_RATE_CTRL_OUT_REGS;

// 量化过程参数
typedef struct _HLMC_QUANT_PARAMS
{
    HLM_S32  quant_mf[6][16];
    HLM_S32  dequant_v[6][16];
    HLM_S32  bias_f[3];
}HLMC_QUANT_PARAMS;

typedef struct _HLMC_PU_INFO
{
    HLM_PU_INFO  inter_pu_info;
    HLM_MV       mv_16x8;                   // PU16x8 mv
    HLM_MV       mv_8x8;                    // PU8x8 mv
} HLMC_PU_INFO;

typedef struct _HLMC_CU_INFO
{
    HLM_CU_INFO        com_cu_info;
#if !MIX_IBC
    HLM_MV             bv_enc[4][8];        // 4种merge类型下的8个bv，按照zscan顺序存储
#endif
    HLMC_PU_INFO       pu_info_enc[2];      // pu信息
    HLMC_QUANT_PARAMS  quant_params;        // 量化过程参数
    HLM_U32            intra_rd_cost;       // intra rdo cost，4x4和16x8pk结果
    HLM_U32            intra_satd_cost;     // 记录当前16x8位置的最优satd
    HLM_U32            satd_comp[3];        // 三分量的satd值，用于计算复杂度
    HLM_U08            mix_flag;            // 当前宏块为混合复杂度
    HLM_S64            inter_satd_cost;
#if LINE_BY_LINE
    HLM_U32            intra_mode_cost;     // 记录当前16x8位置的最优cost
#endif
#if  IBC_SCALE
    HLM_CU_TYPE        best_cu_type;        // 粗搜最优模式
#endif
} HLMC_CU_INFO;

// 码流信息结构体
typedef struct _HLMC_BITSTREAM
{
    HLM_S32  bits_left;                     // 32位的byte_cache缓存中还剩未编码比特数
    HLM_U32  byte_cache;                    // 最新产生的码流先暂时放在这个四字节整型中，然后从MSB开始移到DDR中
    HLM_U32  bit_size;                      // DDR中码流缓冲区的最大尺寸（比特）
    HLM_U32  bit_cnt;                       // 已经写入DDR码流缓存区的比特数
    HLM_U32  emul_bytes;                    // 竞争起始码0的个数，该变量仅在patch_segment_data编码时才用到
    HLM_U08 *ptr_start;                     // DDR中码流首地址
    HLM_U08 *ptr;                           // DDR中当前码流地址
} HLMC_BITSTREAM;

//DPB中的帧结构体
typedef struct _HLMC_FRAME
{
    HLM_S32  poc;
    HLM_U32  dpb_id;
    HLM_U32  refed_flag;
    HLM_U16 *luma_ref_padding_y;            // 参考帧（或重建帧）padding后Y分量图像地址
    HLM_U16 *luma_ref_padding_cb;           // 参考帧（或重建帧）padding后Cb分量图像地址
    HLM_U16 *luma_ref_padding_cr;           // 参考帧（或重建帧）padding后Cr分量图像地址
    HLM_U16 *data[3];
    HLM_S32  step[3];
} HLMC_FRAME;

typedef struct _HLMC_DPB_SPEC
{
    HLM_S32  max_width;                     // 最大图像宽度，需要申请的内存大小与此有关，属于能力集参数
    HLM_S32  max_height;                    // 最大图像高度，需要申请的内存大小与此有关，属于能力集参数
    HLM_S32  max_ref_num;                   // 最大参考帧数量，需要申请的内存大小与此有关，属于能力集参数
    HLM_S32  max_dpb_num;                   // dpb最大帧数
    HLMC_FRAME  dpb[HLMC_MAX_DPB_NUM];
    HLMC_DPB_REF_CTRL  dpb_ctrl;            // 参考配置参数
} HLMC_DPB_SPEC;

//寄存器结构体
typedef struct _HLMC_REGS
{
    HLM_U32   enc_poc;                      // poc值
    HLM_U32   enc_frame_coding_type;        // 编码帧类型
    HLM_U32   bitdepth;                     // 比特深度
    HLM_U32   image_format;
    HLM_U32   intra_8x8_enable_flag;
    HLM_U32   cu_delta_qp_enable_flag;
    HLM_U32   i_frame_enable_ibc;
    HLM_U32   p_frame_enable_ibc;
    HLM_S32   mv_ref_cross_patch;
    HLM_S32   inter_pad_w_left;
    HLM_S32   inter_pad_w_right;
    HLM_S32   inter_pad_h_up;
    HLM_S32   inter_pad_h_down;
    HLM_PATCH_PARAM *cur_patch_param;       // 当前patch信息

    // 码控信息
    HLM_U32   enc_pic_init_qp;
    HLM_U32   enc_pic_qp;
    HLM_S32   enc_qp_min;                   // QP最小值
    HLM_S32   enc_qp_max;                   // QP最大值
    HLM_S32   enc_i_ave_row_qp;             // 行级QP总和
    HLM_S32   enc_p_ave_row_qp;             // 行级QP总和
    HLM_S32   enc_pic_target_bits;          // 帧级的目标比特数

    // 码流信息
    HLM_U08  *enc_output_strm_base;
    HLM_U08  *enc_output_strm;
    HLM_U32   enc_output_strm_buffer_limit; // 码流缓冲区的大小

    // 原始图像信息
    HLM_U16  *enc_input_y_base;             // 原始图像地址
    HLM_U16  *enc_input_cb_base;
    HLM_U16  *enc_input_cr_base;
    HLM_U32   enc_pic_width[2];             // 实际图像宽高
    HLM_U32   enc_pic_height[2];
    HLM_U32   enc_input_luma_stride;
    HLM_U32   enc_input_chroma_stride;

    HLM_U16  *enc_real_input_y_base;        // 原始patch地址
    HLM_U16  *enc_real_input_cb_base;
    HLM_U16  *enc_real_input_cr_base;
    HLM_U32   enc_real_pic_width[2];        // 实际patch宽高
    HLM_U32   enc_real_pic_height[2];
    HLM_U32   enc_real_input_luma_stride;
    HLM_U32   enc_real_input_chroma_stride;

    // 重建图像信息
    HLM_U16  *enc_recon_y_base;             // 重构图像地址
    HLM_U16  *enc_recon_cb_base;
    HLM_U16  *enc_recon_cr_base;

    HLM_U16  *enc_real_rec_y_base;          // 重构patch地址
    HLM_U16  *enc_real_rec_cb_base;
    HLM_U16  *enc_real_rec_cr_base;

    // 参考图像信息
    HLM_U16  *enc_ref_y_base;               // 参考帧图像地址
    HLM_U16  *enc_ref_cb_base;
    HLM_U16  *enc_ref_cr_base;
    HLM_U16  *enc_ref_y_padding_base;       // 参考帧padding后图像地址
    HLM_U16  *enc_ref_cb_padding_base;
    HLM_U16  *enc_ref_cr_padding_base;
    HLM_U32   enc_ref_frame_luma_stride;    // 参考帧跨度，padding后大小
    HLM_U32   enc_ref_frame_chroma_stride;

    // 其他信息
    HLM_U32   enc_satd_lamda[100];          // sad和satd共用的lambda
    HLM_U32   enc_sse_lamda[100];           // sse使用的lambda
    HLM_U08  *enc_ram_buf;
    HLM_SZT   enc_ram_len;
    HLM_S32   segment_enable_flag;
    HLM_S32   segment_width_in_log2;
    HLM_S32   segment_height_in_log2;
#if WRITE_PARAMETERS
    FILE     *fp_param;                     // log file pointer for "tool/BitstreamAnalyzer"
#endif
} HLMC_REGS;

typedef struct _HLMC_QPG
{
    HLM_S32 pic_base_qp;                    // 当前帧的基准qp,行级别qp从这个基准上进行偏移
    HLM_S32 i_sum_row_qp;                   // i帧行级qp累加
    HLM_S32 p_sum_row_qp;                   // i帧行级qp累加
    HLM_S32 cur_cu_qp;                      // 当前cu的qp
    HLM_S32 cu_rows;                        // 当前帧总CU行数
    HLM_S32 cu_cols;                        // 当前帧总CU列数
    HLM_S32 bpp;
#if WRITE_PARAMETERS
    HLM_U08 fullness_level;                 // 0, 1, 2
#endif

    HLM_S64 total_diff_bits;                // 已编码行数 * ave_target_bits_line - 已编码比特数,好像没必要写到结果体里面？
    HLM_S32 pic_target_bits;                // 帧级目标比特数
    HLM_S32 cur_cu_target_bits;             // 当前I宏块的目标比特
    HLM_S32 target_bits_cu[NUMS_COMPLEX];
    HLM_S32 total_bits;
    HLM_S32 total_bits_encoded;             // 已编码宏块的实际比特数
    HLM_S32 patch_target_bits;
    HLMC_RC_COMPLEX_LEVEL complex_level;
    HLMC_RC_COMPLEX_LEVEL complex_level_comp[3];
    HLM_S32 B_lossless[NUMS_COMPLEX];       // 每个复杂度下的无损比特数
    HLM_S32 avg_B;
    HLM_S32 rc_buffer_size_log2;            // 码控buffer大小
}HLMC_QPG;

typedef struct _HLMC_SPEC
{
    HLMC_ABILITY       ability;
    HLMC_CODING_CTRL   coding_ctrl;         // 序列级编码控制参数
    HLMC_RATE_CTRL     rate_ctrl;           // 码控设置参数
    HLMC_DPB_REF_CTRL  dpb_ref_ctrl;        // DPB配置参数
    HLM_PARAM_SPS      sps;                 // SPS
    HLM_PARAM_PPS      pps;                 // PPS
    HLMC_BITSTREAM     bs;                  // 码流信息结构体
    HLMC_BITSTREAM     out_bs;              // 最终输出的包含防竞争码的码流信息结构体
    HLMC_REGS         *regs;
    HLM_PATCH_HEADER   patch_ctx;
    HLM_NEIGHBOR_INFO  nbi_info;
    HLMC_QPG           rc_qpg;
    HLMC_CU_INFO       cur_cu;
    HLMC_CU_INFO       best_cu;

    HLM_VOID          *dpb_handle;          // DPB模块指针
    HLM_VOID          *rc_handle;           // RC模块指针
    HLM_U32            frame_num;           // 帧号
    HLM_U32            cu_cols;             // CU列数
    HLM_U32            cu_rows;             // CU行数
    HLM_U32            poc;                 // POC值
    HLM_U08           *ram_buf;             // RAM缓存地址
    HLM_SZT            ram_size;
    HLM_RAM_BUF        ram_buf_pic;         // RAM缓存地址
    HLM_U32            cu_total;            // CU总数
    HLM_U32            cu_cnt;              // 已编码的CU数
    HLM_U32            total_bs_len;        // 所有patch的码流长度
    HLM_U32            qp_cnt;              // 计算当前帧平均QP的pu个数
#if WRITE_PARAMETERS
    FILE              *fp_param;            // log file pointer for "tool/BitstreamAnalyzer"
#endif
} HLM_SPEC;

#ifdef __cplusplus
}
#endif

#endif //_HLMC_DEFS_H_
