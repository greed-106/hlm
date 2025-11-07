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
#ifndef _HLMD_DBP_H_
#define _HLMD_DBP_H_

#include "hlmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HLMD_MV_NUM_PER_CU                 (2)       // 一个宏块最多两个MV
#define HLMD_REF_IDX_NUM_PER_CU            (1)       // 一个宏块最多一个参考索引
#define HLMD_MAX_DPB_NUM                   (2)       // dpb大小

typedef struct _HLMD_DPB_SPEC
{
    HLM_S32       pic_num;                           // 表示DPB中存放的帧的数量
    HLMD_PIC_DATA pic_data[HLMD_MAX_DPB_NUM + 1];    // 表示DPB中存放的帧，暂不考虑差错隐藏兼容从P帧接入
} HLMD_DPB_SPEC;

/***************************************************************************************************
* 功  能：获取DPB模块需要的buffer size
* 参  数：*
*        width                   -I    解码图像宽度
*        height                  -I    解码图像高度
*        max_ref_num             -I    最大参考帧数量
*        status_size             -O    所需状态内存大小
*        work_size               -O    所需工作内存大小
*        mv_search_width         -I    MV搜索区域的宽度
*        mv_search_height        -I    MV搜索区域的高度
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_DPB_GetMemSize(HLM_S32  width[3],
                               HLM_S32  height[3],
                               HLM_S32  max_ref_num,
                               HLM_SZT *status_size,
                               HLM_SZT *work_size,
                               HLM_S32  mv_search_width,
                               HLM_S32  mv_search_height);

/***************************************************************************************************
* 功  能：DPB模块初始化，在解码库创建的时候调用此函数初始化
* 参  数：*
*        width                   -I    解码图像宽度
*        height                  -I    解码图像高度
*        max_ref_num             -I    最大参考帧数量
*        status_buf              -I    状态内存地址
*        work_buf                -I    工作内存地址
*        handle                  -IO   DPB模块句柄
*        mv_search_width         -I    MV搜索区域的宽度
*        mv_search_height        -I    MV搜索区域的高度
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_DPB_Create(HLM_S32    width[3],
                           HLM_S32    height[3],
                           HLM_S32    max_ref_num,
                           HLM_U08   *status_buf,
                           HLM_U08   *work_buf,
                           HLM_VOID **handle,
                           HLM_S32    mv_search_width,
                           HLM_S32    mv_search_height);

/**************************************************************************************************
* 功  能：从DPB获取当前解码帧存放空间
* 参  数：*
*         handle                -I    DPB模块SPEC内存指针
*         dec_pic_buf           -I    当前spec中的图像缓存区
*         curr_frame            -O    当前帧指针
*         dpb_free_index        -O    空闲帧索引
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_DPB_GetCurrFrame(HLM_VOID     *handle,
                                 HLMD_FRAME   *dec_pic_buf,
                                 HLMD_FRAME  **curr_frame,
                                 HLM_S32      *dpb_free_index);

/***************************************************************************************************
* 功  能：获取当前帧参考队列
* 参  数：*
*        dpb_free_index          -I    空闲帧索引
*        patch_ctx               -I   patch_ctx
*        curr_frame              -I   当前帧指针
*        ref_list                -O   当前图像的参考帧列表
*        ref_count               -O   前向/后向参考列表的参考帧个数
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_DPB_GetRefPicLists(HLM_S32           *dpb_free_index,
                                   HLMD_PATCH_CTX    *patch_ctx,
                                   HLMD_FRAME        *curr_frame,
                                   HLMD_REF_LIST      ref_list[2 * HLMD_MAX_REF_NUM],
                                   HLM_U32            ref_count);

/***************************************************************************************************
* 功  能：更新参考帧列表
* 参  数：*
*       nal_ref_idc           -I     最大参考帧数目
*       curr_frame            -I     当前帧地址
*       patch_ctx             -I0     解码条带级依赖参数
*       frame_poc             -IO     前一帧顶场的poc值
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_DPB_UpdateRefList(HLM_U32          nal_ref_idc,
                                  HLMD_FRAME      *curr_frame,
                                  HLMD_PATCH_CTX  *patch_ctx,
                                  HLM_S32         *frame_poc);

/***************************************************************************************************
* 功  能：释放DPB中无效参考帧
* 参  数：*
*        handle               -I    DPB模块句柄
*        pic_num              -I    DPB中的图像个数
*        dec_pic_buf          -IO   当前spec中的图像缓存区，图像内容指向DPB
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_DPB_ReleaseUnrefFrame(HLM_VOID          *handle,
                                      HLM_S32            pic_num,
                                      HLMD_FRAME        *dec_pic_buf);

#ifdef __cplusplus
}
#endif

#endif // _HLMD_DBP_H_
