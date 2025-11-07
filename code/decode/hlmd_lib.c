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
#include "hlmd_lib.h"
#include "hlmd_defs.h"
#include "hlmd_init.h"
#include "hlmd_hls.h"

// 码流填充
HLM_VOID HLMD_LIB_bitstream_refill(HLMD_PRE_BITSTREAM *bs)
{
    HLM_S32 shift = 64 - bs->next_bits_cnt;  //待填充的比特数
    HLM_U64 val   = 0;

    while ((shift >= 8) && (bs->bytes_remaining))
    {
        val = *bs->data++;      //获取一个字节
        bs->bytes_remaining--;  //码流中剩余字节数
        if (0 == val)
        {
            bs->zero_succession_cnt++;
        }
        else if ((3 == val) && (2 <= bs->zero_succession_cnt))  //记录0x00出现次数，当0x00出现两次时，需要考虑0x03
        {
            bs->zero_succession_cnt = 0;
            continue;
        }
        else
        {
            bs->zero_succession_cnt = 0;
        }

        shift -= 8;             //偏移减8
        val <<= shift;          //将该字节偏移到适当位置
        bs->next_bits |= val;
    }

    bs->next_bits_cnt = 64 - shift;  //nextbits中有效比特数
}

/***************************************************************************************************
* 功  能：码流初始化
* 参  数：
*         bs_in     -IO     码流结构体
*         buffer    -I      buffer缓冲区
*         len       -I      buffer长度
* 返回值：HLM_VOID
* 备  注：
***************************************************************************************************/
HLM_VOID HLMD_LIB_bitstream_Init(HLM_VOID               *bs_in,
                                 HLM_U08                *buffer,
                                 HLM_S32                 len)
{
    HLMD_PRE_BITSTREAM *bs = (HLMD_PRE_BITSTREAM *)bs_in;

    bs->data                = buffer;     //码流起始地址
    bs->bytes_remaining     = len;        //该段码流剩余字节数
    bs->next_bits           = 0;          //即将处理的码流，存放到该变量中。8字节大小。
    bs->next_bits_cnt       = 0;          //存放在nextbits里的码流的剩余比特数
    bs->zero_succession_cnt = 0;          //连续出现0x00的次数
    HLMD_LIB_bitstream_refill(bs);  //填充next_bits变量
}

// 获取码流中的n个比特
HLM_U32 HLMD_LIB_read_bits(HLM_VOID             *bs_in,
                           HLM_S32               n)
{
    HLMD_PRE_BITSTREAM *bs = (HLMD_PRE_BITSTREAM *)bs_in;
    HLM_U64 val            = 0;

    if (bs->next_bits_cnt < n)  //当nextbits中字节不够时，填充
    {
        HLMD_LIB_bitstream_refill(bs);
    }

    val                = bs->next_bits;
    val              >>= 64 - n;  //获取n个比特
    bs->next_bits    <<= n;       //左移最左端n个比特
    bs->next_bits_cnt -= n;       //nextbits中有效比特数减少

    return (HLM_U32)val;
}

