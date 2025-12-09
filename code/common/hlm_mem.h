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
#ifndef _HLM_MEM_H_
#define _HLM_MEM_H_

#include "hlm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// RAM缓存管理结构体
typedef struct _HLM_RAM_BUF
{
    HLM_VOID *start;              // 缓存起始位置
    HLM_VOID *end;                // 缓存结束位置
    HLM_VOID *cur_pos;            // 缓存空余起始位置
} HLM_RAM_BUF;

/***************************************************************************************************
* 功  能：在RAM缓存中分配一块大小为size的内存块，地址align字节对齐
* 参  数：*
*         ram_buf          -I      RAM缓存管理结构体
*         size             -I      分配RAM缓存的大小
*         align            -I      分配RAM缓存所需对齐的字节数
* 返回值：(HLM_VOID *)分配得到RAM缓存位置指针
* 备  注：
***************************************************************************************************/
HLM_VOID* HLM_MEM_Calloc(HLM_RAM_BUF *ram_buf,
                         HLM_S32      size,
                         HLM_S32      align);

/***************************************************************************************************
* 功  能：按照对齐分配内存
* 参  数：*
*         size        -I    所需分配的内存大小
*         align       -I    对齐的字节数，需要为2的幂次方
* 返回值：所分配内存指针
* 备  注：HLM_Malloc与HLM_Free配套使用
***************************************************************************************************/
HLM_VOID* HLM_MEM_Malloc(HLM_SZT size,
                         HLM_S32 align);

/***************************************************************************************************
* 功  能：释放对齐申请的内存
* 参  数：*
*         buf        -I    内存指针
* 返回值：无
* 备  注：HLM_Malloc与HLM_Free配套使用
*         释放后没有将指针赋值为HLM_NULL
***************************************************************************************************/
HLM_VOID HLM_MEM_Free(HLM_VOID *buf);

/***************************************************************************************************
* 功  能：申请HLM_MEM_TAB内存
* 参  数：*
*         mem_tab        -I/O    内存块列表
*         tab_num        -I      内存块数量
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLM_MEM_AllocMemTab(HLM_MEM_TAB       *mem_tab,
                               HLM_S32            tab_num);

/***************************************************************************************************
* 功  能：释放HLM_MEM_TAB内存
* 参  数：*
*         mem_tab        -I/O    内存块列表
*         tab_num        -I      内存块数量
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLM_MEM_FreeMemTab(HLM_MEM_TAB *mem_tab,
                              HLM_S32      tab_num);

/***************************************************************************************************
* 功  能：检测HLM_MEM_TAB参数是否正确
* 参  数：*
*         mem_tab     -I    待检测的内存块列表
*         tab_num     -I    内存块列表数量
*         align       -I    内存块对齐要求
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLM_MEM_CheckMemTab(HLM_MEM_TAB       *mem_tab,
                               HLM_S32            tab_num,
                               HLM_MEM_ALIGNMENT  align);

#ifdef __cplusplus
}
#endif 

#endif //_HLM_MEM_H_
