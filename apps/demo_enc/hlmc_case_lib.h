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
#ifndef _HLMC_CASE_LIB_H_
#define _HLMC_CASE_LIB_H_

#include "hlmc_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HLMC_CFG_MAX_ITEMS     (512)           // 最多可解析的配置项数量
#define HLMC_FILE_PATH_SIZE    (256)           // 算法模型输入输出文件名（含路径）的最大尺寸

// 用户配置的测试用例参数
typedef struct _HLMC_CASE_CFG_PARAM
{
    char input_file[HLMC_FILE_PATH_SIZE];
    char output_file[HLMC_FILE_PATH_SIZE];
    char rec_file[HLMC_FILE_PATH_SIZE];
#if WRITE_PARAMETERS
    char param_file[HLMC_FILE_PATH_SIZE];
#endif
    HLM_U32 rec_flag;
    HLM_S32 total_frames;
#if FIX_1
    char patch_x     [HLM_MAX_PATCH_NUM << 3];
    char patch_y     [HLM_MAX_PATCH_NUM << 3];
    char patch_width [HLM_MAX_PATCH_NUM << 3];
    char patch_height[HLM_MAX_PATCH_NUM << 3];
#else
    char patch_x[HLMC_FILE_PATH_SIZE << 2];    // 最多256个patch，每个信息用4个符号
    char patch_y[HLMC_FILE_PATH_SIZE << 2];
    char patch_width[HLMC_FILE_PATH_SIZE << 2];
    char patch_height[HLMC_FILE_PATH_SIZE << 2];
#endif
} HLMC_CASE_CFG_PARAM;

// 控制命令行结构体
typedef struct _HLMC_DEMO_CMD
{
    char *file_cfg;                     // 配置文件名         -c
    char *file_log;                     // 运行日志文件名     -l
    FILE *fp_cfg;                       // 配置文件指针
    FILE *fp_log;                       // 运行日志文件指针
    char *file_input;                   // 待编码yuv文件名    -i
    char *file_output;                  // 编码码流文件名     -o
    char *file_rec;                     // 编码重构文件名     -rec
    int rec_on;                         // 重构yuv输出开关    -recon

    // 序列参数
    int frame_num;                      // 待编码帧数        -fn
    int width;                          // 输入yuv的宽       -wdt
    int height;                         // 输入yuv的高       -hgt
    int uniform_patch_split;            // 是否均匀划分patch
    char *patch_x;                      // patch横坐标
    char *patch_y;                      // patch纵坐标
    char *patch_width;                  // patch宽度
    char *patch_height;                 // patch高度
    int i_frame_enable_ibc;             // I帧是否开启IBC技术
    int p_frame_enable_ibc;             // P帧是否开启IBC技术
    int sub_ibc_enable_flag;            // 子块IBC使能标记
    int mv_ref_cross_patch;             // MV参考是否允许跨Patch
    int mv_search_width;                // MV搜索区域的宽度
    int mv_search_height;               // MV搜索区域的高度
    int frame_rate_num;                 // 帧率分子部分      -fr
    int frame_rate_den;                 // 帧率分母部分      -fd
    int intra_period;                   // I帧间隔           -I
    int bitdepth;                       // 比特深度
    int intra_8x8_enable_flag;          // 是否intra8x8
    int img_format;                     // 图像格式，0：yuv444, 1: rgb
    int intra_16x1_2x8_enable_flag;     // 是否开启帧内16x1和2x8模式
#if INTRA_CHROMA_MODE_SEPARATE
    int intra_chroma_mode_enable_flag;      // 帧内16x8预测亮色度分离
    int intra_sub_chroma_mode_enable_flag;  // 帧内子块预测亮色度分离
#endif
#if FIX_CFG_ENC
    int chroma_qp_offset;               // 色度QP偏移值
    int segment_enable_flag;            // 是否开启隔断参考
    int segment_width_in_log2;          // 隔断参考区域宽度（4个CU宽）的log
    int segment_height_in_log2;         // 隔断参考区域高度（4个CU高）的log
    int patch_extra_params_present_flag;// 片级是否额外接收部分高层语法
#endif

    //码控参数
    int rc_mode;                        // rc模式，0为关闭    -rc
    int bpp_i;                          // I帧BPP * 16
    int bpp_p;                          // P帧BPP * 16
    int rc_buffer_size_log2;            // 值为0表示采用默认参数
    int minqp;                          // minqp             -minqp
    int maxqp;                          // maxqp             -maxqp
    int iminqp;                         // I帧minqp          -iminqp
    int imaxqp;                         // I帧maxqp          -imaxqp
    int initqp;                         // rc开启时为首帧qp，rc关闭时为定qp的值   -q
    int qp_i;                           // I帧定QP设置
    int qp_p;                           // P帧定QP设置
} HLMC_DEMO_CMD;

