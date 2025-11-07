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
#ifndef _HLM_NBI_H_
#define _HLM_NBI_H_

#include "hlm_com_pred.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* 功  能：相邻信息管理
* 参  数：*
*        cur_cu                 -I      当前CU信息
*        nbi_info               -IO     相邻信息数据
*        frame_type             -I      当前帧类型
*        blk8_pu0_info          -I      8x8划分下第一个PU的信息
*        blk8_pu1_info          -I      8x8划分下第二个PU的信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_NBI_Process(HLM_CU_INFO         *com_cu_info,
                         HLM_NEIGHBOR_INFO   *nbi_info,
                         HLM_U32              frame_type,
                         HLM_PU_INFO         *blk8_pu0_info,
                         HLM_PU_INFO         *blk8_pu1_info);

/***************************************************************************************************
* 功  能：更新相邻的重建像素
* 参  数：*
*        com_cu_info            -I      当前CU信息
*        nbi_info               -IO     相邻信息数据
*        recon_y_base           -I      亮度重建首地址
*        recon_cb_base          -I      色度重建首地址
*        recon_cr_base          -I      色度重建首地址
*        luma_stride            -I      亮度步长
*        chroma_stride          -I      色度步长
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_NBI_UpdateRec(HLM_CU_INFO         *com_cu_info,
                           HLM_NEIGHBOR_INFO   *nbi_info,
                           HLM_U16             *recon_y_base,
                           HLM_U16             *recon_cb_base,
                           HLM_U16             *recon_cr_base,
                           HLM_U32              luma_stride,
                           HLM_U32              chroma_stride);

#ifdef __cplusplus
}
#endif

#endif // _HLM_NBI_H_
