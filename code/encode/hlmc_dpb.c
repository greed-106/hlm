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
#include "hlmc_dpb.h"

// 模块所需状态内存计算和分配
HLM_STATUS HLMC_DPB_alloc_status_buffer(HLM_S32         width,
                                        HLM_S32         height,
                                        HLM_S32         max_ref_num,
                                        HLMC_DPB_SPEC  *spec,
                                        HLM_U08        *status_buf,
                                        HLM_SZT        *status_size,
                                        HLM_S32         mv_search_width,
                                        HLM_S32         mv_search_height)
{
    HLM_S32 i                 = 0;
    HLM_SZT size              = 0;
    HLM_SZT used_size         = 0;
    HLM_U08 *acc_buf          = HLM_NULL;
    HLM_S32 inter_pad_w_left  = mv_search_width >> 1;
    HLM_S32 inter_pad_w_right = mv_search_width - 1 - inter_pad_w_left;
    HLM_S32 inter_pad_h_up    = mv_search_height >> 1;
    HLM_S32 inter_pad_h_down  = mv_search_height - 1 - inter_pad_h_up;

    // 真正需要申请status_buf时才需要下面这句校验
    HLM_CHECK_ERROR((HLM_NULL == status_buf), HLM_STS_ERR_NULL_PTR);

    acc_buf = status_buf;

    //跳过spec结构体，在外面已分配，只计算内存大小
    size = sizeof(HLMC_DPB_SPEC);
    size = HLM_SIZE_ALIGN_16(size);
    used_size += size;
    acc_buf += size;

    for (i = 0; i < (max_ref_num + 1); i++)
    {
        spec->dpb[i].data[0] = (HLM_U16 *)acc_buf;
        spec->dpb[i].step[0] = HLM_SIZE_ALIGN_16(width);
        size = (HLM_S32)(spec->dpb[i].step[0] * HLM_SIZE_ALIGN_8(height) * sizeof(HLM_U16));
        size = HLM_SIZE_ALIGN_64(size);
        used_size += size;
        acc_buf += size;

        // yuv 444
        spec->dpb[i].data[1] = (HLM_U16 *)acc_buf;
        spec->dpb[i].step[1] = HLM_SIZE_ALIGN_16(width);
        used_size += size;
        acc_buf += size;

        spec->dpb[i].data[2] = (HLM_U16 *)acc_buf;
        spec->dpb[i].step[2] = spec->dpb[i].step[1];
        used_size += size;    // size与u分量相同
        acc_buf += size;

        // padding后的参考图像缓冲区，目前色度和亮度padding的像素数量相同
        spec->dpb[i].luma_ref_padding_y = (HLM_U16 *)acc_buf;
        size = (width + (inter_pad_w_left + inter_pad_w_right)) *
            (height + (inter_pad_h_up + inter_pad_h_down)) * sizeof(HLM_U16);
        size = HLM_SIZE_ALIGN_64(size);
        used_size += size;
        acc_buf += size;

        spec->dpb[i].luma_ref_padding_cb = (HLM_U16 *)acc_buf;
        used_size += size;
        acc_buf += size;

        spec->dpb[i].luma_ref_padding_cr = (HLM_U16 *)acc_buf;
        used_size += size;
        acc_buf += size;
    }

    *status_size = used_size;

    return HLM_STS_OK;
}

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
                               HLM_S32    mv_search_height)
{
    HLM_STATUS      sts         = HLM_STS_ERR;
    HLMC_DPB_SPEC   spec        = { 0 };
    HLM_SZT         status_sz   = 0;
    HLM_SZT         work_sz     = 0;
    HLM_U08        *buf         = (HLM_U08 *)&spec;
    HLM_U08         max_ref_num = 1;

    HLM_CHECK_ERROR((width < HLM_IMG_WIDTH_MIN) || (height < HLM_IMG_HEIGHT_MIN), HLM_STS_ERR_DATA_SIZE);
    HLM_CHECK_ERROR((width & 0x1) || (height & 0x1), HLM_STS_ERR_DATA_SIZE);
    HLM_CHECK_ERROR((HLM_NULL == status_size), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((HLM_NULL == work_size), HLM_STS_ERR_NULL_PTR);

    memset(&spec, 0, sizeof(HLMC_DPB_SPEC));
    spec.max_width = width;
    spec.max_height = height;
    spec.max_ref_num = max_ref_num;

    sts = HLMC_DPB_alloc_status_buffer(width, height, max_ref_num, &spec, buf, &status_sz, mv_search_width, mv_search_height);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    if ((status_sz + work_sz) > HLM_MAX_MEM_SIZE)
    {
        return HLM_STS_ERR_OVER_MAX_MEM;
    }

    *status_size = status_sz;
    *work_size = work_sz;

    return HLM_STS_OK;
}

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
                           HLM_S32    mv_search_height)
{
    HLM_STATUS      sts         = HLM_STS_ERR;
    HLMC_DPB_SPEC  *spec        = HLM_NULL;
    HLM_SZT         status_size = 0;
    HLM_SZT         work_size   = 0;
    HLM_S32         max_ref_num = 1;

    HLM_CHECK_ERROR((width < HLM_IMG_WIDTH_MIN) || (height < HLM_IMG_HEIGHT_MIN), HLM_STS_ERR_DATA_SIZE);
    HLM_CHECK_ERROR((width & 0x1) || (height & 0x1), HLM_STS_ERR_DATA_SIZE);
    HLM_CHECK_ERROR((HLM_NULL == handle), HLM_STS_ERR_NULL_PTR);

    // 初始化
    spec = (HLMC_DPB_SPEC *)status_buf;
    memset(spec, 0, sizeof(HLMC_DPB_SPEC));
    spec->max_width = width;
    spec->max_height = height;
    spec->max_ref_num = max_ref_num;
    spec->max_dpb_num = max_ref_num + 1;

    // 分配内存
    sts = HLMC_DPB_alloc_status_buffer(width, height, max_ref_num, spec, status_buf, &status_size, mv_search_width, mv_search_height);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    *handle = (HLM_VOID *)spec;

    return HLM_STS_OK;
}

