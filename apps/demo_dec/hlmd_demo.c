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
#include "hlmd_lib.h"

// 全局变量
HLMD_ABILITY ability = { 0 };

// 最大支持内存分块数
#define TMP_BUFFER_SIZE                 (2000)
#define TMP_SIZE                        (51200)
#define TMP_SHIFT                       (4)
#define READ_4_BYTES(p)                 (*(HLM_S32 *)(p))
#define MAX_UNIT_NUM                    (200)

// 对齐Nalu类型
#define IS_FRM_START_CODE(marker)       (((marker & 0x001F) == 1 || (marker & 0x001F) == 2) && (marker & 0x8000))
#define IS_FRM_END_CODE(marker)         ((marker & 0x001F) == 6 || (marker & 0x001F) == 7)
#define IS_NO_FRM_CODE(marker)          ((marker & 0x001F) == 0 || (marker & 0x001F) > 2)

// 用户参数
typedef struct _DEMO_USER_DATA
{
    HLM_S32  yuv_output_enable;
    FILE    *fp_log;
    FILE    *fp_dec;
    int      crop_left;
    int      crop_right;
    int      crop_top;
    int      crop_bottom;
} DEMO_USER_DATA;

// 获取码流中的视频信息
HLM_S32 HLMD_DEMO_get_video_info(FILE             *hlm_file,
                                 HLMD_VIDEO_INFO  *video_info,
                                 FILE             *fp_log)
{
    HLM_U08 *buf = HLM_NULL;
    HLM_SZT len  = 0;

    buf = (HLM_U08 *)HLM_MEM_Malloc(TMP_BUFFER_SIZE, 16);
    if (buf == HLM_NULL)
    {
        return -1;
    }

    //返回到起始位置
    fseek(hlm_file, 0, SEEK_SET);
    len = fread(buf, 1, TMP_BUFFER_SIZE, hlm_file);
    if (HLMD_LIB_PreParseSeqHeader(buf, TMP_BUFFER_SIZE, video_info) != HLM_STS_OK)
    {
        len = -1;
    }
    if (buf != HLM_NULL)
    {
        HLM_MEM_Free(buf);
        buf = HLM_NULL;
    }

    fseek(hlm_file, 0, SEEK_SET);

    if (len < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

// 创建算法模型句柄
HLM_S32 HLMD_DEMO_create_handle(HLM_MEM_TAB       mem_tab[HLM_MEM_TAB_NUM],
                                HLM_VOID        **handle,
                                FILE             *fp_log,
                                HLMD_VIDEO_INFO  *video_info)
{
    HLM_S32    i          = 0;
    HLM_STATUS sts        = HLM_STS_ERR;
    HLM_VOID  *lib_handle = HLM_NULL;

    // 获取所需资源数量
    sts = HLMD_LIB_GetMemSize(&ability, mem_tab, video_info);
    if (sts != HLM_STS_OK)
    {
        fprintf(stderr, "[LIB] get mem size failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        return -1;
    }

#ifdef _DEBUG
    // 打印内存大小
    for (i = 0; i < HLM_MEM_TAB_NUM; i++)
    {
#if defined(_M_X64)
        fprintf(stdout, "[LIB] memTab %d memory size = %lld\n", i, mem_tab[i].size);
#else
        fprintf(stdout, "[LIB] memTab %d memory size = %d\n", i, mem_tab[i].size);
#endif
    }
#endif

    // 分配算法模型所需资源
    sts = HLM_MEM_AllocMemTab(mem_tab, HLM_MEM_TAB_NUM);
    if (sts != HLM_STS_OK)
    {
        fprintf(stderr, "[LIB] alloc lib mem failed in Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    // 创建算法模型实例
    sts = HLMD_LIB_Create(&ability, mem_tab, &lib_handle, video_info);
    if (sts != HLM_STS_OK)
    {
        fprintf(stderr, "[LIB] create lib failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        return -1;
    }

    *handle = lib_handle;

    return 0;
}

// 销毁算法模型句柄
HLM_S32 HLMD_DEMO_destroy_handle(HLM_MEM_TAB     mem_tab[HLM_MEM_TAB_NUM],
                               HLM_VOID       *handle,
                               FILE           *fp_log)
{
    HLM_STATUS sts = HLM_STS_ERR;

    sts = HLM_MEM_FreeMemTab(mem_tab, HLM_MEM_TAB_NUM);

    if (HLM_STS_OK != sts)
    {
        fprintf(stderr, "[DEMO] free lib mem failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

// 获取一个nalu的码流
HLM_S64 HLMD_DEMO_get_nalu(unsigned char *s,
                           int            max_size,
                           FILE          *f)
{
    HLM_SZT i             = 0;
    char tmpbuf[TMP_SIZE] = { 0 };
    int oldpos            = 0;
    int prepos            = 0;
    HLM_SZT read_len      = 0;
    HLM_S32 tmp           = 0;
    HLM_SZT size          = 0;

    //寻找第一个0x01000000
    size = 0;
    oldpos = ftell(f);
    prepos = oldpos;
    if (feof(f))
    {
        return -1;
    }

    do
    {
        if ((read_len = fread(tmpbuf, 1, TMP_SIZE, f)) == 0)
        {
            return -1;  // 码流结尾
        }
        for (i = 0; i < read_len - TMP_SHIFT; i++)
        {
            if (READ_4_BYTES(&tmpbuf[i]) == 0x01000000)
            {
                oldpos += (long)size;
                goto search_next_start;
            }
            else if ((READ_4_BYTES(&tmpbuf[i]) & 0x0FFFFFF) == 0x00010000)
            {
                oldpos += (long)size;
                goto search_next_start;
            }
            size++;
        }
        fseek(f, -TMP_SHIFT, SEEK_CUR);
    } while (1);

search_next_start:

    fseek(f, oldpos, SEEK_SET);
    size = fread(&tmp, 1, 4, f);

    //寻找第2个 0x01000000
    while (1)
    {
        read_len = fread(tmpbuf, 1, TMP_SIZE, f);
        for (i = 0; i < read_len - TMP_SHIFT; i++)
        {
            if (READ_4_BYTES(&tmpbuf[i]) == 0x01000000)
            {
                goto start_code_find;
            }
            else if ((READ_4_BYTES(&tmpbuf[i]) & 0x0FFFFFF) == 0x00010000)
            {
                goto start_code_find;
            }
            size++;
        }
        if (feof(f))
        {
            size += TMP_SHIFT;
            break;
        }
        fseek(f, -TMP_SHIFT, SEEK_CUR);
    }

    fseek(f, oldpos, SEEK_SET);
    fread(s, 1, size, f);
    fseek(f, 0, SEEK_END);

    return size;

start_code_find:
    fseek(f, oldpos, SEEK_SET);
    fread(s, 1, size, f);

    return size;
}

#define BSWAP(bitswap_t) {                                   \
    unsigned int bitswap_tmp;                                \
    signed char *bitswap_src = (signed char *)&bitswap_t;    \
    signed char *bitswap_dst = (signed char *)&bitswap_tmp;  \
    bitswap_dst[0] = bitswap_src[3];                         \
    bitswap_dst[1] = bitswap_src[2];                         \
    bitswap_dst[2] = bitswap_src[1];                         \
    bitswap_dst[3] = bitswap_src[0];                         \
    bitswap_t = bitswap_tmp;                                 \
}

#define LMBD(t, pos) {                                       \
    unsigned int bitscan_tmp = t;                            \
    int bitscan_pos = 31;                                    \
    while (!(bitscan_tmp & 0x80000000) && bitscan_pos > -1)  \
    {                                                        \
        bitscan_tmp <<= 1;                                   \
        bitscan_pos--;                                       \
    }                                                        \
    pos = 31 - bitscan_pos;                                  \
}

// 读一个无符号指数哥伦布数据
unsigned int read_ue_golomb(unsigned char *bits,
                            int           *idx)
{
    unsigned int tmp     = 0;
    unsigned int len     = 0;
    unsigned char *rdptr = bits + ((*idx) >> 3);

    tmp = *(unsigned int*)rdptr;
    BSWAP(tmp);
    tmp <<= ((*idx) & 7);

    LMBD(tmp, len);

    *idx += len * 2 + 1;
    tmp <<= (len + 1);

    return (1 << len) + ((tmp >> (31 - len)) >> 1) - 1;
}

// 读n个比特
unsigned int read_n_bits(unsigned char *bits,
                         int           *idx,
                         unsigned int   n)
{
    unsigned int tmp = *(unsigned int *)(bits + ((*idx) >> 3));

    BSWAP(tmp);

    tmp <<= (*idx) & 7;
    tmp >>= (32 - n);
    *idx += n;

    return tmp;
}

// 读一个比特
unsigned int read_bit(unsigned char *bits,
                      int           *idx)
{
    unsigned int tmp = bits[(*idx) >> 3];

    tmp <<= 24 + ((*idx) & 7);
    *idx += 1;

    return tmp >> 31;
}

// 读一帧的码流
HLM_S32 HLMD_DEMO_read_one_frame(unsigned char *streambuf,
                                 int           *frm_len,
                                 FILE          *inpf,
                                 int            log2_max_frame_num,
                                 int            frame_cus_only,
                                 FILE          *fp_log)
{
    int i                = 0;
    HLM_S64 len          = 0;
    int size             = 0;
    int start_code       = 0;
    int bfields          = 0;
    int oldpos           = 0;
    short marker         = 0;
    int key_filed_cnt    = 0;
    unsigned char *bits  = 0;
    int interlace        = 0;
    int top_field        = 0;
    int have_top         = 0;
    int have_bottom      = 0;
    int pre_frame_num    = -1;
    int cur_frame_num    = -1;
    int frame_strm_len   = 0;  //一帧码流长度
    int header_len       = 0;  //SPS PPS等总长度
    int nalu_prior_flag  = 0;
    char is_patch_before = 1;  //0:表示上一个nalu是头信息；1：表示上一个nalu是patch
    unsigned int code    = 0;
    int index            = 0;

    *frm_len = 0;
    for (i = 0; i < MAX_UNIT_NUM; i++)
    {
        oldpos = ftell(inpf);
        len = HLMD_DEMO_get_nalu(streambuf, 0x7FFFFFFF, inpf); //从码流文件中获取一个NALU, 包含起始码部分
        if (len <= 0)
        {
            if (key_filed_cnt > 1) //如果出现过1个关键帧或者2个关键场
            {
                fseek(inpf, oldpos, SEEK_SET); //文件指针恢复到原来位置
                *frm_len = frame_strm_len;
                return 1;
            }
            else
            {
                return 0;
            }
        }

        start_code = *(int *)(streambuf);
        if (start_code == 0x01000000)
        {
            bits = streambuf + 4; //使用4字节的起始码
        }
        else if ((start_code & 0x00FFFFFF) == (0x01000000 >> 8))
        {
            bits = streambuf + 3; //使用3字节的起始码
        }
        else
        {
            return 0;
        }

        marker = *(short *)(bits);
        if (IS_FRM_END_CODE(marker)) //end_of_seq end_of_stream
        {
            if (nalu_prior_flag)
            {
                *frm_len = frame_strm_len;
                return 1;
            }
            else
            {
                continue;
            }
        }

        if (IS_FRM_START_CODE(marker)) //遇到一帧或一场的开始，first_cu_in_patch=0
        {
            nalu_prior_flag = 1;
            if (is_patch_before)
            {
                header_len = 0;
            }

            index = 1;  //first_cu_in_patch
            code = read_ue_golomb(bits + 1, &index);  //patch_type
            code = read_ue_golomb(bits + 1, &index);  //pic_parameter_set_id
            cur_frame_num = read_n_bits(bits + 1, &index, log2_max_frame_num); //frame_num
            interlace = 0;  // 不是场nalu

            if (key_filed_cnt > 1) //如果出现过1个关键帧或者2个关键场
            {
                if (!interlace)
                {
                    fseek(inpf, (oldpos - header_len), SEEK_SET); //文件指针恢复到原来位置
                    *frm_len = frame_strm_len - header_len;
                    return 1;
                }
                else
                {
                    if (have_top && have_bottom)
                    {
                        fseek(inpf, (oldpos - header_len), SEEK_SET); //文件指针恢复到原来位置
                        *frm_len = frame_strm_len - header_len;
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
            else
            {
                if (!interlace)
                {
                    key_filed_cnt += 2;
                }
                else
                {
                    key_filed_cnt += 1;
                    if (top_field)
                    {
                        have_top = 1;
                    }
                    else
                    {
                        have_bottom = 1;
                    }
                }
            }
        }

        if (IS_NO_FRM_CODE(marker))
        {
            if (is_patch_before)
            {
                header_len = (HLM_S32)len;
            }
            else
            {
                header_len += (HLM_S32)len;
            }
        }

        is_patch_before = !(IS_NO_FRM_CODE(marker));
        pre_frame_num = cur_frame_num;
        frame_strm_len += (HLM_S32)len;
        streambuf += len;
    }

    return 0;
}

// 打印help信息
HLM_VOID show_help()
{
    printf("    infile must be HLM bitstream \n");
    printf("    outfile is raw AVC bitstream\n");
    printf("\nExecutable Options:\n");
    printf("   -h/-help                           show HLMD help info.\n");
    printf("\nInput Parameters:\n");
    printf("   -i <filename>                  sets HLM bitstream input file name.\n");
    printf("   -y <integer>                   sets output yuv file enable flag.\n");
}

HLM_S32 main(HLM_S32 argc, char *argv[])
{
    HLM_S32           i                         = 0;
    HLM_S32           luma_size                 = 0;
    HLM_S32           chroma_size               = 0;
    HLM_S32           cu_width                  = 0;
    HLM_S32           cu_height                 = 0;
    HLM_S32           blk_status_size           = 0;
    HLM_S32           img_size                  = 0;
    HLM_S32           frame_cus_only            = 1;    // 默认为1，不考虑场
    HLM_S32           log2_max_frame_num        = 0;
    HLM_STATUS        sts                       = HLM_STS_OK;
    HLM_STATUS        read_ret                  = HLM_STS_OK;
    HLM_S32           yuv_out                   = 0;
    HLM_S32           tmp_buf_size              = 0;
    HLM_U16          *tmp_buf                   = HLM_NULL;
    HLM_U08          *p_bsm                     = HLM_NULL;
    HLM_S32           bsm_len                   = 0;
    size_t            elem_len                  = 0;
    char             *put                       = HLM_NULL;
    char              tmp_file[256]             = { 0 };
    FILE             *hlm_file                  = HLM_NULL;  // 码流文件
    HLM_VOID         *handle                    = HLM_NULL;
    HLM_MEM_TAB       mem_tab[HLM_MEM_TAB_NUM]  = { 0 };
    HLMD_VIDEO_INFO   hlm_video_info            = { 0 };
    HLM_U16          *yuv[3]                    = { 0 };
    DEMO_USER_DATA    user_data                 = { 0 };
    HLMD_STREAM_IN    stream_in                 = { 0 };
    HLM_S32           in_size                   = 0;
    HLMD_PROCESS_OUT  decode_out                = { 0 };
    HLM_S32           out_size                  = 0;
    HLM_S32           luma_size1                = 0;
    HLM_U32           chroma_offset_w           = 0;
    HLM_U32           chroma_offset_h           = 0;
    HLM_S32           chroma_size1              = 0;
    HLM_U16           chroma_crop_top           = 0;
    HLM_U16           chroma_crop_bottom        = 0;
    HLM_U16           chroma_crop_left          = 0;
    HLM_S32           j                         = 0;
    HLM_U08           byte_size                 = 0;

    // 解析参数
    if (argc <= 1)
    {
        show_help();
        sts = HLM_STS_ERR;
        fprintf(stderr, "Invalid input parameters!!!\n");
    }

    for (i = 1; i < argc; i += 2)
    {
        if ((0 == strncmp(argv[i], "-h", 2)) || (0 == strncmp(argv[i], "-help", 5)))
        {
            show_help();
            return HLM_STS_ERR;
        }
        if (0 == strncmp(argv[i], "-i", 2))
        {
            strcpy(tmp_file, argv[i + 1]);
            hlm_file = fopen(argv[i + 1], "rb");
            if (HLM_NULL == hlm_file)
            {
                sts = HLM_STS_ERR;
                fprintf(stderr, "Open HLM bitstream file <%s> failed!!!\n", argv[i + 1]);
            }
        }
        if (0 == strncmp(argv[i], "-y", 2))
        {
            yuv_out = atoi(argv[i + 1]);
        }
    }

    // 预解析获取视频信息
    put = strrchr(tmp_file, '.');
    elem_len = strlen(tmp_file) - strlen(put);
    sprintf(tmp_file + elem_len, ".log");
    sts = HLMD_DEMO_get_video_info(hlm_file, &hlm_video_info, HLM_NULL);

    // 给全局变量赋值用于后面分配内存
    ability.bit_depth_luma   = hlm_video_info.bit_depth_luma;
    ability.bit_depth_chroma = hlm_video_info.bit_depth_chroma;
    ability.code_width[0]    = hlm_video_info.code_width[0];
    ability.code_height[0]   = hlm_video_info.code_height[0];
    ability.code_width[1]    = hlm_video_info.code_width[1];
    ability.code_height[1]   = hlm_video_info.code_height[2];
    ability.code_width[2]    = hlm_video_info.code_width[2];
    ability.code_height[2]   = hlm_video_info.code_height[2];
    ability.ref_frm_num      = hlm_video_info.ref_frm_num;
    user_data.crop_left      = hlm_video_info.crop_left;
    user_data.crop_right     = hlm_video_info.crop_right;
    user_data.crop_top       = hlm_video_info.crop_top;
    user_data.crop_bottom    = hlm_video_info.crop_bottom;

    // 修改解码图像文件名，格式为“码流文件名_dec.yuv"
    if (hlm_video_info.format == HLM_IMG_RGB)
    {
        sprintf(tmp_file + elem_len, "_dec.rgb");
    }
    else
    {
        sprintf(tmp_file + elem_len, "_dec.yuv");
    }
    user_data.fp_dec = fopen(tmp_file, "wb");
    if (HLM_NULL == user_data.fp_dec)
    {
        goto MAIN_EXIT;
    }
    user_data.yuv_output_enable = yuv_out;

    // 分配内存
    sts = HLMD_DEMO_create_handle(mem_tab, &handle, user_data.fp_log, &hlm_video_info);

    // 分配码流缓冲区、解码输出yuv缓冲区
    luma_size    = HLM_SIZE_ALIGN_16(hlm_video_info.code_width[0]) * HLM_SIZE_ALIGN_16(hlm_video_info.code_height[0]);
    luma_size   += 16;
    chroma_size  = HLM_SIZE_ALIGN_16(hlm_video_info.code_width[1]) * HLM_SIZE_ALIGN_16(hlm_video_info.code_height[1]);
    chroma_size += 16;
    img_size     = luma_size + (chroma_size << 1);
    cu_width     = (hlm_video_info.code_width[0] >> 4);
    cu_height    = (hlm_video_info.code_height[0] >> 3);
    switch (hlm_video_info.format)
    {
    case HLM_IMG_YUV_444:
    case HLM_IMG_RGB:
        chroma_offset_w = 0;
        chroma_offset_h = 0;
        break;
    case HLM_IMG_YUV_420:
        chroma_offset_w = 1;
        chroma_offset_h = 1;
        break;
    case HLM_IMG_YUV_422:
        chroma_offset_w = 1;
        chroma_offset_h = 0;
        break;
    }

    blk_status_size = HLM_SIZE_ALIGN_16(cu_height * cu_width);
    tmp_buf_size    = (img_size << 1) + blk_status_size;
    tmp_buf         = (HLM_U16 *)HLM_MEM_Malloc(tmp_buf_size*sizeof(HLM_U16), 16);
    if (HLM_NULL == tmp_buf)
    {
        goto MAIN_EXIT;
    }

    // 初始化
    p_bsm  = (HLM_U08*)tmp_buf;
    yuv[0] = tmp_buf + img_size;
    yuv[1] = yuv[0] + luma_size;
    yuv[2] = yuv[1] + chroma_size;

    decode_out.is_mismatch = 0;
    decode_out.finish_one_frame = 0;
    while (!feof(hlm_file))
    {
        read_ret = HLMD_DEMO_read_one_frame(p_bsm, &bsm_len, hlm_file, log2_max_frame_num, frame_cus_only, user_data.fp_log);
        if (0 == read_ret)
        {
            break;
        }

        // 输入输出转化
        stream_in.stream_buf = p_bsm;
        stream_in.stream_len = bsm_len;
        in_size = sizeof(HLMD_STREAM_IN);

        decode_out.image_out.data[0] = yuv[0];
        decode_out.image_out.data[1] = yuv[1];
        decode_out.image_out.data[2] = yuv[2];
        out_size = sizeof(HLMD_PROCESS_OUT);

        // 解码一帧
        sts = HLMD_LIB_DecodeFrame(handle, (HLM_VOID *)&stream_in, in_size, (HLM_VOID *)&decode_out, out_size);
        if (sts != HLM_STS_OK)
        {
            decode_out.is_mismatch = 1;
            break;
        }

        // 每次解码一个patch，在解完一帧的所有patch之后，写一次重建
        if (user_data.yuv_output_enable && decode_out.finish_one_frame)
        {
            decode_out.finish_one_frame = 0;  // 用于下一帧
            if (hlm_video_info.format == HLM_IMG_RGB)
            {
                HLM_COM_YCbCr_To_Rgb(yuv[0], yuv[1], yuv[2],
                    hlm_video_info.code_width[0], hlm_video_info.code_height[0], ability.bit_depth_luma);
            }
            luma_size1 = (ability.code_width[0] - user_data.crop_left - user_data.crop_right);
            byte_size = ability.bit_depth_luma == 8 ? 1 : 2;
            for (i = user_data.crop_top; i < (hlm_video_info.code_height[0] - user_data.crop_bottom); i++)
            {
                for (j = 0; j < luma_size1; j++)
                {
                    fwrite((HLM_U16*)decode_out.image_out.data[0] + (i * hlm_video_info.code_width[0]) + user_data.crop_left + j,
                        byte_size, 1, user_data.fp_dec);
                }
            }

            if (hlm_video_info.format != HLM_IMG_YUV_400)
            {
                chroma_size1       = luma_size1 >> chroma_offset_w;
                chroma_crop_top    = user_data.crop_top >> chroma_offset_h;
                chroma_crop_bottom = user_data.crop_bottom >> chroma_offset_h;
                chroma_crop_left   = user_data.crop_left >> chroma_offset_w;
                for (i = chroma_crop_top; i < (hlm_video_info.code_height[1] - chroma_crop_bottom); i++)
                {
                    for (j = 0; j < chroma_size1; j++)
                    {
                        fwrite((HLM_U16*)decode_out.image_out.data[1] + (i * hlm_video_info.code_width[1]) + (chroma_crop_left)+j,
                            byte_size, 1, user_data.fp_dec);
                    }
                }
                for (i = chroma_crop_top; i < (hlm_video_info.code_height[1] - chroma_crop_bottom); i++)
                {
                    for (j = 0; j < chroma_size1; j++)
                    {
                        fwrite((HLM_U16*)decode_out.image_out.data[2] + (i * hlm_video_info.code_width[1]) + (chroma_crop_left)+j,
                            byte_size, 1, user_data.fp_dec);
                    }
                }
            }
        }
    }

    if (decode_out.is_mismatch == 1)
    {
        printf("[DEMO] Mismatch.\n");
    }
    else
    {
        printf("[DEMO] Matched.\n");
    }

MAIN_EXIT:

    // 释放内存
    sts = HLMD_DEMO_destroy_handle(mem_tab, handle, user_data.fp_log);

    if (tmp_buf != HLM_NULL)
    {
        HLM_MEM_Free(tmp_buf);
        tmp_buf = HLM_NULL;
    }
    if (hlm_file != HLM_NULL)
    {
        fclose(hlm_file);
        hlm_file = HLM_NULL;
    }
    if (user_data.fp_dec != HLM_NULL)
    {
        fclose(user_data.fp_dec);
        user_data.fp_dec = HLM_NULL;
    }

    return sts;
}
