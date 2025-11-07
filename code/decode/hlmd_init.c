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
#include "hlmd_init.h"
#include "hlmd_dpb.h"

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
                             HLM_SZT   out_size)
{
    HLMD_SW_SPEC     *spec    = HLM_NULL;
    HLMD_STREAM_IN   *input   = HLM_NULL;
    HLMD_PROCESS_OUT *output  = HLM_NULL;
    HLM_IMAGE        *src_img = HLM_NULL;

    HLM_CHECK_ERROR((HLM_NULL == handle) || (HLM_NULL == in_buf) || (HLM_NULL == out_buf), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((in_size != sizeof(HLMD_STREAM_IN)) || (out_size != sizeof(HLMD_PROCESS_OUT)), HLM_STS_ERR_PRC_SIZE);

    // 参数初始化
    spec = (HLMD_SW_SPEC *)handle;
    input = (HLMD_STREAM_IN *)in_buf;
    output = (HLMD_PROCESS_OUT *)out_buf;

   // 输入码流检测
    HLM_CHECK_ERROR(HLM_NULL == input->stream_buf , HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR(input->stream_len < 0 , HLM_STS_ERR_PARAM_VALUE);

    // 输出图像检测
    HLM_CHECK_ERROR(HLM_NULL == output->image_out.data, HLM_STS_ERR_NULL_PTR);

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：检测能力集参数
* 参  数：*
*         ability       -I            能力集参数
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_INIT_CheckAbility(HLMD_ABILITY *ability)
{
    // 保证宽度和高度均不小于64
    HLM_CHECK_ERROR((ability->code_width[0]  < HLM_IMG_WIDTH_MIN)
        || (ability->code_height[0] < HLM_IMG_HEIGHT_MIN), HLM_STS_ERR_ABILITY_ARG);

    // 保证宽度和高度均为偶数
    HLM_CHECK_ERROR((ability->code_width[0] & 0x1)
        || (ability->code_height[0] & 0x1), HLM_STS_ERR_ABILITY_ARG);

    HLM_CHECK_ERROR((ability->code_width[0] > HLM_IMG_WIDTH_MAX) || (ability->code_height[0] > HLM_IMG_HEIGHT_MAX), HLM_STS_ERR_ABILITY_ARG);

    // 限制最大参考帧数量
    HLM_CHECK_ERROR((ability->ref_frm_num > HLMD_MAX_REF_NUM), HLM_STS_ERR_ABILITY_ARG);

    return HLM_STS_OK;
}

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
                                        HLM_SZT        *alloc_size)
{
    HLM_SZT            size       = 0;
    HLM_SZT            used_size  = 0;
    HLM_SZT            total_size = mem_tab->size;
    HLM_MEM_ALIGNMENT  alignment  = mem_tab->alignment;

    // 跳过spec结构体，在外面已分配，只计算缓存大小
    size = sizeof(HLMD_SW_SPEC);
    size = HLM_SIZE_ALIGN(size, alignment);
    used_size += size;

    //检测内存大小是否超过分配大小
    HLM_CHECK_ERROR(used_size > total_size, HLM_STS_ERR_MEM_LACK);

    *alloc_size = used_size;

    return HLM_STS_OK;
}

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
                                        HLM_SZT        *alloc_size)
{
    HLM_SZT            size       = 0;
    HLM_SZT            used_size  = 0;
    HLM_SZT            total_size = mem_tab->size;
    HLM_MEM_ALIGNMENT  alignment  = mem_tab->alignment;
    HLM_U08           *acc_addr   = (HLM_U08 *)(mem_tab->base);

    //按照最大规格8K 16bit YUV444来申请,需分配64MB
    size = 64 * 1024 * 1024;
    size = HLM_SIZE_ALIGN(size, alignment);
    spec->ram_buf = acc_addr;
    spec->ram_size = size;
    used_size += size;

    //检测内存大小是否超过分配大小
    HLM_CHECK_ERROR(used_size > total_size, HLM_STS_ERR_MEM_LACK);

    *alloc_size = used_size;

    return HLM_STS_OK;
}

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
                                  HLM_SZT       *work_size)
{
    HLM_STATUS sts            = HLM_STS_ERR;
    HLMD_ABILITY *ability     = &(spec->ability);
    HLM_U32 max_width[3]      = { HLM_SIZE_ALIGN_8(ability->code_width[0]),
                                  HLM_SIZE_ALIGN_8(ability->code_width[0]),
                                  HLM_SIZE_ALIGN_8(ability->code_width[0]) };
    HLM_U32 max_height[3]     = { HLM_SIZE_ALIGN_8(ability->code_height[0]),
                                  HLM_SIZE_ALIGN_8(ability->code_height[0]),
                                  HLM_SIZE_ALIGN_8(ability->code_height[0]) };
    HLM_SZT modu_status_size  = 0; //单个模块所需状态内存大小
    HLM_SZT modu_work_size    = 0; //单个模块所需状态工作大小
    HLM_SZT total_status_size = 0; //模块使用累计状态内存大小
    HLM_SZT total_work_size   = 0; //模块使用累计工作内存大小，暂时先不考虑模块之间复用

    switch (spec->sps.format)
    {
    case HLM_IMG_YUV_420:
        max_width[1]  = max_width[0] >> 1;
        max_height[1] = max_height[0] >> 1;
        break;
    case HLM_IMG_YUV_422:
        max_width[1]  = max_width[0] >> 1;
        max_height[1] = max_height[0];
        break;
    }
    max_width[2] = max_width[1];
    max_height[2] = max_height[1];

    // 申请DPB模块缓存
    sts = HLMD_DPB_GetMemSize(max_width, max_height, ability->ref_frm_num, &modu_status_size, &modu_work_size,
        spec->sps.mv_search_width, spec->sps.mv_search_height);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);
    total_status_size += modu_status_size;
    total_work_size += modu_work_size;

    // dec_pic_buf 计算内存
    modu_status_size = (ability->ref_frm_num + 1) * sizeof(HLMD_FRAME);
    modu_status_size = HLM_SIZE_ALIGN_64(modu_status_size);
    total_status_size += modu_status_size;

    *status_size = total_status_size;
    *work_size = total_work_size;

    return HLM_STS_OK;
}

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
                                    HLM_SZT        work_size)
{
    HLM_STATUS sts           = HLM_STS_ERR;
    HLMD_ABILITY *ability    = &spec->ability;
    HLM_U32 max_width[3]     = { ability->code_width[0],  ability->code_width[1],  ability->code_width[2] };
    HLM_U32 max_height[3]    = { ability->code_height[0], ability->code_height[1], ability->code_height[2] };
    HLM_U08 *left_status_buf = status_buf;
    HLM_U08 *left_work_buf   = work_buf;
    HLM_SZT modu_status_size = 0;           //单个模块所需状态内存大小
    HLM_SZT modu_work_size   = 0;           //单个模块所需状态工作大小
    HLM_SZT left_status_size = status_size; //剩余状态内存大小
    HLM_SZT left_work_size   = work_size;   //剩余工作内存大小
    HLM_S32 j                = 0;

    // DPB模块先判断内存是否足够，再创建
    sts = HLMD_DPB_GetMemSize(max_width, max_height, ability->ref_frm_num, &modu_status_size, &modu_work_size,
        spec->sps.mv_search_width, spec->sps.mv_search_height);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    //判断状体内存、工作内存是否足够
    HLM_CHECK_ERROR((left_status_size < modu_status_size), HLM_STS_ERR_MEM_LACK);
    HLM_CHECK_ERROR((left_work_size   < modu_work_size), HLM_STS_ERR_MEM_LACK);

    //内存足够，可以创建模块
    sts = HLMD_DPB_Create(max_width, max_height, ability->ref_frm_num, left_status_buf, left_work_buf,
        &(spec->dpb_handle), spec->sps.mv_search_width, spec->sps.mv_search_height);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    // 状态内存不可复用，需要扣除
    left_status_size -= modu_status_size;
    left_work_size -= modu_work_size;
    left_status_buf += modu_status_size;
    left_work_buf += modu_work_size;

    // dec_pic_buf 分配内存
    modu_status_size = (ability->ref_frm_num + 1) * sizeof(HLMD_FRAME);
    modu_status_size = HLM_SIZE_ALIGN_64(modu_status_size);
    spec->dec_pic_buf = (HLMD_FRAME *)left_status_buf;

    left_status_size -= modu_status_size;
    left_status_buf += modu_status_size;

    for (j = 0; j < (ability->ref_frm_num + 1); j++)
    {
        spec->dec_pic_buf[j].ref_pic_data = &((HLMD_DPB_SPEC *)(spec->dpb_handle))->pic_data[j];
    }

    return HLM_STS_OK;
}
