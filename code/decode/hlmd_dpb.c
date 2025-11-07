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
#include "hlmd_dpb.h"

// 产生默认参考列表
HLM_STATUS HLMD_DPB_get_defult_ref_list(HLM_S32           *dpb_free_index,
                                        HLMD_FRAME       **short_ref_list,
                                        HLM_U32            short_ref_cnt,
                                        HLM_U32            patch_type,
                                        HLM_S32            img_step,
                                        HLM_U32            ref_count,
                                        HLMD_FRAME        *curr_frame,
                                        HLMD_REF_LIST      ref_list[2 * HLMD_MAX_REF_NUM])
{
    HLMD_REF_LIST  *frame_list = ref_list;
    HLM_U32         index      = 0;
    HLM_S32         sel        = 0;
    HLM_S32         short_len  = 0;
    HLM_U32         i          = 0;
    HLM_STATUS      sts        = HLM_STS_ERR;

    // 构建参考队列
    for (i = 0; i < short_ref_cnt; i++)
    {
        frame_list[index].ref_pic_data = *(short_ref_list[i]->ref_pic_data);
    }
    short_len = index;

    //找到dpb中非当前帧的缓存
    if (*dpb_free_index > 0)
    {
        *dpb_free_index--;
    }
    else
    {
        *dpb_free_index++;
    }
    for (i = 0; i < HLM_MAX(index, ref_count); i++)
    {
        if (HLM_NULL == ref_list[i].ref_pic_data.yuv.data[0])
        {
            ref_list[i].ref_pic_data = *(curr_frame->ref_pic_data);
        }
    }

    return HLM_STS_OK;
}

// 移除一个短期参考帧
HLM_VOID HLMD_DPB_remove_short_ref(HLM_U32      *short_ref_cnt,
                                   HLMD_FRAME  **short_ref_list,
                                   HLM_S32       index)
{
    HLM_S32      ref_cnt             = *short_ref_cnt;
    HLMD_FRAME **short_ref_list_pre  = short_ref_list + index;
    HLMD_FRAME **short_ref_list_post = short_ref_list + index + 1;

    short_ref_list[index] = HLM_NULL;
    ref_cnt = (ref_cnt < 2) ? 0 : (ref_cnt - 1);
    if (ref_cnt > index)
    {
        memmove(short_ref_list_pre, short_ref_list_post, (ref_cnt - index) * sizeof(HLMD_FRAME*));
        memset(&short_ref_list[ref_cnt], 0, (HLMD_MAX_REF_NUM - ref_cnt) * sizeof(HLMD_FRAME*));
    }
    *short_ref_cnt = ref_cnt;
}

// 当前帧入队列
HLM_STATUS HLMD_DPB_put_curr_frame_to_ref_list(HLMD_FRAME   *curr_frame,
                                               HLM_U32      *short_ref_cnt,
                                               HLMD_FRAME  **short_ref_list)
{
    HLM_U32 i = 0;

    for (i = 0; i < *short_ref_cnt; i++)
    {
        if (short_ref_list[i] == curr_frame)
        {
            return HLM_STS_ERR;
        }
    }
    if (*short_ref_cnt)
    {
        memmove(&short_ref_list[1], &short_ref_list[0], *short_ref_cnt * sizeof(HLMD_FRAME*));
    }

    short_ref_list[0] = curr_frame;
    (*short_ref_cnt)++;

    return HLM_STS_OK;
}