typedef struct _HLMC_CFG_TOKEN_MAPPING
{
  HLM_S08  *token_name;
  HLM_VOID *place;
  HLM_S32   type;
} HLMC_CFG_TOKEN_MAPPING;

typedef struct _HLM_MSE_INFO
{
    HLM_F64 frame_mse[3];      // 三分量的帧级MSE
    HLM_F64 max_block_mse[3];  // 三分量的最大块级MSE
    HLM_S32 bmse_cu_x[3];      // 三分量的最大块级MSE对应的宏块坐标
    HLM_S32 bmse_cu_y[3];
} HLM_MSE_INFO;

extern HLMC_CASE_CFG_PARAM  case_cfg_param;    // 用户可配置的测试用例参数
extern HLMC_ABILITY         ability;           // 能力集参数
extern HLMC_CODING_CTRL     coding_ctrl;       // 序列级编码控制参数
extern HLMC_RATE_CTRL       rate_ctrl;         // 码率控制参数
extern HLMC_DPB_REF_CTRL    dpb_ref_ctrl;      // 参考配置参数
extern HLMC_PROCESS_IN      prc_in;            // 输入信息结构体
extern HLMC_PROCESS_OUT     prc_out;           // 输出信息结构体

/***************************************************************************************************
* 功  能：初始化配置文件内容
* 参  数：*
*        case_cfg_param           -OI       文件参数
*        coding_ctrl              -OI       编码控制参数
*        rate_ctrl                -OI       码控参数
*        dpb_ref_ctrl             -OI       dpb参数
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_CASE_InitCfgFile(HLMC_CASE_CFG_PARAM  *case_cfg_param,
                               HLMC_CODING_CTRL     *coding_ctrl,
                               HLMC_RATE_CTRL       *rate_ctrl,
                               HLMC_DPB_REF_CTRL    *dpb_ref_ctrl);

/***************************************************************************************************
* 功  能：读取测试用例配置文件内容
* 参  数：*
*         fp_cfg               -I         测试用例配置文件指针
*         fp_log               -O         运行日志文件指针
* 返回值：0：成功；-1：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_ReadCfgFile(FILE *fp_cfg,
                              FILE *fp_log);

/***************************************************************************************************
* 功  能：创建算法模型句柄
* 参  数：*
*         mem_tab              -I         内存指针，由外界统一管理
*         handle               -O         算法模型句柄
*         fp_log               -O         log文件指针
* 返回值：0：成功，其他：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_CreateHandle(HLM_MEM_TAB     mem_tab[HLM_MEM_TAB_NUM],
                               HLM_VOID      **handle,
                               FILE           *fp_log);

/***************************************************************************************************
* 功  能：销毁算法模型句柄
* 参  数：*
*         mem_tab              -I         内存指针，由外界统一管理
*         handle               -O         算法模型句柄
*         fp_log               -O         log文件指针
* 返回值：0：成功，其他：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_DestroyHandle(HLM_MEM_TAB     mem_tab[HLM_MEM_TAB_NUM],
                                HLM_VOID       *handle,
                                FILE           *fp_log);

/***************************************************************************************************
* 功  能：获取编码算法模型的版本和时间
* 参  数：无
* 返回值：编码算法模型版本和时间
* 备  注：版本信息格式为：主版本号（6位）＋子版本号（5位）＋修正版本号（5位）
***************************************************************************************************/
HLM_U32 HLMC_CASE_GetVersion();

/***************************************************************************************************
* 功  能：处理一个测试用例
* 参  数：*
*         handle               -I         算法模型句柄
*         fp_log               -O         log文件指针
* 返回值：0：成功，其他：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_ProcessSequence(HLM_VOID *handle,
                                  FILE     *fp_log);

/***************************************************************************************************
* 功  能：处理一个测试用例
* 参  数：*
*         fp_cfg               -I         测试用例配置文件名
*         fp_log               -O         log文件指针
*         cmd                  -I         命令行参数
* 返回值：0：成功；-1：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_Process(FILE           *fp_cfg,
                          FILE           *fp_log,
                          HLMC_DEMO_CMD  *cmd);

#ifdef __cplusplus
}
#endif

#endif // _HLMC_CASE_LIB_H_
