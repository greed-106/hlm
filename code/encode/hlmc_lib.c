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
#include "hlmc_lib.h"
#include "hlmc_defs.h"
#include "hlmc_init.h"
#include "hlmc_rc.h"
#include "hlmc_dpb.h"
#include "hlmc_ecd.h"
#include "hlmc_hls.h"
#include "hlmc_patch.h"

/***************************************************************************************************
* 功  能：获取编码算法模型所需存储信息
* 参  数：*
*         ability          -I    能力集参数指针
*         mem_tab          -O    存储空间参数结构体
*         coding_ctrl      -I    编码控制结构体
* 返回值：状态码
* 备  注：如果mtab[i].size为0，则不需要分配该块内存
***************************************************************************************************/
HLM_STATUS HLMC_LIB_GetMemSize(HLMC_ABILITY      *ability,
                               HLM_MEM_TAB        mem_tab[HLM_MEM_TAB_NUM],
                               HLMC_CODING_CTRL  *coding_ctrl)
{
    HLM_STATUS   sts                        = HLM_STS_ERR;
    HLM_SPEC     spec                       = {0};
    HLM_SZT      tmp_buf_size               = 0;
    HLM_SZT      ddr_persist_size           = 0;
    HLM_SZT      ddr_scratch_size           = 0;
    HLM_SZT      ram_scratch_size           = 0;
    HLM_SZT      modu_status_size           = 0;
    HLM_SZT      modu_work_size             = 0;
    HLM_MEM_TAB  mtab_tmp[HLM_MEM_TAB_NUM] = {0};
    HLM_MEM_TAB *mtab_ddr_persist           = HLM_NULL;
    HLM_MEM_TAB *mtab_ddr_scratch           = HLM_NULL;
    HLM_MEM_TAB *mtab_ram_scratch           = HLM_NULL;

    //参数检测
    HLM_CHECK_ERROR((HLM_NULL == ability) || (HLM_NULL == mem_tab), HLM_STS_ERR_NULL_PTR);

    sts = HLMC_INIT_CheckAbilityParam(ability);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    // 初始化局部变量
    ddr_persist_size = 0;
    ddr_scratch_size = 0;
    ram_scratch_size = 0;
    mtab_ddr_persist = &mtab_tmp[HLM_MEM_TAB_DDR_PERSIST];
    mtab_ddr_scratch = &mtab_tmp[HLM_MEM_TAB_DDR_SCRATCH];
    mtab_ram_scratch = &mtab_tmp[HLM_MEM_TAB_RAM_SCRATCH];

    // 初始化临时 spec
    memset(&spec, 0, sizeof(HLM_SPEC));
    memcpy(&spec.ability, ability, sizeof(HLMC_ABILITY));
    memcpy(&spec.coding_ctrl, coding_ctrl, sizeof(HLMC_CODING_CTRL));

    // 获取接口模块所需外部DDR不可复用的缓存大小
    mtab_ddr_persist->size      = HLM_MAX_MEM_SIZE;     // 虚拟分配足够大空间
    mtab_ddr_persist->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_persist->base      = (HLM_VOID *)&spec;

    sts = HLMC_INIT_AllocDdrPersistMem(&spec, mtab_ddr_persist, &tmp_buf_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    ddr_persist_size += tmp_buf_size;

    // 获取接口模块所需外部DDR可复用的缓存大小
    mtab_ddr_scratch->size      = HLM_MAX_MEM_SIZE;    // 虚拟分配足够大空间
    mtab_ddr_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_scratch->base      = (HLM_VOID *)&spec;

    // 获取接口模块所需内部RAM缓存大小
    mtab_ram_scratch->size      = HLM_MAX_MEM_SIZE;     // 虚拟分配足够大空间
    mtab_ram_scratch->alignment = HLM_MEM_ALIGN_16BYTE; // 在ASIC硬件中，RAM不需要字节对齐，但在C model中16字节对齐有利于软件实现
    mtab_ram_scratch->base      = (HLM_VOID *)&spec;

    sts = HLMC_INIT_AllocRamScratchMem(&spec, mtab_ram_scratch, &tmp_buf_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    ram_scratch_size += tmp_buf_size;

    //计算各模块所需的内存，包括状态内存、工作内存
    sts = HLMC_INIT_GetModuleBuf(&spec, &modu_status_size, &modu_work_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    ddr_persist_size += modu_status_size;
    ddr_scratch_size += modu_work_size;

    //总大小
    ddr_persist_size  = HLM_SIZE_ALIGN(ddr_persist_size, HLM_MEM_ALIGN_16BYTE);
    ddr_scratch_size  = HLM_SIZE_ALIGN(ddr_scratch_size, HLM_MEM_ALIGN_16BYTE);
    ram_scratch_size  = HLM_SIZE_ALIGN(ram_scratch_size, HLM_MEM_ALIGN_16BYTE);

    // 在硬件ASIC中，缓存大小是有限的，需要校验，在C Model中假设缓存大小上限为HLM_MAX_MEM_SIZE
    HLM_CHECK_ERROR((ddr_persist_size + ddr_scratch_size + ram_scratch_size) > HLM_MAX_MEM_SIZE, 
                    HLM_STS_ERR_OVER_MAX_MEM);

    mtab_ddr_persist            = &mem_tab[HLM_MEM_TAB_DDR_PERSIST];
    mtab_ddr_persist->size      = ddr_persist_size;
    mtab_ddr_persist->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_persist->space     = HLM_MEM_EXTERNAL_DDR;
    mtab_ddr_persist->attrs     = HLM_MEM_PERSIST;
    mtab_ddr_persist->base      = HLM_NULL;

    mtab_ddr_scratch            = &mem_tab[HLM_MEM_TAB_DDR_SCRATCH];
    mtab_ddr_scratch->size      = ddr_scratch_size;
    mtab_ddr_scratch->alignment = HLM_MEM_ALIGN_16BYTE;  // 在ASIC硬件中，ROM不需要字节对齐，但在C model中必须字节对齐
    mtab_ddr_scratch->space     = HLM_MEM_INTERNAL_ROM;
    mtab_ddr_scratch->attrs     = HLM_MEM_PERSIST;      // ROM缓存维持固定值，断电前不可擦除
    mtab_ddr_scratch->base      = HLM_NULL;

    mtab_ram_scratch            = &mem_tab[HLM_MEM_TAB_RAM_SCRATCH];
    mtab_ram_scratch->size      = ram_scratch_size;
    mtab_ram_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ram_scratch->space     = HLM_MEM_EXTERNAL_DDR;
    mtab_ram_scratch->attrs     = HLM_MEM_PERSIST;
    mtab_ram_scratch->base      = HLM_NULL;

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：创建编码算法模型实例,内存初始化
* 参  数：*
*         ability          -I    能力集参数指针
*         mem_tab          -O    存储空间参数结构体
*         handle           -O    编码实例句柄指针
*         coding_ctrl      -I    编码控制结构体
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_LIB_Create(HLMC_ABILITY     *ability,
                           HLM_MEM_TAB       mem_tab[HLM_MEM_TAB_NUM],
                           HLM_VOID        **handle,
                           HLMC_CODING_CTRL *coding_ctrl)
{
    HLM_STATUS   sts                        = HLM_STS_ERR;
    HLM_SPEC    *spec                       = HLM_NULL;
    HLM_MEM_TAB  mtab_tmp[HLM_MEM_TAB_NUM] = {0};
    HLM_MEM_TAB *mtab_ddr_persist           = HLM_NULL;
    HLM_MEM_TAB *mtab_ddr_scratch           = HLM_NULL;
    HLM_MEM_TAB *mtab_ram_scratch           = HLM_NULL;
    HLM_U08     *left_ddr_persist_buf       = HLM_NULL;
    HLM_U08     *left_ddr_scratch_buf       = HLM_NULL;
    HLM_U08     *left_ram_scratch_buf       = HLM_NULL;
    HLM_SZT      left_ddr_persist_size      = 0;
    HLM_SZT      left_ddr_scratch_size      = 0;
    HLM_SZT      left_ram_scratch_size      = 0;
    HLM_SZT      tmp_buf_size               = 0;

    // 参数检测
    HLM_CHECK_ERROR((HLM_NULL == ability) || (HLM_NULL == mem_tab) || (HLM_NULL == handle),
        HLM_STS_ERR_NULL_PTR);

    // 检查缓存表是否合法
    sts = HLM_MEM_CheckMemTab(mem_tab, (HLM_S32)HLM_MEM_TAB_NUM, HLM_MEM_ALIGN_16BYTE);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 检查配置参数是否在有效范围
    sts = HLMC_INIT_CheckAbilityParam(ability);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 初始化
    mtab_ddr_persist            = &mem_tab[HLM_MEM_TAB_DDR_PERSIST];
    mtab_ddr_persist->alignment = HLM_MEM_ALIGN_16BYTE;
    left_ddr_persist_size       = mtab_ddr_persist->size;
    left_ddr_persist_buf        = (HLM_U08 *)mtab_ddr_persist->base;

    mtab_ddr_scratch            = &mem_tab[HLM_MEM_TAB_DDR_SCRATCH];
    mtab_ddr_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    left_ddr_scratch_size       = mtab_ddr_scratch->size;
    left_ddr_scratch_buf        = (HLM_U08 *)mtab_ddr_scratch->base;

    mtab_ram_scratch            = &mem_tab[HLM_MEM_TAB_RAM_SCRATCH];
    mtab_ram_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    left_ram_scratch_size       = mtab_ram_scratch->size;
    left_ram_scratch_buf        = (HLM_U08 *)mtab_ram_scratch->base;

    // 分配spec参数实例
    tmp_buf_size = sizeof(HLM_SPEC);
    tmp_buf_size = HLM_SIZE_ALIGN_16(tmp_buf_size);
    HLM_CHECK_ERROR(left_ddr_persist_size < tmp_buf_size, HLM_STS_ERR_MEM_LACK);
    spec = (HLM_SPEC *)(left_ddr_persist_buf); //该结构体大小在alloc persist里面分配，不用扣除大小

    // 初始化spec结构体参数
    memset(spec, 0, sizeof(HLM_SPEC));
    memcpy(&spec->ability, ability, sizeof(HLMC_ABILITY));
    memcpy(&spec->coding_ctrl, coding_ctrl, sizeof(HLMC_CODING_CTRL));

    // 分配接口模块所需外部DDR不可复用的缓存大小
    mtab_ddr_persist            = &mtab_tmp[HLM_MEM_TAB_DDR_PERSIST];
    mtab_ddr_persist->size      = left_ddr_persist_size;     // 虚拟分配足够大空间
    mtab_ddr_persist->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_persist->base      = left_ddr_persist_buf;

    sts = HLMC_INIT_AllocDdrPersistMem(spec, mtab_ddr_persist, &tmp_buf_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    left_ddr_persist_size -= tmp_buf_size;
    left_ddr_persist_buf  += tmp_buf_size;

    // 分配接口模块所需外部DDR可复用的缓存大小
    mtab_ddr_scratch            = &mtab_tmp[HLM_MEM_TAB_DDR_SCRATCH];
    mtab_ddr_scratch->size      = left_ddr_scratch_size;
    mtab_ddr_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_scratch->base      = left_ddr_scratch_buf;

    // 分配接口模块所需内部RAM缓存
    mtab_ram_scratch            = &mtab_tmp[HLM_MEM_TAB_RAM_SCRATCH];
    mtab_ram_scratch->size      = left_ram_scratch_size;
    mtab_ram_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ram_scratch->base      = left_ram_scratch_buf;

    sts = HLMC_INIT_AllocRamScratchMem(spec, mtab_ram_scratch, &tmp_buf_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    //left_ram_scratch_size -= tmp_buf_size;
    //left_ram_scratch_buf  += tmp_buf_size;

    //分配子模块所需内存，并创建子模块
    sts = HLMC_INIT_AllocModuleBuf(spec, left_ddr_persist_buf, left_ddr_persist_size,
        left_ddr_scratch_buf, left_ddr_scratch_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    //输出实例句柄
    *handle = (HLM_VOID *)spec;

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：设置序列级编码控制参数
* 参  数：*
*         handle       -O  编码实例句柄指针
*         code_params  -I   编码模型配置参数
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_LIB_SetCodingCtrl(HLM_VOID          *handle,
                                  HLMC_CODING_CTRL  *code_params)
{
    HLM_STATUS        sts         = HLM_STS_ERR;
    HLM_SPEC         *spec        = HLM_NULL;
    HLMC_CODING_CTRL *coding_ctrl = HLM_NULL;

    // 参数检测
    sts = HLMC_INIT_CheckCodingCtrl(handle, code_params);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 初始化
    spec        = (HLM_SPEC *)handle;
    coding_ctrl = &(spec->coding_ctrl);

    memcpy(coding_ctrl, code_params, sizeof(HLMC_CODING_CTRL));

    // 其他工作参数，由上述配置参数推导而得
    spec->cu_cols = HLM_SIZE_ALIGN_16(coding_ctrl->width) >> HLM_LOG2_WIDTH_SIZE;
    spec->cu_rows = HLM_SIZE_ALIGN_8(coding_ctrl->height) >> HLM_LOG2_HEIGHT_SIZE;

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：设置DPB及参考结构参数
* 参  数：*
*         handle        -O   编码实例句柄指针
*         dpb_params    -I   DPB结构和参考关系控制参数
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_LIB_SetDpbRefCtrl(HLM_VOID           *handle,
                                  HLMC_DPB_REF_CTRL  *dpb_params)
{
    HLM_STATUS  sts     = HLM_STS_ERR;
    HLM_SPEC   *spec    = (HLM_SPEC *)handle;
    HLM_U32     dpb_num = 0;

    // 参数检测
    HLM_CHECK_ERROR((HLM_NULL == handle) || (HLM_NULL == dpb_params), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((dpb_params->base_period < 1), HLM_STS_ERR_PARAM_VALUE);
    HLM_CHECK_ERROR((dpb_params->reffed_enable > 1), HLM_STS_ERR_PARAM_VALUE);

    memcpy(&spec->dpb_ref_ctrl, dpb_params, sizeof(HLMC_DPB_REF_CTRL));
    sts = HLMC_DPB_SetDpbRefCtrl(spec->dpb_handle, dpb_params, &dpb_num);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：编码一帧图像
* 参  数：*
*         handle    -I  编码实例句柄指针
*         in_buf    -I  编码模型输入参数地址
*         in_size   -I  编码模型输入参数大小
*         out_buf   -O  编码模型输出参数地址
*         out_size  -I  编码模型输出参数大小
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_LIB_EncodeFrame(HLM_VOID *handle,
                                HLM_VOID *in_buf,
                                HLM_SZT   in_size,
                                HLM_VOID *out_buf,
                                HLM_SZT   out_size)
{
    HLM_STATUS              sts             = HLM_STS_ERR;
    HLMC_PROCESS_IN        *input           = HLM_NULL;
    HLMC_PROCESS_OUT       *output          = HLM_NULL;
    HLM_SPEC               *spec            = HLM_NULL;
    HLM_PATCH_HEADER       *patch_ctx       = HLM_NULL;
    HLMC_BITSTREAM         *bs              = HLM_NULL;
    HLMC_BITSTREAM         *out_bs          = HLM_NULL;
    HLM_S32                 poc_out         = 0;
    HLMC_PATCH_REF_TYPE     patch_type      = HLMC_BASE_IDRPATCH;
    static HLMC_REGS        regs            = { 0 };
    VENC_RATE_CTRL_OUT_REGS rc_out_regs     = { 0 };
    HLM_S32                 ref_dpb_idx     = 0;
    HLM_S32                 rec_dpb_idx     = 0;
    HLM_U32                 sps_pps_bits    = 0;  // 记录SPS和PPS的比特数

    // 参数检测
    sts = HLMC_INIT_CheckPrcIOParam(handle, in_buf, in_size, out_buf, out_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 参数初始化
    spec   = (HLM_SPEC *)handle;
    input  = (HLMC_PROCESS_IN *)in_buf;
    output = (HLMC_PROCESS_OUT *)out_buf;
    bs     = &(spec->bs);
    out_bs = &(spec->out_bs);

    // 初始化四个码流信息结构体
    HLMC_ECD_InitBitstream(input->stream_buf, input->stream_buf_size, bs);
    HLMC_ECD_InitBitstream(input->out_stream_buf, input->stream_buf_size, out_bs);

    sts = HLMC_DPB_Get(spec->dpb_handle, spec->poc, input->force_idr, &poc_out,
        &ref_dpb_idx, &rec_dpb_idx, &patch_type);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 获取帧级QP
    sts = HLMC_RC_Process(spec->rc_handle, patch_type, &rc_out_regs, spec->coding_ctrl.bitdepth);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 如果是IDR帧，则需要产生SPS
    if (HLMC_BASE_IDRPATCH == patch_type)
    {
        HLMC_HLS_InitSeqHeader(&(spec->coding_ctrl), &(spec->sps), &((HLMC_RC_SPEC *)spec->rc_handle)->rate_ctrl);
        HLMC_HLS_GeneratSeqHeader(&(spec->sps), bs);
        HLMC_ECD_WriteEmulaPreventBytes(bs, out_bs);
        sps_pps_bits += bs->bit_cnt;
    }

    // 编码图像头
    bs->ptr_start = bs->ptr;
    bs->bit_cnt = 0;
    HLMC_HLS_InitPicHeader(&(spec->coding_ctrl), patch_type, spec->poc, rc_out_regs.reg_patch_qp, &(spec->pps));
    HLMC_HLS_GeneratPicHeader(&(spec->pps), &(spec->sps), bs);
    HLMC_ECD_WriteEmulaPreventBytes(bs, out_bs);
    sps_pps_bits += bs->bit_cnt;

    // 配置寄存器
    bs->ptr_start = bs->ptr;
    bs->bit_cnt = sps_pps_bits;  // 在计算码流缓冲区的剩余空间时，将已经用掉的sps_pps_bits扣除
    sts = HLMC_HLS_WriteReg(spec, input, rec_dpb_idx, ref_dpb_idx, patch_type, &regs, &rc_out_regs);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 从寄存器获取SDK配置给ASIC的编码参数
    HLMC_HLS_InitSpec(&regs, spec);
    patch_ctx = &(spec->patch_ctx);

    // 避免每个patch重复开辟内存
    sts = HLMC_HLS_AllocRam(&(spec->ram_buf_pic), &(spec->cur_cu), &(spec->best_cu), &(spec->nbi_info), &regs);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    do
    {
        HLMC_PATCH_Init(spec, patch_ctx);
        HLMC_PATCH_Padding(spec, patch_ctx);
        HLMC_PATCH_Encode(spec, patch_ctx);
        HLMC_PATCH_WriteRec(spec, patch_ctx);
        patch_ctx->patch_idx++;
    } while (patch_ctx->patch_idx < spec->sps.patch_info.patch_num);

    // 从寄存器获取编码结果
    HLMC_HLS_ReadReg(spec, output, &regs, sps_pps_bits);

    // 将MD5写入SEI中
    bs->ptr_start = bs->ptr;
    bs->bit_cnt = 0;
    HLMC_HLS_GeneratSEI(&regs, bs);
    HLMC_ECD_WriteEmulaPreventBytes(bs, out_bs);
    output->sei_len = bs->bit_cnt >> 3;

    // 更新状态参数
    spec->frame_num++;
    spec->poc++;
    if (spec->poc == spec->dpb_ref_ctrl.intra_period)
    {
        spec->poc = 0;
    }

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：码率控制模块初始化
* 参  数：*
*         lib_handle         -IO       算法模型句柄
*         pic_width          -I        图像宽
*         pic_height         -I        图像高
*         rate_ctrl          -I        码控参数
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_LIB_InitRc(HLM_VOID        *lib_handle,
                         HLM_U32          pic_width,
                         HLM_U32          pic_height,
                         HLMC_RATE_CTRL  *rate_ctrl)
{
    HLM_SPEC *hlm_spec    = (HLM_SPEC *)lib_handle;
    HLMC_RC_SPEC *rc_spec = (HLMC_RC_SPEC*)hlm_spec->rc_handle;
    HLM_S32 bitdepth      = hlm_spec->coding_ctrl.bitdepth;
    HLM_S64 bpp           = 0;
    HLM_S64 rate          = 0;

    memcpy(&rc_spec->rate_ctrl, rate_ctrl, sizeof(HLMC_RATE_CTRL));
    rc_spec->rate_ctrl.rc_cbr_ctrl.qp_min   = 4;
    rc_spec->rate_ctrl.rc_cbr_ctrl.qp_max   = HLM_MAX_QP(bitdepth);
    rc_spec->rate_ctrl.rc_cbr_ctrl.qp_min_i = 4;
    rc_spec->rate_ctrl.rc_cbr_ctrl.qp_max_i = HLM_MAX_QP(bitdepth);

    // I帧
    bpp = rate_ctrl->rc_cbr_ctrl.bpp_i;
    rate = (bpp * pic_width * pic_height + 8) >> 4;
    rc_spec->m_targetRate_i = (HLM_S32)rate;

    // P帧
    bpp = rate_ctrl->rc_cbr_ctrl.bpp_p;
    rate = (bpp * pic_width * pic_height + 8) >> 4;
    rc_spec->m_targetRate_p = (HLM_S32)rate;
}