// 获取所需状态内存大小
HLM_VOID HLMD_DPB_alloc_status_buf(HLM_S32         width[3],
                                   HLM_S32         height[3],
                                   HLM_S32         max_ref_num,
                                   HLM_U08        *status_buf,
                                   HLM_SZT        *status_size,
                                   HLMD_DPB_SPEC  *dpb_spec,
                                   HLM_S32         mv_search_width,
                                   HLM_S32         mv_search_height)
{
    HLM_S32 i                 = 0;
    HLM_SZT size              = 0;
    HLM_SZT used_size         = 0;
    HLM_S32 cu_width          = (width[0] >> HLM_LOG2_WIDTH_SIZE);
    HLM_S32 cu_height         = (height[0] >> HLM_LOG2_HEIGHT_SIZE);
    HLM_S32 inter_pad_w_left  = mv_search_width >> 1;
    HLM_S32 inter_pad_w_right = mv_search_width - 1 - inter_pad_w_left;
    HLM_S32 inter_pad_h_up    = mv_search_height >> 1;
    HLM_S32 inter_pad_h_down  = mv_search_height - 1 - inter_pad_h_up;

    // 跳过spec结构体，在外面已分配，只计算内存大小
    size = sizeof(HLMD_DPB_SPEC);
    size = HLM_SIZE_ALIGN_64(size);
    used_size += size;
    status_buf += size;

    // 本算法模型默认参考帧内部管理
    for (i = 0; i < max_ref_num; i++)
    {
        // y（按照YUV444申请）
        dpb_spec->pic_data[i].yuv.data[0] = (HLM_U16 *)status_buf;
        dpb_spec->pic_data[i].yuv.step[0] = (width[0]);
        size = width[0] * height[0] * sizeof(HLM_U16);
        size = HLM_SIZE_ALIGN_64(size);
        used_size += size;
        status_buf += size;

        // u
        dpb_spec->pic_data[i].yuv.data[1] = (HLM_U16 *)status_buf;
        dpb_spec->pic_data[i].yuv.step[1] = (width[1]);
        used_size += size;
        status_buf += size;

        // v
        dpb_spec->pic_data[i].yuv.data[2] = (HLM_U16 *)status_buf;
        dpb_spec->pic_data[i].yuv.step[2] = (width[2]);
        used_size += size;
        status_buf += size;

        // padding后的参考图像缓冲区，目前色度和亮度padding的像素数量相同
        dpb_spec->pic_data[i].luma_ref_padding_y = (HLM_U16 *)status_buf;
        size = (width[0] + inter_pad_w_left + inter_pad_w_right)
             * (height[0] + inter_pad_h_up + inter_pad_h_down) * sizeof(HLM_U16);
        size = HLM_SIZE_ALIGN_64(size);
        used_size += size;
        status_buf += size;

        // u
        dpb_spec->pic_data[i].luma_ref_padding_cb = (HLM_U16 *)status_buf;
        used_size += size;
        status_buf += size;

        // v
        dpb_spec->pic_data[i].luma_ref_padding_cr = (HLM_U16 *)status_buf;
        used_size += size;
        status_buf += size;

        // 申请padding后的patch缓冲区，stride同yuv.step
        dpb_spec->pic_data[i].patch_padding_recon[0] = (HLM_U16 *)status_buf;
        size = width[0] * height[0] * sizeof(HLM_U16);
        size = HLM_SIZE_ALIGN_64(size);
        used_size += size;
        status_buf += size;

        // u
        dpb_spec->pic_data[i].patch_padding_recon[1] = (HLM_U16 *)status_buf;
        used_size += size;
        status_buf += size;

        // v
        dpb_spec->pic_data[i].patch_padding_recon[2] = (HLM_U16 *)status_buf;
        used_size += size;
        status_buf += size;

        // 申请cu_type内存
        dpb_spec->pic_data[i].cu_type_mem = (HLM_U16 *)status_buf;
        size = cu_width * cu_height * sizeof(HLM_U16);
        size = HLM_SIZE_ALIGN_64(size);
        used_size += size;
        status_buf += size;

        // 申请mv内存
        dpb_spec->pic_data[i].mv = (HLM_S16 *)status_buf;
        size = cu_width * cu_height * HLMD_MV_NUM_PER_CU * 2 * sizeof(HLM_S16);
        size = HLM_SIZE_ALIGN_64(size);
        used_size += size;
        status_buf += size;

        // 申请ref_idx内存
        dpb_spec->pic_data[i].ref_idx = (HLM_S08 *)status_buf;
        size = cu_width * cu_height * HLMD_REF_IDX_NUM_PER_CU * sizeof(HLM_S08);
        size = HLM_SIZE_ALIGN_64(size);
        used_size += size;
        status_buf += size;
    }

    *status_size = used_size;
}

