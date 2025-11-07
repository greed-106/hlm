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
#include <stdlib.h>
#include "hlm_mem.h"
#include "hlm_common.h"

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
                         HLM_S32      align)
{
    HLM_SZT free_size = 0;  // 剩余空间大小
    HLM_VOID *buf     = HLM_NULL;

    // 缓存中空余空间起始位置
    buf = (HLM_VOID *)(((HLM_SZT)(ram_buf->cur_pos) + (align - 1)) & (~(align - 1)));

    // 计算缓存中的空余空间大小
    free_size = (HLM_SZT)(ram_buf->end) - (HLM_SZT)buf + 1;

    // 空间不够，返回空指针
    if (free_size < (HLM_SZT)size)
    {
        printf("Memory space is not enough!\n");
        buf = HLM_NULL;
    }
    else
    {
        // 清空分配缓存
        memset(buf, 0, size);

        // 更新空余指针位置
        ram_buf->cur_pos = (HLM_VOID *)((HLM_SZT)buf + size);
    }

    return buf;
}

/***************************************************************************************************
* 功  能：按照对齐分配内存
* 参  数：*
*         size        -I    所需分配的内存大小
*         align       -I    对齐的字节数，需要为2的幂次方
* 返回值：所分配内存指针
* 备  注：HLM_Malloc与HLM_Free配套使用
***************************************************************************************************/
HLM_VOID* HLM_MEM_Malloc(HLM_SZT size,
                         HLM_S32 align)
{
    HLM_U08 *base_buf = HLM_NULL;
    HLM_U08 *use_buf  = HLM_NULL;

    HLM_CHECK_ERROR((size  <= 0), HLM_NULL);
    HLM_CHECK_ERROR((align <= 0), HLM_NULL);

    base_buf = (HLM_U08 *)malloc(size + align + sizeof(HLM_U08 *));
    if(HLM_NULL == base_buf)
    {
        return HLM_NULL;
    }

    use_buf = base_buf + sizeof(HLM_U08 *);

    while((HLM_SZT)use_buf % (HLM_SZT)align)
    {
        use_buf++;
    }

    *(HLM_U08 **)(use_buf - sizeof(HLM_U08 *)) = base_buf;

    return (HLM_VOID *)use_buf;
}

/***************************************************************************************************
* 功  能：释放对齐申请的内存
* 参  数：*
*         buf        -I    内存指针
* 返回值：无
* 备  注：HLM_Malloc与HLM_Free配套使用
*         释放后没有将指针赋值为HLM_NULL
***************************************************************************************************/
HLM_VOID HLM_MEM_Free(HLM_VOID *buf)
{
    HLM_U08 *base_buf = HLM_NULL;
    HLM_U08 *use_buf  = HLM_NULL;

    if(HLM_NULL != buf)
    {
        use_buf  = (HLM_U08 *)buf;
        base_buf = *(HLM_U08 **)(use_buf - sizeof(HLM_U08 *));

        free(base_buf);
    }
}

/***************************************************************************************************
* 功  能：申请HLM_MEM_TAB内存
* 参  数：*
*         mem_tab        -I/O    内存块列表
*         tab_num        -I      内存块数量
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLM_MEM_AllocMemTab(HLM_MEM_TAB       *mem_tab,
                               HLM_S32            tab_num)
{
    HLM_S32   i             = 0;
    HLM_SZT   size          = 0;
    HLM_VOID *buf           = HLM_NULL;
    HLM_MEM_ALIGNMENT align = HLM_MEM_ALIGN_4BYTE;

    HLM_CHECK_ERROR((HLM_NULL == mem_tab), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((tab_num < 1),         HLM_STS_ERR_BAD_ARG);

    for (i = 0; i < tab_num; i++)
    {
        size  = mem_tab[i].size;
        align = mem_tab[i].alignment;

        if (size != 0)
        {
            buf = HLM_MEM_Malloc(size, (HLM_S32)align);
            HLM_CHECK_ERROR((HLM_NULL == buf), HLM_STS_ERR_NULL_PTR);
        }
        else
        {
            buf = HLM_NULL;
        }

        mem_tab[i].base = buf;
    }

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：释放HLM_MEM_TAB内存
* 参  数：*
*         mem_tab        -I/O    内存块列表
*         tab_num        -I      内存块数量
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLM_MEM_FreeMemTab(HLM_MEM_TAB *mem_tab,
                              HLM_S32      tab_num)
{
    HLM_S32 i = 0;

    HLM_CHECK_ERROR((HLM_NULL == mem_tab), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((tab_num < 1),         HLM_STS_ERR_BAD_ARG);

    for (i = 0; i < tab_num; i++)
    {
        if (HLM_NULL != mem_tab[i].base)
        {
            HLM_MEM_Free(mem_tab[i].base);
            mem_tab[i].base = HLM_NULL;
        }
    }

    return HLM_STS_OK;
}

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
                               HLM_MEM_ALIGNMENT  align)
{
    HLM_S32 i    = 0;
    HLM_SZT addr = 0;

    HLM_CHECK_ERROR((HLM_NULL == mem_tab), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((tab_num <= 0), HLM_STS_ERR_NULL_PTR);

    for (i = 0; i < tab_num; i++)
    {
        if (mem_tab[i].size != 0)
        {
            HLM_CHECK_ERROR((HLM_NULL == mem_tab[i].base), HLM_STS_ERR_MEM_NULL);
            HLM_CHECK_ERROR((mem_tab[i].alignment != align), HLM_STS_ERR_MEM_ALIGN);

            addr = (HLM_SZT)mem_tab[i].base;
            HLM_CHECK_ERROR((addr & (align - 1)), HLM_STS_ERR_MEM_ADDR_ALIGN);
        }
    }

    return HLM_STS_OK;
}