// 预解析序列头
HLM_STATUS HLMD_LIB_pre_parse_sps(HLMD_PRE_BITSTREAM      *bs,
                                  HLM_PARAM_SPS           *sps)
{
    HLM_S32 i                   = 0;
    HLM_S32 uniform_patch_split = 1;  // patch划分是否均匀
    HLM_S32 seq_horizontal_size = 0;
    HLM_S32 seq_vertical_size   = 0;
    HLM_S32 padded_pic_width    = 0;
    HLM_S32 padded_pic_height   = 0;

    sps->profile                 = HLMD_LIB_read_bits(bs,  8);  // profile_id
    seq_horizontal_size          = HLMD_LIB_read_bits(bs, 32);  // seq_horizontal_size
    seq_vertical_size            = HLMD_LIB_read_bits(bs, 32);  // seq_vertical_size
    sps->bit_depth_luma_minus8   = HLMD_LIB_read_bits(bs,  4);  // seq_bit_depth_luma_minus8
    sps->bit_depth_chroma_minus8 = HLMD_LIB_read_bits(bs,  4);  // seq_bit_depth_chroma_minus8
    sps->bpp_i                   = HLMD_LIB_read_bits(bs, 12);  // seq_bpp_i_picture
    sps->bpp_p                   = HLMD_LIB_read_bits(bs, 12);  // seq_bpp_p_picture
    sps->format                  = HLMD_LIB_read_bits(bs,  3);  // seq_picture_format
    sps->i_frame_enable_ibc      = HLMD_LIB_read_bits(bs,  1);  // seq_i_picture_ibc_enable_flag
    sps->p_frame_enable_ibc      = HLMD_LIB_read_bits(bs,  1);  // seq_p_picture_ibc_enable_flag
    sps->mv_ref_cross_patch      = HLMD_LIB_read_bits(bs,  1);  // seq_mv_cross_patch_enable_flag
    sps->intra_8x8_enable_flag   = HLMD_LIB_read_bits(bs,  1);  // seq_intra_8x8_enable_flag
    sps->cu_delta_qp_enable_flag = HLMD_LIB_read_bits(bs,  1);  // seq_cu_delta_qp_enable_flag
    sps->mv_limit_enable_flag    = HLMD_LIB_read_bits(bs,  1);  // seq_mv_search_range_limit_enable_flag
    if (sps->mv_limit_enable_flag)
    {
        sps->mv_search_width  = HLMD_LIB_read_bits(bs, 8);  // seq_mv_search_range_width
        sps->mv_search_height = HLMD_LIB_read_bits(bs, 8);  // seq_mv_search_range_height
    }
    else
    {
        sps->mv_search_width  = seq_horizontal_size << 1;
        sps->mv_search_height = seq_vertical_size << 1;
    }
    sps->patch_info.patch_num = HLMD_LIB_read_bits(bs, 8) + 1;  // seq_patch_num_minus1
    for (i = 0; i < (HLM_S32)sps->patch_info.patch_num; i++)
    {
        sps->patch_info.patch_param[i].patch_x               = HLMD_LIB_read_bits(bs, 32);  // seq_patch_x
        sps->patch_info.patch_param[i].patch_y               = HLMD_LIB_read_bits(bs, 32);  // seq_patch_y
        sps->patch_info.patch_param[i].patch_width[0]        = HLMD_LIB_read_bits(bs, 32);  // seq_patch_width
        sps->patch_info.patch_param[i].patch_height[0]       = HLMD_LIB_read_bits(bs, 32);  // seq_patch_height
        sps->patch_info.patch_param[i].patch_coded_width[0]  = HLM_SIZE_ALIGN_16(sps->patch_info.patch_param[i].patch_width[0]);
        sps->patch_info.patch_param[i].patch_coded_height[0] = HLM_SIZE_ALIGN_8(sps->patch_info.patch_param[i].patch_height[0]);
        switch (sps->format)
        {
        case HLM_IMG_YUV_444:
        case HLM_IMG_RGB:
            sps->patch_info.patch_param[i].patch_width[1]        = sps->patch_info.patch_param[i].patch_width[0];
            sps->patch_info.patch_param[i].patch_height[1]       = sps->patch_info.patch_param[i].patch_height[0];
            sps->patch_info.patch_param[i].patch_coded_width[1]  = sps->patch_info.patch_param[i].patch_coded_width[0];
            sps->patch_info.patch_param[i].patch_coded_height[1] = sps->patch_info.patch_param[i].patch_coded_height[0];
            break;
        case HLM_IMG_YUV_420:
            sps->patch_info.patch_param[i].patch_width[1]        = sps->patch_info.patch_param[i].patch_width[0] >> 1;
            sps->patch_info.patch_param[i].patch_height[1]       = sps->patch_info.patch_param[i].patch_height[0] >> 1;
            sps->patch_info.patch_param[i].patch_coded_width[1]  = sps->patch_info.patch_param[i].patch_coded_width[0] >> 1;
            sps->patch_info.patch_param[i].patch_coded_height[1] = sps->patch_info.patch_param[i].patch_coded_height[0] >> 1;
            break;
        case HLM_IMG_YUV_422:
            sps->patch_info.patch_param[i].patch_width[1]        = sps->patch_info.patch_param[i].patch_width[0] >> 1;
            sps->patch_info.patch_param[i].patch_height[1]       = sps->patch_info.patch_param[i].patch_height[0];
            sps->patch_info.patch_param[i].patch_coded_width[1]  = sps->patch_info.patch_param[i].patch_coded_width[0] >> 1;
            sps->patch_info.patch_param[i].patch_coded_height[1] = sps->patch_info.patch_param[i].patch_coded_height[0];
            break;
        }
    }

    sps->pic_width_in_cus_minus1 = ((seq_horizontal_size + 15) >> 4) - 1;
    sps->pic_height_in_map_units_minus1 = ((seq_vertical_size + 7) >> 3) - 1;
    padded_pic_width = (sps->pic_width_in_cus_minus1 + 1) << 4;
    padded_pic_height = (sps->pic_height_in_map_units_minus1 + 1) << 3;
    if (seq_horizontal_size != padded_pic_width || seq_vertical_size != padded_pic_height)
    {
        sps->frame_cropping_flag      = 1;
        sps->frame_crop_left_offset   = 0;
        sps->frame_crop_right_offset  = padded_pic_width - seq_horizontal_size;
        sps->frame_crop_top_offset    = 0;
        sps->frame_crop_bottom_offset = padded_pic_height - seq_vertical_size;
    }
    else
    {
        sps->frame_cropping_flag      = 0;
        sps->frame_crop_left_offset   = 0;
        sps->frame_crop_right_offset  = 0;
        sps->frame_crop_top_offset    = 0;
        sps->frame_crop_bottom_offset = 0;
    }

    if (sps->format == HLM_IMG_YUV_420 || sps->format == HLM_IMG_YUV_422)
    {
        sps->intra_8x8_enable_flag = 1;  // 420和422强制intra8x8
    }

    // 校验patch划分的合理性
    assert(0 < sps->patch_info.patch_num && sps->patch_info.patch_num < HLM_MAX_PATCH_NUM);
    for (i = 1; i < (HLM_S32)sps->patch_info.patch_num; i++)
    {
        if (sps->patch_info.patch_param[i].patch_width[0] != sps->patch_info.patch_param[0].patch_width[0] ||
            sps->patch_info.patch_param[i].patch_height[0] != sps->patch_info.patch_param[0].patch_height[0])
        {
            uniform_patch_split = 0;
            break;
        }
    }
    if (uniform_patch_split == 0 && 0 == HLM_COM_CheckPatchSplit(
        ((sps->pic_width_in_cus_minus1 + 1) << 4) - sps->frame_crop_left_offset - sps->frame_crop_right_offset,  // 真实的图像宽高
        ((sps->pic_height_in_map_units_minus1 + 1) << 3) - sps->frame_crop_top_offset - sps->frame_crop_bottom_offset,
        &sps->patch_info))
    {
        printf("Patch划分信息无法拼成完整图像!\n");
        assert(0);
    }

    return HLM_STS_OK;
}