/***************************************************************************************************
* 功  能：DPB模块初始化函数
* 参  数：*
*         width                   -I    解码图像宽度
*         height                  -I    解码图像高度
*         max_ref_num             -I    最大参考帧数量
*         dpb_spec                -I    DPB模块句柄
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMD_DPB_init(HLM_S32         width[3],
                       HLM_S32         height[3],
                       HLM_S32         max_ref_num,
                       HLMD_DPB_SPEC  *dpb_spec)
{
    HLM_S32   i         = 0;
    HLM_SZT   size      = 0;
    HLM_S32   cu_width  = (width[0] >> HLM_LOG2_WIDTH_SIZE);
    HLM_S32   cu_height = (height[0] >> HLM_LOG2_HEIGHT_SIZE);
    HLM_VOID *tmp_buf   = HLM_NULL;

    for (i = 0; i < max_ref_num; i++)
    {
        // 初始化cu_type内存
        size = cu_width * cu_height * sizeof(HLM_U16);
        size = HLM_SIZE_ALIGN_64(size);
        tmp_buf = (HLM_VOID *)(dpb_spec->pic_data[i].cu_type_mem);
        memset(tmp_buf, 0, size);

        // 前向ref_idx初始化
        size = cu_width * cu_height * HLMD_REF_IDX_NUM_PER_CU * sizeof(HLM_S08);
        size = HLM_SIZE_ALIGN_64(size);
        tmp_buf = (HLM_VOID *)(dpb_spec->pic_data[i].ref_idx);
        memset(tmp_buf, 0, size);

        // 非指针变量初始化
        dpb_spec->pic_num = max_ref_num;

        // DPB中YUV不进行padding
        dpb_spec->pic_data[i].yuv.step[0] = width[0];
        dpb_spec->pic_data[i].yuv.step[1] = width[1];
        dpb_spec->pic_data[i].yuv.step[2] = width[2];
    }
}

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
                               HLM_S32  mv_search_height)
{
    HLMD_DPB_SPEC   dpb_spec   = { 0 };
    HLM_U08        *status_buf = (HLM_U08 *)&dpb_spec;
    HLM_U08        *work_buf   = (HLM_U08 *)&dpb_spec;

    HLM_CHECK_ERROR((HLM_NULL == status_size) || (HLM_NULL == work_size), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((max_ref_num < 0) || (width[0] < HLM_IMG_WIDTH_MIN) || (height[0] < HLM_IMG_HEIGHT_MIN), HLM_STS_ERR_PARAM_VALUE);

    // 计算所需状态内存
    HLMD_DPB_alloc_status_buf(width, height, max_ref_num + 1, status_buf, status_size,
        &dpb_spec, mv_search_width, mv_search_height);

    *work_size = 0;

    return HLM_STS_OK;
}

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
                           HLM_S32    mv_search_height)
{
    HLM_SZT        temp_status_size = 0;
    HLM_SZT        temp_work_size   = 0;
    HLMD_DPB_SPEC *dpb_spec         = (HLMD_DPB_SPEC *)status_buf;

    HLM_CHECK_ERROR((HLM_NULL == handle), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((max_ref_num < 0) || (width[0] < HLM_IMG_WIDTH_MIN) || (height[0] < HLM_IMG_HEIGHT_MIN), HLM_STS_ERR_DATA_SIZE);
    HLM_CHECK_ERROR((width[0] & 0x1) || (height[0] & 0x1), HLM_STS_ERR_DATA_SIZE);

    memset(dpb_spec, 0, sizeof(HLMD_DPB_SPEC));

    // 计算所需状态内存
    HLMD_DPB_alloc_status_buf(width, height, max_ref_num + 1, status_buf, &temp_status_size,
        dpb_spec, mv_search_width, mv_search_height);

    // 初始化
    HLMD_DPB_init(width, height, max_ref_num + 1, dpb_spec);

    *handle = (HLM_VOID *)dpb_spec;

    return HLM_STS_OK;
}

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
                                 HLM_S32      *dpb_free_index)
{
    HLMD_DPB_SPEC  *dpb_spec = (HLMD_DPB_SPEC*)handle;
    HLM_S32         i        = 0;
    HLMD_FRAME     *pic      = dec_pic_buf;

    // 找到DPB中空闲缓存区
    for (i = 0; i < dpb_spec->pic_num; i++, pic++)
    {
        if (!(dpb_spec->pic_data[i].dpb_lock_flag))
        {
            dpb_spec->pic_data[i].dpb_lock_flag = 1;
            *curr_frame = pic;
            *dpb_free_index = i;
            break;
        }
    }

    if (!(i ^ dpb_spec->pic_num))
    {
        return HLM_STS_ERR;
    }

    return HLM_STS_OK;
}

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
                                   HLM_U32            ref_count)
{
    HLM_STATUS sts              = HLM_STS_ERR;
    HLM_S32    not_i_patch_flag = (HLM_FRAME_TYPE_I != patch_ctx->patch_header.pps.pic_type);
    HLM_S32    img_step         = curr_frame->ref_pic_data->yuv.step[0];
    HLM_U32    i                = 0;
    HLM_U32    index            = 0;

    curr_frame->ref_pic_data->fram_num = patch_ctx->patch_header.pps.poc;  // poc

    if ((!(patch_ctx->first_cu_in_patch)) ||
        (patch_ctx->last_patch_type != patch_ctx->patch_header.pps.pic_type) ||
        (!(patch_ctx->last_ref_count ^ ref_count)))
    {
        if (not_i_patch_flag)
        {
            // 获取默认的参考帧队列
            sts = HLMD_DPB_get_defult_ref_list(dpb_free_index, patch_ctx->short_ref_list, patch_ctx->short_ref_cnt,
                patch_ctx->patch_header.pps.pic_type, img_step, ref_count, curr_frame, patch_ctx->ref_list);
            HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);
        }
        patch_ctx->last_patch_type = patch_ctx->patch_header.pps.pic_type;
        patch_ctx->last_ref_count = ref_count;
    }

    // 对ref_list正确性的检查
    for (i = 0; i < patch_ctx->ref_count; i++)
    {
        if ((ref_list[i].ref_pic_data.fram_num == curr_frame->ref_pic_data->fram_num) ||
            (HLM_NULL == ref_list[i].ref_pic_data.yuv.data[0]))
        {
            return HLM_STS_ERR;
        }
        if (ref_list[i].ref_pic_data.yuv.data[0] == curr_frame->ref_pic_data->yuv.data[0])
        {
            return HLM_STS_ERR;
        }
    }

    return HLM_STS_OK;
}

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
                                  HLM_S32         *frame_poc)
{
    HLM_STATUS sts                 = HLM_STS_ERR;
    HLM_S32    current_is_long_ref = 0;
    HLM_U32    i = 0;

    if (!nal_ref_idc)
    {
        if (curr_frame->ref_pic_data->unref_set_id == 0)
        {
            curr_frame->ref_pic_data->unref_set_id = 1;
        }
        return HLM_STS_OK;
    }

    // 如果遇到IDR帧, 则释放所有短期参考帧
    if (patch_ctx->idr_frame_flag)
    {
        for (i = 0; i < patch_ctx->short_ref_cnt; i++)
        {
            if (patch_ctx->short_ref_list[i])
            {
                if (patch_ctx->short_ref_list[i]->ref_pic_data->unref_set_id == 0)
                {
                    patch_ctx->short_ref_list[i]->ref_pic_data->unref_set_id = 1;
                }
            }
        }
        memset(&patch_ctx->short_ref_list[0], 0, patch_ctx->short_ref_cnt * sizeof(HLMD_FRAME*));
        patch_ctx->short_ref_cnt = 0;
    }

    // 移除多余的参考帧
    if (nal_ref_idc)
    {
        if (patch_ctx->short_ref_cnt)
        {
            if (patch_ctx->short_ref_list[patch_ctx->short_ref_cnt - 1])
            {
                patch_ctx->short_ref_list[patch_ctx->short_ref_cnt - 1]->ref_pic_data->unref_set_id = 1;
                HLMD_DPB_remove_short_ref(&patch_ctx->short_ref_cnt, patch_ctx->short_ref_list, patch_ctx->short_ref_cnt - 1);
            }
        }
    }

    // 解码一帧都被默认放入短期参考队列开始
    if (!current_is_long_ref && nal_ref_idc)
    {
        sts = HLMD_DPB_put_curr_frame_to_ref_list(curr_frame, &(patch_ctx->short_ref_cnt), patch_ctx->short_ref_list);
        HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);
    }

    return HLM_STS_OK;
}

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
                                      HLMD_FRAME        *dec_pic_buf)
{
    HLM_S32         i        = 0;
    HLMD_FRAME     *pic      = dec_pic_buf;
    HLMD_DPB_SPEC  *dpb_spec = (HLMD_DPB_SPEC *)handle;

    HLM_CHECK_ERROR((dpb_spec->pic_num != pic_num), HLM_STS_ERR);

    for (i = 0; i < dpb_spec->pic_num; i++)
    {
        if (pic->ref_pic_data->unref_set_id == 1)
        {
            pic->ref_pic_data->unref_set_id = 0;
            pic->ref_pic_data->dpb_lock_flag = 0;
        }
        pic++;
    }

    return HLM_STS_OK;
}