// 获取当前帧重构图像的缓冲区地址
HLM_STATUS HLMC_DPB_get_curr_frame(HLM_VOID          *handle,
                                   HLM_U32            poc,
                                   HLM_U32            dpb_id,
                                   HLM_S32           *dpb_idx)
{
    HLMC_DPB_SPEC *dpb_spec = (HLMC_DPB_SPEC *)handle;
    HLM_S32 i               = 0;
    HLMC_FRAME *frame       = HLM_NULL;

    //分配一帧空闲帧
    for (i = 0; i < dpb_spec->max_dpb_num; i++)
    {
        frame = &dpb_spec->dpb[i];
        if (!frame->refed_flag)
        {
            *dpb_idx = i;
            break;
        }
    }

    HLM_CHECK_ERROR(i >= dpb_spec->max_dpb_num, HLM_STS_ERR);

    frame->poc = poc;
    frame->dpb_id = dpb_id;
    frame->refed_flag = 1;

    return HLM_STS_OK;
}

// 根据参考帧的poc值找到对应在DPB中的位置
HLM_STATUS HLMC_DPB_find_ref_pic(HLMC_DPB_SPEC  *dpb_spec,
                                 HLM_S32        *dpb_idx,
                                 HLM_U32         poc,
                                 HLM_U32         dpb_id)
{
    HLMC_FRAME *frame  = HLM_NULL;
    HLM_S32 i          = 0;
    HLM_U08 refed_flag = 0;

    *dpb_idx = -1;
    for (i = 0; i < dpb_spec->max_dpb_num; i++)
    {
        frame = &dpb_spec->dpb[i];
        refed_flag = frame->refed_flag;
        if ((frame->dpb_id == dpb_id) && (frame->poc == poc) && refed_flag)
        {
            *dpb_idx = i;
            break;
        }
    }

    // Can't find the ref picture.
    HLM_CHECK_ERROR(-1 == *dpb_idx, HLM_STS_ERR);

    return HLM_STS_OK;
}

// 获得待编码的patch类型
HLM_STATUS HLMC_DPB_get_patch_type(HLM_VOID                    *handle,
                                   HLM_S32                      poc,
                                   HLM_S32                      intra_period,
                                   HLM_S32                      force_idr,
                                   HLMC_PATCH_REF_TYPE         *patch_type)
{
    HLMC_PATCH_REF_TYPE   type     = HLMC_BASE_IDRPATCH;
    HLMC_DPB_SPEC        *dpb_spec = (HLMC_DPB_SPEC *)handle;
    HLMC_DPB_REF_CTRL    *dpb_ctrl = &(dpb_spec->dpb_ctrl);

    if (!(poc % intra_period) || force_idr)
    {
        type = HLMC_BASE_IDRPATCH;
    }
    else
    {
        type = HLMC_BASE_PPATCH;
    }
    *patch_type = type;

    return HLM_STS_OK;
}