// RAM空间分配
HLM_STATUS HLMD_LIB_alloc_ram(HLM_U32             code_height,
                              HLM_U32             code_width,
                              HLM_U32             component,
                              HLM_RAM_BUF        *ram_buf,
                              HLMD_CU_INFO       *cur_cu,
                              HLM_NEIGHBOR_INFO  *nbi_info,
                              HLMD_PATCH_CTX     *patch_ctx)
{
    HLM_U32 i        = 0;
    HLM_U32 j        = 0;
    HLM_U32 cu_width = code_width >> HLM_LOG2_WIDTH_SIZE;

    for (i = 0; i < component; i++)
    {
        cur_cu->com_cu_info.cu_pred_info.pred[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.pred[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.res[i] = (HLM_COEFF *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.res[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.rec[i] = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_U16), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.rec[i], HLM_STS_ERR_MEM_LACK);
        cur_cu->com_cu_info.cu_pred_info.coeff[i] = (HLM_COEFF *)HLM_MEM_Calloc(ram_buf, HLM_CU_SIZE * sizeof(HLM_COEFF), 16);
        HLM_CHECK_ERROR(HLM_NULL == cur_cu->com_cu_info.cu_pred_info.coeff[i], HLM_STS_ERR_MEM_LACK);
    }

    nbi_info->intra_rec_up_y = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, (cu_width << 4) * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->intra_rec_up_y, HLM_STS_ERR_MEM_LACK);
    nbi_info->intra_rec_up_u = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, (cu_width << 4) * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->intra_rec_up_u, HLM_STS_ERR_MEM_LACK);
    nbi_info->intra_rec_up_v = (HLM_U16 *)HLM_MEM_Calloc(ram_buf, (cu_width << 4) * sizeof(HLM_U16), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->intra_rec_up_v, HLM_STS_ERR_MEM_LACK);
    nbi_info->intra_pred_mode_up = (HLM_U08 *)HLM_MEM_Calloc(ram_buf, cu_width << 2, 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->intra_pred_mode_up, HLM_STS_ERR_MEM_LACK);
    nbi_info->inter_mv_up = (HLM_MV *)HLM_MEM_Calloc(ram_buf, sizeof(HLM_MV)*(cu_width << 2), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->inter_mv_up, HLM_STS_ERR_MEM_LACK);
    nbi_info->pred_type_up = (HLM_CU_TYPE *)HLM_MEM_Calloc(ram_buf, sizeof(HLM_CU_TYPE)* (cu_width << 2), 16);
    HLM_CHECK_ERROR(HLM_NULL == nbi_info->pred_type_up, HLM_STS_ERR_MEM_LACK);

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：预解析序列头
* 参  数：*
*         in_buf       -I       SPS包数据指针
*         in_size      -O       SPS包的大小
*         video_info   -I       图像序列信息结构体
* 返回值：状态码
* 备  注：如果mtab[i].size为0，则不需要分配该块内存
***************************************************************************************************/
HLM_STATUS HLMD_LIB_PreParseSeqHeader(HLM_VOID           *in_buf,
                                      HLM_SZT             in_size,
                                      HLMD_VIDEO_INFO    *video_info)
{
    HLM_STATUS         sts            = HLM_STS_ERR;
    HLM_U08           *stream_buf     = in_buf;  // 指向当前待解析的码流地址
    HLM_SZT            stream_len     = in_size;
    HLM_U08           *nalu_buf       = HLM_NULL;
    HLM_S32            nalu_len       = 0;
    HLM_S32            start_code_len = 0;
    HLM_PARAM_SPS      tmp_sps        = { 0 };
    HLMD_PRE_BITSTREAM ip_bs          = { 0 };
    HLMD_NALU_HEADER   nalu_header    = { 0 };

    HLM_CHECK_ERROR((HLM_NULL == in_buf), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((HLM_NULL == video_info), HLM_STS_ERR_NULL_PTR);

    while (stream_len > 0 && HLM_STS_OK == HLMD_HLS_GetNalu(stream_buf, stream_len, &nalu_buf, &nalu_len, &start_code_len))
    {
        nalu_buf += start_code_len;
        nalu_len -= start_code_len;
        if (nalu_len > 0)
        {
            sts = HLMD_HLS_ProcessNaluHeader(nalu_buf, nalu_len, &nalu_header);

            nalu_buf += HLMD_NALU_HEADER_LEN;  // 跳过nalu_header(1字节)
            nalu_len -= HLMD_NALU_HEADER_LEN;
            HLMD_LIB_bitstream_Init(&ip_bs, nalu_buf, nalu_len);

            if (nalu_header.nal_unit_type == HLM_SPS_NUT)
            {
                sts = HLMD_LIB_pre_parse_sps(&ip_bs, &tmp_sps);

                video_info->code_width[0]    = (tmp_sps.pic_width_in_cus_minus1 + 1) * 16;
                video_info->code_height[0]   = (tmp_sps.pic_height_in_map_units_minus1 + 1) * 8;
                video_info->bit_depth_luma   = (tmp_sps.bit_depth_luma_minus8 + 8);
                video_info->bit_depth_chroma = (tmp_sps.bit_depth_chroma_minus8 + 8);
                video_info->format           = tmp_sps.format;
                video_info->ref_frm_num      = 1;
                video_info->profile_idc      = tmp_sps.profile;
                video_info->mv_search_width  = tmp_sps.mv_search_width;
                video_info->mv_search_height = tmp_sps.mv_search_height;
                if (tmp_sps.frame_cropping_flag)
                {
                    video_info->crop_left    = tmp_sps.frame_crop_left_offset;
                    video_info->crop_right   = tmp_sps.frame_crop_right_offset;
                    video_info->crop_top     = tmp_sps.frame_crop_top_offset;
                    video_info->crop_bottom  = tmp_sps.frame_crop_bottom_offset;
                }
                else
                {
                    video_info->crop_left    = 0;
                    video_info->crop_right   = 0;
                    video_info->crop_top     = 0;
                    video_info->crop_bottom  = 0;
                }
            }
            stream_len -= (nalu_buf - stream_buf) + nalu_len;
            stream_buf = nalu_buf + nalu_len;
        }
    }

    switch (video_info->format)
    {
    case HLM_IMG_YUV_444:
    case HLM_IMG_RGB:
        video_info->code_width[1]  = video_info->code_width[0];
        video_info->code_height[1] = video_info->code_height[0];
        video_info->code_width[2]  = video_info->code_width[0];
        video_info->code_height[2] = video_info->code_height[0];
        break;
    case HLM_IMG_YUV_422:
        video_info->code_width[1]  = video_info->code_width[0] >> 1;
        video_info->code_height[1] = video_info->code_height[0];
        video_info->code_width[2]  = video_info->code_width[0] >> 1;
        video_info->code_height[2] = video_info->code_height[0];
        break;
    case HLM_IMG_YUV_420:
        video_info->code_width[1]  = video_info->code_width[0] >> 1;
        video_info->code_height[1] = video_info->code_height[0]>>1;
        video_info->code_width[2]  = video_info->code_width[0] >> 1;
        video_info->code_height[2] = video_info->code_height[0]>>1;
        break;
    }

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：获取解码算法模型所需存储信息
* 参  数：*
*         ability         -I    能力集参数指针
*         mem_tab         -O    存储空间参数结构体
*         video_info      -I    视频信息
* 返回值：状态码
* 备  注：如果mtab[i].size为0，则不需要分配该块内存
***************************************************************************************************/
HLM_STATUS HLMD_LIB_GetMemSize(HLMD_ABILITY     *ability,
                               HLM_MEM_TAB       mem_tab[HLM_MEM_TAB_NUM],
                               HLMD_VIDEO_INFO  *video_info)
{
    HLM_STATUS   sts                       = HLM_STS_ERR;
    HLMD_SW_SPEC spec                      = { 0 };
    HLM_SZT      tmp_buf_size              = 0;
    HLM_SZT      ddr_persist_size          = 0;
    HLM_SZT      ddr_scratch_size          = 0;
    HLM_SZT      ram_scratch_size          = 0;
    HLM_SZT      modu_status_size          = 0;
    HLM_SZT      modu_work_size            = 0;
    HLM_MEM_TAB  mtab_tmp[HLM_MEM_TAB_NUM] = { 0 };
    HLM_MEM_TAB *mtab_ddr_persist          = HLM_NULL;
    HLM_MEM_TAB *mtab_ddr_scratch          = HLM_NULL;
    HLM_MEM_TAB *mtab_ram_scratch          = HLM_NULL;

    //参数检测
    HLM_CHECK_ERROR((HLM_NULL == ability) || (HLM_NULL == mem_tab), HLM_STS_ERR_NULL_PTR);

    sts = HLMD_INIT_CheckAbility(ability);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    // 初始化局部变量
    ddr_persist_size = 0;
    ddr_scratch_size = 0;
    ram_scratch_size = 0;
    mtab_ddr_persist = &mtab_tmp[HLM_MEM_TAB_DDR_PERSIST];
    mtab_ddr_scratch = &mtab_tmp[HLM_MEM_TAB_DDR_SCRATCH];
    mtab_ram_scratch = &mtab_tmp[HLM_MEM_TAB_RAM_SCRATCH];

    // 初始化临时spec
    memset(&spec, 0, sizeof(HLMD_SW_SPEC));
    memcpy(&spec.ability, ability, sizeof(HLMD_ABILITY));
    spec.sps.mv_search_width = video_info->mv_search_width;
    spec.sps.mv_search_height = video_info->mv_search_height;

    // 获取接口模块所需外部DDR不可复用的缓存大小
    mtab_ddr_persist->size = HLM_MAX_MEM_SIZE;  // 虚拟分配足够大空间
    mtab_ddr_persist->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_persist->base = (HLM_VOID *)&spec;

    sts = HLMD_INIT_AllocDdrPersistMem(&spec, mtab_ddr_persist, &tmp_buf_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    ddr_persist_size += tmp_buf_size;

    // 获取接口模块所需外部DDR可复用的缓存大小
    mtab_ddr_scratch->size = HLM_MAX_MEM_SIZE;  // 虚拟分配足够大空间
    mtab_ddr_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_scratch->base = (HLM_VOID *)&spec;

    // 获取接口模块所需内部RAM缓存大小
    mtab_ram_scratch->size = HLM_MAX_MEM_SIZE;  // 虚拟分配足够大空间
    mtab_ram_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ram_scratch->base = (HLM_VOID *)&spec;

    sts = HLMD_INIT_AllocRamScratchMem(&spec, mtab_ram_scratch, &tmp_buf_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    ram_scratch_size += tmp_buf_size;

    //计算各模块所需的内存，包括状态内存、工作内存
    sts = HLMD_INIT_GetModuleBuf(&spec, &modu_status_size, &modu_work_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    ddr_persist_size += modu_status_size;
    ddr_scratch_size += modu_work_size;

    //总大小
    ddr_persist_size = HLM_SIZE_ALIGN(ddr_persist_size, HLM_MEM_ALIGN_16BYTE);
    ddr_scratch_size = HLM_SIZE_ALIGN(ddr_scratch_size, HLM_MEM_ALIGN_16BYTE);
    ram_scratch_size = HLM_SIZE_ALIGN(ram_scratch_size, HLM_MEM_ALIGN_16BYTE);
    HLM_CHECK_ERROR((ddr_persist_size + ddr_scratch_size + ram_scratch_size) > HLM_MAX_MEM_SIZE, HLM_STS_ERR_OVER_MAX_MEM);

    mtab_ddr_persist = &mem_tab[HLM_MEM_TAB_DDR_PERSIST];
    mtab_ddr_persist->size = ddr_persist_size;
    mtab_ddr_persist->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_persist->space = HLM_MEM_EXTERNAL_DDR;
    mtab_ddr_persist->attrs = HLM_MEM_PERSIST;
    mtab_ddr_persist->base = HLM_NULL;

    mtab_ddr_scratch = &mem_tab[HLM_MEM_TAB_DDR_SCRATCH];
    mtab_ddr_scratch->size = ddr_scratch_size;
    mtab_ddr_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_scratch->space = HLM_MEM_INTERNAL_ROM;
    mtab_ddr_scratch->attrs = HLM_MEM_PERSIST;
    mtab_ddr_scratch->base = HLM_NULL;

    mtab_ram_scratch = &mem_tab[HLM_MEM_TAB_RAM_SCRATCH];
    mtab_ram_scratch->size = ram_scratch_size;
    mtab_ram_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ram_scratch->space = HLM_MEM_EXTERNAL_DDR;
    mtab_ram_scratch->attrs = HLM_MEM_PERSIST;
    mtab_ram_scratch->base = HLM_NULL;

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：创建解码算法模型实例,内存初始化
* 参  数：*
*         ability         -I    能力集参数指针
*         mem_tab         -O    存储空间参数结构体
*         handle          -O    编码实例句柄指针
*         video_info      -I    视频信息
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_LIB_Create(HLMD_ABILITY     *ability,
                           HLM_MEM_TAB       mem_tab[HLM_MEM_TAB_NUM],
                           HLM_VOID        **handle,
                           HLMD_VIDEO_INFO  *video_info)
{
    HLM_STATUS     sts                       = HLM_STS_ERR;
    HLMD_SW_SPEC  *spec                      = HLM_NULL;
    HLM_MEM_TAB    mtab_tmp[HLM_MEM_TAB_NUM] = { 0 };
    HLM_MEM_TAB   *mtab_ddr_persist          = HLM_NULL;
    HLM_MEM_TAB   *mtab_ddr_scratch          = HLM_NULL;
    HLM_MEM_TAB   *mtab_ram_scratch          = HLM_NULL;
    HLM_U08       *left_ddr_persist_buf      = HLM_NULL;
    HLM_U08       *left_ddr_scratch_buf      = HLM_NULL;
    HLM_U08       *left_ram_scratch_buf      = HLM_NULL;
    HLM_SZT        left_ddr_persist_size     = 0;
    HLM_SZT        left_ddr_scratch_size     = 0;
    HLM_SZT        left_ram_scratch_size     = 0;
    HLM_SZT        tmp_buf_size              = 0;
    HLM_RAM_BUF    ram_buf                   = { 0 };

    // 参数检测
    HLM_CHECK_ERROR((HLM_NULL == ability) || (HLM_NULL == mem_tab) || (HLM_NULL == handle), HLM_STS_ERR_NULL_PTR);

    // 检查缓存表是否合法
    sts = HLM_MEM_CheckMemTab(mem_tab, (HLM_S32)HLM_MEM_TAB_NUM, HLM_MEM_ALIGN_16BYTE);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 检查配置参数是否在有效范围
    sts = HLMD_INIT_CheckAbility(ability);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 初始化
    mtab_ddr_persist = &mem_tab[HLM_MEM_TAB_DDR_PERSIST];
    mtab_ddr_persist->alignment = HLM_MEM_ALIGN_16BYTE;
    left_ddr_persist_size = mtab_ddr_persist->size;
    left_ddr_persist_buf = (HLM_U08 *)mtab_ddr_persist->base;

    mtab_ddr_scratch = &mem_tab[HLM_MEM_TAB_DDR_SCRATCH];
    mtab_ddr_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    left_ddr_scratch_size = mtab_ddr_scratch->size;
    left_ddr_scratch_buf = (HLM_U08 *)mtab_ddr_scratch->base;

    mtab_ram_scratch = &mem_tab[HLM_MEM_TAB_RAM_SCRATCH];
    mtab_ram_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    left_ram_scratch_size = mtab_ram_scratch->size;
    left_ram_scratch_buf = (HLM_U08 *)mtab_ram_scratch->base;

    // 分配spec参数实例
    tmp_buf_size = sizeof(HLMD_SW_SPEC);
    tmp_buf_size = HLM_SIZE_ALIGN_16(tmp_buf_size);
    HLM_CHECK_ERROR(left_ddr_persist_size < tmp_buf_size, HLM_STS_ERR_MEM_LACK);
    spec = (HLMD_SW_SPEC *)(left_ddr_persist_buf);

    // 初始化spec结构体参数
    memset(spec, 0, sizeof(HLMD_SW_SPEC));
    memcpy(&spec->ability, ability, sizeof(HLMD_ABILITY));
    spec->sps.mv_search_width = video_info->mv_search_width;
    spec->sps.mv_search_height = video_info->mv_search_height;

    // 分配接口模块所需外部DDR不可复用的缓存大小
    mtab_ddr_persist = &mtab_tmp[HLM_MEM_TAB_DDR_PERSIST];
    mtab_ddr_persist->size = left_ddr_persist_size;
    mtab_ddr_persist->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_persist->base = left_ddr_persist_buf;

    sts = HLMD_INIT_AllocDdrPersistMem(spec, mtab_ddr_persist, &tmp_buf_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);
    left_ddr_persist_size -= tmp_buf_size;
    left_ddr_persist_buf += tmp_buf_size;

    // 分配接口模块所需外部DDR可复用的缓存大小
    mtab_ddr_scratch = &mtab_tmp[HLM_MEM_TAB_DDR_SCRATCH];
    mtab_ddr_scratch->size = left_ddr_scratch_size;
    mtab_ddr_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ddr_scratch->base = left_ddr_scratch_buf;

    // 分配接口模块所需内部RAM缓存
    mtab_ram_scratch = &mtab_tmp[HLM_MEM_TAB_RAM_SCRATCH];
    mtab_ram_scratch->size = left_ram_scratch_size;
    mtab_ram_scratch->alignment = HLM_MEM_ALIGN_16BYTE;
    mtab_ram_scratch->base = left_ram_scratch_buf;

    sts = HLMD_INIT_AllocRamScratchMem(spec, mtab_ram_scratch, &tmp_buf_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 将RAM空间分配给cur_cu和nbi_info，避免每个patch重复开辟
    ram_buf.start = spec->ram_buf;
    ram_buf.end = spec->ram_buf + spec->ram_size;
    ram_buf.cur_pos = spec->ram_buf;
    sts = HLMD_LIB_alloc_ram(HLM_SIZE_ALIGN_8(spec->ability.code_height[0]), HLM_SIZE_ALIGN_16(spec->ability.code_width[0]),
        spec->sps.format== HLM_IMG_YUV_400 ? 1 : 3, &ram_buf, &spec->cur_cu, &spec->nbi_info, &spec->patch_ctx);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    //分配子模块所需内存，并创建子模块
    sts = HLMD_INIT_AllocModuleBuf(spec, left_ddr_persist_buf, left_ddr_persist_size, left_ddr_scratch_buf, left_ddr_scratch_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    //输出实例句柄
    *handle = (HLM_VOID *)spec;

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：解码一帧码流
* 参  数：*
*         handle    -I  解码实例句柄指针
*         in_buf    -I  解码模型输入参数地址
*         in_size   -I  解码模型输入参数大小
*         out_buf   -O  解码模型输出参数地址
*         out_size  -I  解码模型输出参数大小
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMD_LIB_DecodeFrame(HLM_VOID *handle,
                                HLM_VOID *in_buf,
                                HLM_SZT   in_size,
                                HLM_VOID *out_buf,
                                HLM_SZT   out_size)
{
    HLM_STATUS         sts            = HLM_STS_ERR;
    HLMD_STREAM_IN    *input          = HLM_NULL;
    HLMD_PROCESS_OUT  *output         = HLM_NULL;
    HLMD_SW_SPEC      *spec           = HLM_NULL;
    HLM_U08           *stream_buf     = 0;
    HLM_S64            stream_len     = 0;
    HLM_S32            start_code_len = 0;
    HLM_U08           *nalu_buf       = HLM_NULL;
    HLM_S32            nalu_len       = 0;
    HLMD_NALU_HEADER   nalu_header    = { 0 };

    // 参数检测
    sts = HLMD_INIT_CheckIo(handle, in_buf, in_size, out_buf, out_size);
    HLM_CHECK_ERROR(sts != HLM_STS_OK, sts);

    // 参数初始化
    spec = (HLMD_SW_SPEC *)handle;
    input = (HLMD_STREAM_IN *)in_buf;
    output = (HLMD_PROCESS_OUT *)out_buf;
    stream_buf = input->stream_buf;
    stream_len = input->stream_len;

    while (stream_len > 0 && HLM_STS_OK == HLMD_HLS_GetNalu(stream_buf, stream_len, &nalu_buf, &nalu_len, &start_code_len))
    {
        nalu_buf += start_code_len;  // 跳过起始码
        nalu_len -= start_code_len;
        if (nalu_len > 0)
        {
            sts = HLMD_HLS_ProcessNalu(spec, nalu_buf, nalu_len, output);
            HLM_CHECK_ERROR((sts != HLM_STS_OK), sts);
        }
        stream_len -= (nalu_buf - stream_buf) + nalu_len;
        stream_buf = nalu_buf + nalu_len;
    }

    return HLM_STS_OK;
}
