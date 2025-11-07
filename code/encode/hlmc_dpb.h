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
#ifndef _HLMC_DBP_H_
#define _HLMC_DBP_H_

#include "hlmc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* 功  能：获取DPB模块需要的buffer size
* 参  数：*
*        width                   -I    解码图像宽度
*        height                  -I    解码图像高度
*        status_size             -O    所需状态内存大小
*        work_size               -O    所需工作内存大小
*        mv_search_width         -I    MV搜索区域的宽度
*        mv_search_height        -I    MV搜索区域的高度
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_DPB_GetMemSize(HLM_S32    width,
                               HLM_S32    height,
                               HLM_SZT   *status_size,
                               HLM_SZT   *work_size,
                               HLM_S32    mv_search_width,
                               HLM_S32    mv_search_height);

/***************************************************************************************************
* 功  能：DPB模块初始化，在解码库创建的时候调用此函数初始化
* 参  数：*
*        width                   -I    解码图像宽度
*        height                  -I    解码图像高度
*        status_buf              -I    状态内存地址
*        work_buf                -I    工作内存地址
*        handle                  -IO   DPB模块句柄
*        mv_search_width         -I    MV搜索区域的宽度
*        mv_search_height        -I    MV搜索区域的高度
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_DPB_Create(HLM_S32    width,
                           HLM_S32    height,
                           HLM_U08   *status_buf,
                           HLM_U08   *work_buf,
                           HLM_VOID **handle,
                           HLM_S32    mv_search_width,
                           HLM_S32    mv_search_height);

/****************************************************************************************
* 功  能：设置DPB参数
* 参  数：
*         handle            -IO         DPB模块句柄
*         dpb_params        -I          DPB结构和参考关系控制参数
*         dpb_num           -O          当前DPB下需要DPB中帧数
* 返回值：状态码，成功返回HLM_STS_OK，创建失败返回HLM_STS_ERR
* 备  注：
***************************************************************************************/
HLM_STATUS HLMC_DPB_SetDpbRefCtrl(HLM_VOID             *handle,
                                  HLMC_DPB_REF_CTRL    *dpb_params,
                                  HLM_U32              *dpb_num);

/****************************************************************************************
* 功  能：获取参考帧有关信息
* 参  数：
*           handle              -IO     DPB模块句柄
*           poc_in              -I      输入poc
*           force_idr           -I      强制IDR帧标志
*           poc_out             -O      输出poc
*           ref_idx             -O      当前帧参考帧在DPB的序号
*           rec_idx             -O      重构帧在DPB的序号
*           patch_type          -O      帧类型
* 返回值：状态码，成功返回HLM_STS_OK，创建失败返回HLM_STS_ERR
* 备  注：
***************************************************************************************/
HLM_STATUS HLMC_DPB_Get(HLM_VOID               *handle,
                        HLM_S32                 poc_in,
                        HLM_S32                 force_idr,
                        HLM_S32                *poc_out,
                        HLM_S32                *ref_idx,
                        HLM_S32                *rec_idx,
                        HLMC_PATCH_REF_TYPE    *patch_type);

/***************************************************************************************************
* 功  能：获取DPB参考关系控制参数
* 参  数：*
*         handle        -I     DPB模块句柄指针
*         dpb_params    -O     DPB结构和参考关系控制参数
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_DPB_GetDpbRefCtrl(HLM_VOID              *handle,
                                  HLMC_DPB_REF_CTRL     *dpb_params);

#ifdef __cplusplus
}
#endif

#endif // _HLMC_DBP_H_