// 初始化RPS中的参考队列
HLM_STATUS HLMC_DPB_get_ref_list(HLM_VOID             *handle,
                                 HLM_S32               poc_curr,
                                 HLM_U32               dpb_id,
                                 HLM_S32              *dpb_index,
                                 HLMC_PATCH_REF_TYPE   patch_type)
{
    HLM_STATUS sts              = HLM_STS_ERR;
    HLMC_DPB_SPEC *dpb_spec     = (HLMC_DPB_SPEC *)handle;
    HLMC_DPB_REF_CTRL *dpb_ctrl = &(dpb_spec->dpb_ctrl);
    HLM_U32 frame_num_reg       = 0;
    HLM_U32 idr_pic_id_reg      = 0;
    HLM_S32 num_ref_idx_active  = 0;
    HLM_S32 nb_list             = 1;  // 只支持P帧
    HLM_S32 poc_ref             = 0;

    if (patch_type != HLMC_BASE_IDRPATCH)
    {
        poc_ref = poc_curr - 1;
        sts = HLMC_DPB_find_ref_pic(dpb_spec, dpb_index, poc_ref, dpb_id);
        HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    }

    return HLM_STS_OK;
}

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
                                  HLM_U32              *dpb_num)
{
    HLMC_DPB_SPEC *spec = (HLMC_DPB_SPEC *)handle;

    HLM_CHECK_ERROR((HLM_NULL == handle) || (HLM_NULL == dpb_params), HLM_STS_ERR_NULL_PTR);

    memcpy(&(spec->dpb_ctrl), dpb_params, sizeof(HLMC_DPB_REF_CTRL));
    *dpb_num = 2;
    spec->max_dpb_num = *dpb_num;

    return HLM_STS_OK;
}

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
                        HLMC_PATCH_REF_TYPE    *patch_type)
{
    HLMC_DPB_SPEC *dpb_spec       = (HLMC_DPB_SPEC *)handle;
    HLM_STATUS sts                = HLM_STS_ERR;
    HLM_S32 i                     = 0;
    HLMC_FRAME *frame             = HLM_NULL;

    // 获取帧类型
    sts = HLMC_DPB_get_patch_type(handle, poc_in, dpb_spec->dpb_ctrl.intra_period, force_idr, patch_type);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    HLM_CHECK_ERROR(*patch_type > HLMC_BASE_PPATCH, sts);

    // I帧时清空DPB
    if (HLMC_BASE_IDRPATCH == *patch_type)
    {
        for (i = 0; i < dpb_spec->max_dpb_num; i++)
        {
            frame = &dpb_spec->dpb[i];
            frame->refed_flag = 0;
            frame->poc = 0;
        }
    }

    // 构建参考列表
    sts = HLMC_DPB_get_ref_list(handle, poc_in, 0, ref_idx, *patch_type);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    //释放掉无用参考帧
    frame = &dpb_spec->dpb[poc_in % 2];
    frame->refed_flag = 0;

    //从DPB中获取一帧空间存放当前帧的重构帧
    sts = HLMC_DPB_get_curr_frame(handle, poc_in, 0, rec_idx);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    *poc_out = poc_in + 1;
    if (*poc_out == dpb_spec->dpb_ctrl.intra_period)  //I帧POC置零
    {
        *poc_out = 0;
    }

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：获取DPB参考关系控制参数
* 参  数：*
*         handle        -I     DPB模块句柄指针
*         dpb_params    -O     DPB结构和参考关系控制参数
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_DPB_GetDpbRefCtrl(HLM_VOID              *handle,
                                  HLMC_DPB_REF_CTRL     *dpb_params)
{
    HLMC_DPB_SPEC *spec = (HLMC_DPB_SPEC *)handle;

    HLM_CHECK_ERROR((HLM_NULL == handle), HLM_STS_ERR_NULL_PTR);
    memcpy(dpb_params, &(spec->dpb_ctrl), sizeof(HLMC_DPB_REF_CTRL));

    return HLM_STS_OK;
}
