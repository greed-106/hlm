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
#ifndef _HLMC_INTER_H_
#define _HLMC_INTER_H_

#include "hlmc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* 功  能：整像素运动搜索
* 参  数：*
*        regs                 -I    当前数据硬件寄存器
*        nbi_info             -I    相邻块信息
*        cur_cu               -IO   当前cu信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_INTER_MODE(HLMC_REGS           *regs,
                         HLM_NEIGHBOR_INFO   *nbi_info,
                         HLMC_CU_INFO        *cur_cu);

/***************************************************************************************************
* 功  能：帧间RDO模块
* 参  数：*
*        channel_index        -I    当前通道的索引
*        channel_size         -I    当前通道的大小
*        regs                 -I    当前数据硬件寄存器
*        nbi_info             -I    相邻块信息
*        best_cu              -O    最优cu信息
*        cur_cu               -I    当前cu信息
* 返回值：无
***************************************************************************************************/
HLM_VOID HLMC_INTER_RDO(HLM_U32                 channel_index,
                        HLM_U32                 channel_size,
                        HLMC_REGS              *regs,
                        HLM_NEIGHBOR_INFO      *nbi_info,
                        HLMC_CU_INFO           *best_cu,
                        HLMC_CU_INFO           *cur_cu);

#ifdef __cplusplus
}
#endif

#endif // _HLMC_INTER_H_
