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
#ifndef _HLMD_INIT_H_
#define _HLMD_INIT_H_

#include "hlmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* 功  能：检查算法库主处理函数输入输出参数是否有效
* 参  数：
*         handle         -I          库实例句柄
*         in_buf         -I          处理输入参数地址
*         in_size        -I          处理输入参数大小
*         out_buf        -O          处理输出参数地址
*         out_size       -I          处理输出参数大小
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_INIT_CheckIo(HLM_VOID *handle,
                             HLM_VOID *in_buf,
                             HLM_SZT   in_size,
                             HLM_VOID *out_buf,
                             HLM_SZT   out_size);

/***************************************************************************************************
* 功  能：检测能力集参数
* 参  数：*
*         ability       -I            能力集参数
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_INIT_CheckAbility(HLMD_ABILITY *ability);

/***************************************************************************************************
* 功  能：分配外部DDR不可复用的缓存，并计算所需大小
* 参  数：*
*         spec          -I/O        模块参数结构体指针
*         mem_tab       -I          缓存表结构体指针
*         alloc_size    -O          所需DDR缓存大小 
* 返回值：状态码
* 备  注：主要是公共缓存部分
***************************************************************************************************/
HLM_STATUS HLMD_INIT_AllocDdrPersistMem(HLMD_SW_SPEC   *spec,
                                        HLM_MEM_TAB    *mem_tab,
                                        HLM_SZT        *alloc_size);

/***************************************************************************************************
* 功  能：分配内部RAM缓存，并计算所需RAM缓存大小
* 参  数：*
*         spec          -I/O        模块参数结构体指针
*         mem_tab       -I          缓存表结构体指针
*         alloc_size    -O          所需RAM缓存大小 
* 返回值：状态码
* 备  注：RAM缓存实际为硬件模块使用，但是因为硬件模块没有GetMemSize和Create接口，因此在上层进行分配
***************************************************************************************************/
HLM_STATUS HLMD_INIT_AllocRamScratchMem(HLMD_SW_SPEC   *spec,
                                        HLM_MEM_TAB    *mem_tab,
                                        HLM_SZT        *alloc_size);

/***************************************************************************************************
* 功  能：计算各子模块所需状态内存大小
* 参  数：*
*         spec          -I/O  模块参数结构体指针
*         status_size   -O    所需状态内存大小
*         work_size     -O    所需工作内存大小
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_INIT_GetModuleBuf(HLMD_SW_SPEC  *spec,
                                  HLM_SZT       *status_size,
                                  HLM_SZT       *work_size);

/***************************************************************************************************
* 功  能：分配各子模块所需状态内存大小，创建子模块
* 参  数：*
*         spec          -I/O  模块参数结构体指针
*         status_buf    -I    状态内存地址
*         status_size   -I    所分配状态内存大小
*         work_buf      -I    工作内存地址
*         work_size     -I    所分配工作内存大小
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_INIT_AllocModuleBuf(HLMD_SW_SPEC  *spec,
                                    HLM_U08       *status_buf,
                                    HLM_SZT        status_size,
                                    HLM_U08       *work_buf,
                                    HLM_SZT        work_size);

#ifdef __cplusplus
}
#endif

#endif // _HLMD_INIT_H_
