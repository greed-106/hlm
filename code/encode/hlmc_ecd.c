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
#include "hlmc_ecd.h"
#include "hlmc_intra.h"
#include "hlmc_common.h"

/***************************************************************************************************
* 功  能：初始化码流信息结构体
* 参  数：* 
*         stream_buf                -I         码流缓冲区地址
*         stream_buf_size           -I         码流缓冲区长度
*         bs                        -O         码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_InitBitstream(HLM_U08         *stream_buf,
                                HLM_S32          stream_buf_size,
                                HLMC_BITSTREAM  *bs)
{
    bs->bits_left  = 32;
    bs->byte_cache = 0;
    bs->bit_size   = ((stream_buf_size - HLMC_STREAM_BUF_RSV) << 3);
    bs->bit_cnt    = 0;
    bs->emul_bytes = 0;
    bs->ptr_start  = bs->ptr = stream_buf;
}

/***************************************************************************************************
* 功  能：编码四字节的起始码0x00 00 00 01
* 参  数：* 
*         bs                -I        码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_PutStartCode4Byte(HLMC_BITSTREAM *bs)
{
    *bs->ptr++   = 0;       // BYTE STREAM: leadin_zero_8bits
    *bs->ptr++   = 0;       // BYTE STREAM: Start_code_prefix
    *bs->ptr++   = 0;       // BYTE STREAM: Start_code_prefix
    *bs->ptr++   = 1;       // BYTE STREAM: Start_code_prefix
    bs->bit_cnt += 32;
}

#if START_CODE_FIX
/***************************************************************************************************
* 功  能：编码三字节的起始码0x00 00 01
* 参  数：*
*         bs                -I        码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_PutStartCode3Byte(HLMC_BITSTREAM *bs)
{
    *bs->ptr++   = 0;       // BYTE STREAM: leadin_zero_8bits
    *bs->ptr++   = 0;       // BYTE STREAM: Start_code_prefix
    *bs->ptr++   = 1;       // BYTE STREAM: Start_code_prefix
    bs->bit_cnt += 24;
}
#endif

/***************************************************************************************************
* 功  能：生成nal_unit_header
* 参  数：*
*         nal_unit_type             -I         nalu类型
*         nal_ref_idc               -I         nalu参考类型索引
*         bs                        -O         码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_PutNaluHeader(HLM_NAL_UNIT_TYPE   nal_unit_type,
                                HLM_U32             nal_ref_idc,
                                HLMC_BITSTREAM     *bs)
{
    HLMC_ECD_PutShortBits(bs, 0, 1, "forbidden_zero_bit");
    HLMC_ECD_PutShortBits(bs, nal_ref_idc, 2, "nal_ref_idc");
    HLMC_ECD_PutShortBits(bs, nal_unit_type, 5, "nal_unit_type");
}

/***************************************************************************************************
* 功  能：编码不超过8bit的无符号整数
* 参  数：* 
*         bs                -I         码流信息结构体
*         code              -I         待编码字符
*         len               -I         字符bit数
*         syntax_element    -I         语法元素名称，加上该字符串是为了便于阅读，函数中用不到
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_PutShortBits(HLMC_BITSTREAM  *bs,
                               HLM_U32          code,
                               HLM_S32          len,
                               const HLM_S08   *syntax_element)
{
    bs->bits_left -= len;

    // 如果byte_cache中剩余比特数超过len，那么直接把code写在byte_cache中
    if (bs->bits_left > 0)
    {
        bs->byte_cache |= (code << bs->bits_left);
        return;
    }

    // 如果byte_cache中剩余比特数不足len，那么先把byte_cache中最高字节写入码流
    *bs->ptr++      = (bs->byte_cache >> 24);
    bs->bit_cnt    += 8;
    bs->bits_left  += 8;
    bs->byte_cache  = ((bs->byte_cache << 8) | (code << bs->bits_left));
}

/***************************************************************************************************
* 功  能：编码不超过32bit的无符号整数
* 参  数：* 
*         bs                -I         码流信息结构体
*         code              -I         待编码字符
*         len               -I         字符bit数
*         syntax_element    -I         语法元素名称，加上该字符串是为了便于阅读，函数中用不到
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_PutLongBits(HLMC_BITSTREAM  *bs,
                              HLM_U32          code,
                              HLM_S32          len,
                              const HLM_S08   *syntax_element)
{
    HLM_S32 len_tmp   = 0;           // 本次编码的bit数
    HLM_S32 len_bytes = 24;          // len中整数byte的bit数
    HLM_S32 len_bits  = len;         // len中整数byte以外的bit数

    assert(len < 64);  // 当指数哥伦布编码残差时，可能存在len大于32情况，此时code前缀都是0
    while (len_bits > 32)
    {
        HLMC_ECD_PutShortBits(bs, 0, 1, HLM_NULL);
        len_bits--;
    }

     while (len_bits)
     {
         if (len_bits > len_bytes)
         {
             len_tmp = len_bits - len_bytes;
             HLMC_ECD_PutShortBits(bs, code >> len_bytes, len_tmp, HLM_NULL);
             len_bits -= len_tmp;
         }
         len_bytes -= 8;
     }
}

/***************************************************************************************************
* 功  能：无符号整数指数哥伦布编码
* 参  数：* 
*         bs                -I         码流信息结构体
*         code              -I         待编码字符
*         write_flag        -I         是否写入码流
*         syntax_element    -I         语法元素名称，加上该字符串是为了便于阅读，函数中用不到
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_ECD_PutUeBits(HLMC_BITSTREAM  *bs,
                           HLM_U32          code,
                           HLM_U08          write_flag,
                           const HLM_S08   *syntax_element)
{
    unsigned long idx      = 0;
    HLM_U32       length   = 0;
    HLM_U32       tmp_code = code + 1;

    while (tmp_code >> ++idx);  // 求Log2(tmp_code)

    length = idx * 2 - 1;
    if (write_flag)
    {
        HLMC_ECD_PutLongBits(bs, tmp_code, length, syntax_element);
    }

    return length;
}

/***************************************************************************************************
* 功  能：有符号整数指数哥伦布编码
* 参  数：* 
*         bs                -I         码流信息结构体
*         code              -I         待编码字符
*         write_flag        -I         是否写入码流
*         syntax_element    -I         语法元素名称，加上该字符串是为了便于阅读，函数中用不到
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_ECD_PutSeBits(HLMC_BITSTREAM  *bs,
                           HLM_S32          code,
                           HLM_U08          write_flag,
                           const HLM_S08   *syntax_element)
{
    unsigned long idx      = 0;
    HLM_U32       length   = 0;
    HLM_U32       tmp_code = code + 1;

    if (code > 0)
    {
        tmp_code = 2 * code;
    }
    else
    {
        tmp_code = -2 * code + 1;
    }

    while (tmp_code >> ++idx);  // 求Log2(tmp_code)

    length = idx * 2 - 1;
    if (write_flag)
    {
        HLMC_ECD_PutLongBits(bs, tmp_code, length, HLM_NULL);
    }

    return length;
}

/***************************************************************************************************
* 功  能：拖尾比特编码 
* 参  数：* 
*         bs                -I         码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_RbspTrailingBits(HLMC_BITSTREAM *bs)
{
    HLMC_ECD_PutShortBits(bs, 1,                     1, "rbsp_stop_one_bit");
    HLMC_ECD_PutShortBits(bs, 0, (bs->bits_left & 0x7), "rbsp_alignment_zero_bit");

    while (bs->bits_left < 32)
    {
        *bs->ptr++       = bs->byte_cache >> 24;
        bs->bit_cnt     += 8;
        bs->bits_left   += 8;
        bs->byte_cache <<= 8;
    }
}

/***************************************************************************************************
* 功  能：在整个码流中添加防竞争码
* 参  数：*
*         src_bs                -I           不包含防竞争码的码流信息结构体
*         dst_bs                -I           包含防竞争码的码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_WriteEmulaPreventBytes(HLMC_BITSTREAM *src_bs,
                                         HLMC_BITSTREAM *dst_bs)
{
    HLM_U32 bits_cnt  = 0;
    HLM_U32 num_bytes = 0;
    HLM_U32 i         = 0;
    HLM_U32 bytes     = 0;
    HLM_U08 *ori      = HLM_NULL;

    bits_cnt  = src_bs->bit_cnt;
    num_bytes = bits_cnt / 8;
    ori       = src_bs->ptr_start;

    //首先写入，排除起始码的干扰
    for (i = 0; i < 4; i++)
    {
        dst_bs->ptr[bytes++] = ori[i];
    }

    //起始码之后的部分需要添加防竞争码
    for (i = 4; i < num_bytes; i++)
    {
        dst_bs->ptr[bytes++] = ori[i];
        if (i > 4 + 2 && !dst_bs->ptr[bytes - 2] && !dst_bs->ptr[bytes - 3] && dst_bs->ptr[bytes - 1] <= 0x03)
        {
            dst_bs->ptr[bytes] = dst_bs->ptr[bytes - 1];
            dst_bs->ptr[bytes - 1] = 0x03;
            bytes++;
            src_bs->bit_cnt += 8;
        }
    }

    //检查最后一个比特
    if (!dst_bs->ptr[bytes - 1])
    {
        dst_bs->ptr[bytes++] = 0x03;
        src_bs->bit_cnt += 8;
    }
    dst_bs->ptr += bytes;
}

// 分组定长编码
HLM_U32 HLMC_ECD_write_gfl(HLMC_BITSTREAM  *bs,
                           HLM_S16         *value,
                           HLM_U32          level,
                           HLM_U32          start,
                           HLM_U32          end,
                           HLM_U08          write_flag,
                           HLM_U32          skip_flag)
{
    HLM_U32 step   = 1 << level;
    HLM_U32 cost   = 0;
    HLM_U32 skip   = 0;
    HLM_U32 temp_0 = 0;
    HLM_U32 temp_1 = 0;
    HLM_U32 half   = 0;
    HLM_U32 i      = 0;
    HLM_U32 j      = 0;

    for (i = start; i < end; i += step)
    {
        step = (end + 1) > i + step + step ? step : end - i;
        if (!skip_flag && step > 3)
        {
            temp_0 = 0;
            temp_1 = 0;
            half = step >> 1;
            for (j = 0; j < half; j++)
            {
                temp_0 += (value[i + j] != 0);
            }
            for (j = half; j < step; j++)
            {
                temp_1 += (value[i + j] != 0);
            }
            if ((temp_1 + temp_0) == 0)
            {
                cost += 1;
                if (write_flag)
                {
                    HLMC_ECD_PutLongBits(bs, 0, 1, "group_merge");
                }
            }
            else if (temp_1 == 0 || temp_0 == 0)
            {
                if (temp_1 == 0)
                {
                    cost += 3 + half;
                    if (write_flag)
                    {
                        HLMC_ECD_PutLongBits(bs, 6, 3, "group_merge");
                        for (j = 0; j < half; j++)
                        {
                            HLMC_ECD_PutLongBits(bs, value[i + j] == 0, 1, "group_value");
                        }
                    }
                }
                if (temp_0 == 0)
                {
                    cost += 3 + step - half;
                    if (write_flag)
                    {
                        HLMC_ECD_PutLongBits(bs, 7, 3, "group_merge");
                        for (j = half; j < step; j++)
                        {
                            HLMC_ECD_PutLongBits(bs, value[i + j] == 0, 1, "group_value");
                        }
                    }
                }
            }
            else
            {
                cost += 2 + step;
                if (write_flag)
                {
                    HLMC_ECD_PutLongBits(bs, 2, 2, "group_split_flag");
                    for (j = 0; j < step; j++)
                    {
                        HLMC_ECD_PutLongBits(bs, value[i + j] == 0, 1, "group_value");
                    }
                }
            }
        }
        else
        {
            cost += step;
            if (write_flag)
            {
                for (j = 0; j < step; j++)
                {
                    HLMC_ECD_PutLongBits(bs, value[i + j] == 0, 1, "group_value");
                }
            }

        }
    }
    return cost;
}

// 系数二值化，获取gt0、gt1、符号、剩余值
HLM_VOID HLMC_ECD_coeff_binary(HLM_S16    *gt0,
                               HLM_S16    *gt1,
                               HLM_S16    *sign,
                               HLM_COEFF  *remain,
                               HLM_COEFF  *tmp_value,
                               HLM_U32    *num_zero,
                               HLM_U32    *num_sign,
                               HLM_U32    *num_one,
                               HLM_U32    *num_two,
                               HLM_U32     num_value)
{
    HLM_U08 i        = 0;
    HLM_U08 j        = 0;
    HLM_U08 k        = 0;
    HLM_S08 number_1 = 1;
    HLM_S08 gt1_temp = 1;

    for (j = 0; j < num_value; j++)
    {
        if (tmp_value[j] != 0)
        {
            gt0[num_zero[0]++] = number_1;
            sign[num_sign[0]++] = (tmp_value[j] > 0) ? 0 : number_1;
            if (HLM_ABS(tmp_value[j]) - gt1_temp != 0)
            {
                gt1[num_one[0]++] = number_1;
                remain[num_two[0]++] = HLM_ABS(tmp_value[j]) - 1 - gt1_temp;
            }
            else
            {
                gt1[num_one[0]++] = 0;
            }
        }
        else
        {
            gt0[num_zero[0]++] = 0;
        }
    }
}

// 编码系数二值化后的bin
HLM_U32 HLMC_ECD_write_coeff_bin(HLMC_BITSTREAM  *bs,
                                 HLM_S16         *gt0,
                                 HLM_S16         *gt1,
                                 HLM_S16         *sign,
                                 HLM_COEFF       *remain,
                                 HLM_COEFF        gt0_num,
                                 HLM_COEFF        gt1_num,
                                 HLM_COEFF        sign_num,
                                 HLM_COEFF        remain_num,
                                 HLM_U08          ac_flag,
                                 HLM_U08          write_flag)
{
    HLM_U32 cost_bit                    = 0;
    HLM_U32 cost_bit_best               = 0;
    HLM_U32 level                       = 0;
    HLM_U32 t                           = 0;
    HLM_U32 level_last[HLM_TU_4x4_NUMS] = { 3,3,4,4,4,4 };
    HLM_S16 StartPos                    = 0;
    HLM_S16 EndPos                      = 0;
    HLM_U32 group                       = (gt0_num + 16) >> 5;
    HLM_U32 start_t                     = 0;
    HLM_S16 i                           = 0;

    if (ac_flag)
    {
        for (t = start_t; t < group; t++)
        {
            cost_bit = 0;
            cost_bit_best = 1000000;
            StartPos = 32 * t;
            EndPos = 32 * (t + 1) > (HLM_U32)gt0_num ? gt0_num : 32 * (t + 1);
            for (level = 3; level < 6; level += 2)
            {
                cost_bit = HLMC_ECD_write_gfl(bs, gt0, level, StartPos, EndPos, 0, 0);
                if (cost_bit < cost_bit_best)
                {
                    cost_bit_best = cost_bit;
                    level_last[t] = level;
                }
            }
        }
        cost_bit = 0;
        for (t = start_t; t < group; t++)
        {
            StartPos = 32 * t;
            EndPos = 32 * (t + 1) > (HLM_U32)gt0_num ? gt0_num : 32 * (t + 1);
            cost_bit += 1;
            if (write_flag)
            {
                HLMC_ECD_PutLongBits(bs, level_last[t] == 3, 1, "group_merge");
            }
            cost_bit += HLMC_ECD_write_gfl(bs, gt0, level_last[t], StartPos, EndPos, write_flag, 0);
        }
        cost_bit += HLMC_ECD_write_gfl(bs, gt1, 4, 0, gt1_num, write_flag, 0);
        cost_bit += HLMC_ECD_write_gfl(bs, sign, 6, 0, sign_num, write_flag, 1);
    }
    else
    {
        cost_bit += HLMC_ECD_write_gfl(bs, gt0, 3, 0, gt0_num, write_flag, 0);
        cost_bit += HLMC_ECD_write_gfl(bs, gt1, 2, 0, gt1_num, write_flag, 1);
        cost_bit += HLMC_ECD_write_gfl(bs, sign, 2, 0, sign_num, write_flag, 1);
    }

    for (i = 0; i < remain_num; i++)
    {
        cost_bit += HLMC_ECD_PutUeBits(bs, remain[i], write_flag, "remain");
    }

    return cost_bit;
}

// 编码dc
HLM_VOID HLMC_ECD_write_dc(HLMC_CU_INFO           *cur_cu,
                           HLM_NEIGHBOR_INFO      *nbi_info,
                           HLM_COMPONENT           plane,
                           HLMC_BITSTREAM         *bs,
                           HLM_U32                *total_bit,
                           HLM_U08                 write_flag)
{
    HLM_COEFF tmp_value[HLM_TU_4x4_NUMS] = { 0 };
    HLM_COEFF tmp_coeff[HLM_TU_4x4_NUMS] = { 0 };
    HLM_S16 gt0        [HLM_TU_4x4_NUMS] = { 0 };
    HLM_S16 sign       [HLM_TU_4x4_NUMS] = { 0 };
    HLM_S16 gt1        [HLM_TU_4x4_NUMS] = { 0 };
    HLM_COEFF remain   [HLM_TU_4x4_NUMS] = { 0 };
    HLM_COEFF *praster                   = &tmp_coeff[0];
    HLM_U08 log_w                        = cur_cu->com_cu_info.cu_width[plane];
    HLM_U08 log_h                        = cur_cu->com_cu_info.cu_height[plane];
    HLM_U08 w                            = 1 << log_w;
    HLM_U08 h                            = 1 << log_h;
    HLM_U08 max_coeff_num                = 1 << (log_w + log_h - 4);  // DC的个数
    HLM_S32 num_zero                     = 0;
    HLM_S32 num_one                      = 0;
    HLM_S32 num_sign                     = 0;
    HLM_S32 num_two                      = 0;
    HLM_U08 i                            = 0;
    HLM_U08 j                            = 0;
    HLM_U08 k                            = 0;

    for (j = 0; j < h; j += 4)
    {
        for (i = 0; i < w; i += 4)
        {
            praster[k++] = cur_cu->com_cu_info.cu_pred_info.coeff[plane][j * HLM_WIDTH_SIZE + i];
        }
    }
    for (k = 0; k < max_coeff_num; k++)
    {
        if (max_coeff_num == HLM_TU_4x4_NUMS)
        {
            tmp_value[k] = praster[HLM_INTRA_RASTER_TO_ZSCAN[k]];
        }
        else
        {
            tmp_value[k] = praster[k];
        }
    }

    HLMC_ECD_coeff_binary(gt0, gt1, sign, remain, tmp_value, &num_zero, &num_sign, &num_one, &num_two, max_coeff_num);
    total_bit[0] += HLMC_ECD_write_coeff_bin(bs, gt0, gt1, sign, remain, num_zero, num_sign, num_one, num_two, 0, write_flag);
}

// 编码ac
HLM_VOID HLMC_ECD_write_ac(HLMC_CU_INFO           *cur_cu,
                           HLM_NEIGHBOR_INFO      *nbi_info,
                           HLM_COMPONENT           plane,
                           HLMC_BITSTREAM         *bs,
                           HLM_U32                *total_bit,
                           HLM_U08                 write_flag)
{
    HLM_S16 gt0          [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_S16 sign         [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_S16 gt1          [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_COEFF remain     [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_COEFF tmp_value  [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_COEFF tmp_value_1[HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_U08 x                                   = 0;
    HLM_U08 y                                   = 0;
    HLM_U08 i                                   = 0;
    HLM_U08 j                                   = 0;
    HLM_U08 k                                   = 0;
    HLM_U08 max_coeff_num                       = (!cur_cu->com_cu_info.ts_flag) ? 15 : 16;
    HLM_S32 num_zero                            = 0;
    HLM_S32 num_one                             = 0;
    HLM_S32 num_sign                            = 0;
    HLM_S32 num_two                             = 0;
    HLM_COEFF *tmp_p                            = HLM_NULL;
    HLM_U08 log_w                               = cur_cu->com_cu_info.cu_width[plane];
    HLM_U08 log_h                               = cur_cu->com_cu_info.cu_height[plane];
    HLM_U16 DC_number                           = 1 << (log_w + log_h - 4);
    HLM_U16 pu_width                            = 4;
    HLM_U16 pu_height                           = 4;
    HLM_U16 block_4x4                           = 0;
    HLM_U32 cost_tmp                            = 0;
    HLM_U32 cost_tmp_1                          = 0;

    for (i = 0; i < (1 << log_h); i += pu_height)
    {
        for (j = 0; j < (1 << log_w); j += pu_width)
        {
            block_4x4 = ((i / pu_height) * (1 << (log_w - 2)) + j / pu_width);
            k = 0;
            for (y = 0; y < pu_height; y++)
            {
                for (x = 0; x < pu_width; x++)
                {
                    if (y == 0 && x == 0 && !cur_cu->com_cu_info.ts_flag)
                    {
                        continue;
                    }
                    tmp_value[block_4x4* max_coeff_num + k] = cur_cu->com_cu_info.cu_pred_info.coeff[plane][(i + y) * HLM_WIDTH_SIZE + j + x];
                    tmp_value_1[k * DC_number + block_4x4] = cur_cu->com_cu_info.cu_pred_info.coeff[plane][(i + y) * HLM_WIDTH_SIZE + j + x];
                    if (tmp_value[block_4x4* max_coeff_num + k] != 0)
                    {
                        num_one++;
                    }
                    k++;
                }
            }
        }
    }

    if (!cur_cu->com_cu_info.ts_flag)
    {
        total_bit[0] += 1;
        if (write_flag)
        {
            HLMC_ECD_PutLongBits(bs, num_one == 0, 1, "ac_zero_flag");
        }
        if (num_one == 0)
        {
            return;
        }
    }

    total_bit[0] += 1;
    num_zero = num_sign = num_one = num_two = 0;
    HLMC_ECD_coeff_binary(gt0, gt1, sign, remain, tmp_value, &num_zero, &num_sign, &num_one, &num_two, max_coeff_num * DC_number);
    cost_tmp = HLMC_ECD_write_coeff_bin(bs, gt0, gt1, sign, remain, num_zero, num_sign, num_one, num_two, 1, 0);

    num_zero = num_sign = num_one = num_two = 0;
    HLMC_ECD_coeff_binary(gt0, gt1, sign, remain, tmp_value_1, &num_zero, &num_sign, &num_one, &num_two, max_coeff_num * DC_number);
    cost_tmp_1 = HLMC_ECD_write_coeff_bin(bs, gt0, gt1, sign, remain, num_zero, num_sign, num_one, num_two, 1, 0);

    if (write_flag)
    {
        HLMC_ECD_PutLongBits(bs, (cost_tmp <= cost_tmp_1), 1, "scan_flag");
    }
    tmp_p = cost_tmp <= cost_tmp_1 ? &tmp_value[0] : &tmp_value_1[0];
    num_zero = num_sign = num_one = num_two = 0;
    HLMC_ECD_coeff_binary(gt0, gt1, sign, remain, tmp_p, &num_zero, &num_sign, &num_one, &num_two, max_coeff_num * DC_number);
    total_bit[0] += HLMC_ECD_write_coeff_bin(bs, gt0, gt1, sign, remain, num_zero, num_sign, num_one, num_two, 1, write_flag);
}

// 16x8宏块残差系数编码
HLM_VOID HLMC_ECD_write_coeff_16x8(HLMC_CU_INFO              *cur_cu,
                                   HLM_NEIGHBOR_INFO         *nbi_info,
                                   HLM_COMPONENT              plane,
                                   HLMC_BITSTREAM            *bs)
{
    HLM_U32 cost = 0;

    if (cur_cu->com_cu_info.cbf[plane])
    {
        if (!cur_cu->com_cu_info.ts_flag)
        {
            HLMC_ECD_write_dc(cur_cu, nbi_info, plane, bs, &cost, 1);
        }
        HLMC_ECD_write_ac(cur_cu, nbi_info, plane, bs, &cost, 1);
#if LINE_BY_LINE_4x1_RESI 
        if (cur_cu->com_cu_info.cu_type == HLM_I_LINE || cur_cu->com_cu_info.cu_type == HLM_I_ROW)
            HLMC_ECD_PutShortBits(bs, cur_cu->com_cu_info.cu_line_resi[plane], 2, "resi_pred_mode");
#endif
    }
}


// 编码intra16x8的预测模式
HLM_VOID HLMC_ECD_write_pred_mode_i16x8(HLM_U08          pred_mode,
                                        HLM_U08          mpm,
                                        HLMC_BITSTREAM  *bs)
{
    HLM_U32 bits_len  = 0;
    HLM_U32 bits_code = 0;

    if (pred_mode == mpm)
    {
        bits_len = 1;
        bits_code = 1;
    }
    else
    {
        if (pred_mode > mpm)
        {
            bits_code = pred_mode - 1;
        }
        else
        {
            bits_code = pred_mode;
        }
        bits_len = 2 + (bits_code != 0);
        bits_code += (bits_code != 0);
    }
    HLMC_ECD_PutShortBits(bs, bits_code, bits_len, "intra_pred_mode");
}

/***************************************************************************************************
* 功  能：cu_type编码和比特估计
* 参  数：*
*        cu_type             -I                   CU类型
*        frame_type          -I                   帧类型
*        write_flag          -I                   是否写码流
*        bs                  -IO                  码流结构体
*        ibc_enable_flag     -I                   ibc使能开关
* 返回值：比特长度
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_ECD_EncodeCuType(HLM_CU_TYPE          cu_type,
                              HLM_FRAME_TYPE       frame_type,
                              HLM_U08              write_flag,
                              HLMC_BITSTREAM      *bs,
                              HLM_U32              ibc_enable_flag)
{
    HLM_U32 bits_len       = 0;
    HLM_U08 intra_flag     = 0;
    HLM_U08 ibc_flag       = 0;
    HLM_U08 partition_flag = 0;
#if  LINE_BY_LINE
    HLM_U08 intra_pred_mode = cu_type;
#endif
    intra_flag     = (cu_type == HLM_P_16x8 || cu_type == HLM_P_8x8) ? 0 : 1;
    ibc_flag       = (cu_type == HLM_IBC_4x4) ? 1 : 0;
    partition_flag = (cu_type == HLM_P_16x8 || cu_type == HLM_I_16x8) ? 0 : 1;
    if (frame_type == HLM_FRAME_TYPE_P)
    {
        if (write_flag)
        {
            HLMC_ECD_PutShortBits(bs, intra_flag, 1, "intra_flag");
        }
        bits_len++;
    }
    else
    {
        assert(intra_flag == 1);
    }
    if (intra_flag)
    {
        if (ibc_enable_flag)
        {
            if (write_flag)
            {
                HLMC_ECD_PutShortBits(bs, ibc_flag, 1, "ibc_flag");
            }
            bits_len++;
        }
        else
        {
            assert(ibc_flag == 0);
        }
        if (!ibc_flag)
        {
            if (write_flag)
            {
#if  LINE_BY_LINE
                assert(intra_pred_mode < HLM_IBC_4x4);
                HLMC_ECD_PutShortBits(bs, intra_pred_mode, 2, "partition_flag");
#else
                HLMC_ECD_PutShortBits(bs, partition_flag, 1, "partition_flag");
#endif
            }
            bits_len++;
        }
    }
    else
    {
        if (write_flag)
        {
            HLMC_ECD_PutShortBits(bs, partition_flag, 1, "partition_flag");
        }
        bits_len++;
    }

    return bits_len;
}

// 编码intra4x4的预测模式
HLM_VOID HLMC_ECD_write_pred_mode_i4x4(HLM_U08               intra_pred_mode,
                                       HLM_U08               mpm,
                                       HLMC_BITSTREAM       *bs)
{
    HLM_U32 bits_len  = 0;
    HLM_U32 bits_code = 0;

    if (intra_pred_mode == mpm)
    {
        bits_len = 1;
        bits_code = 1;
    }
    else if (intra_pred_mode > mpm)
    {
        bits_len = 4;
        bits_code = intra_pred_mode - 1;
    }
    else
    {
        bits_len = 4;
        bits_code = intra_pred_mode;
    }
    HLMC_ECD_PutShortBits(bs, bits_code, bits_len, "intra_pred_mode");
}

// 编码cbf
HLM_VOID HLMC_ECD_write_cbf(HLM_CU_TYPE        cu_type,
                            HLM_U08            cbf[3],
                            HLM_U32            yuv_comp,
                            HLMC_BITSTREAM    *bs)
{
    HLM_U32 bits_len  = 0;
    HLM_U32 bits_code = 0;
    HLM_U32 y_cbf     = cbf[0];
    HLM_U32 cb_cbf    = cbf[1];
    HLM_U32 cr_cbf    = cbf[2];

    if (yuv_comp > 1)
    {
        HLMC_ECD_PutShortBits(bs, (y_cbf && cb_cbf && cr_cbf) ? 1 : 0, 1, "y_cbf");
        if ((y_cbf && cb_cbf && cr_cbf) == 0)
        {
            HLMC_ECD_PutShortBits(bs, y_cbf, 1, "y_cbf");
            HLMC_ECD_PutShortBits(bs, cb_cbf, 1, "cb_cbf");
            if ((y_cbf && cb_cbf) == 0)
            {
                HLMC_ECD_PutShortBits(bs, cr_cbf, 1, "cr_cbf");
            }
        }
    }
    else
    {
        HLMC_ECD_PutShortBits(bs, y_cbf, 1, "y_cbf");
    }
}

/***************************************************************************************************
* 功  能：BV编码和比特估计
* 参  数：*
*        cu_x                -I                   CU横坐标
*        merge_flag          -I                   merge子模式索引
*        zscan_idx           -I                   Z字型扫描顺序索引
*        bv                  -I                   bv
*        bs                  -O                   码流结构体
*        is_bitcount         -I                   是否做比特估计
* 返回值：比特长度
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_ECD_EncodeBV(HLM_U32            cu_x,
#if BVY_ZERO
                          HLM_U08            bvy_zero_flag,
#endif
                          HLM_U08            merge_flag,
                          HLM_U08            zscan_idx,
                          HLM_MV             bv[8],
                          HLMC_BITSTREAM    *bs,
                          HLM_U08            is_bitcount)
{
    HLM_U32 bits           = 0;
    HLM_S32 val            = 0;
    HLM_S32 bvx_bits       = (cu_x >= 8) ? HLM_IBC_HOR_SEARCH_LOG : tbl_bvx_bits[cu_x];
    HLM_U08 direct         = tbl_merge_type[merge_flag][zscan_idx];
    HLM_S32 pu_x           = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
    HLM_S32 pu_y           = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
    HLM_S32 bvx_offset     = ((zscan_idx & 1) + 1) << 2;
    HLM_S32 left_delta_idx = (zscan_idx == 4 || zscan_idx == 6) ? 3 : 1;

    // 校验bv范围
    val = -bv[zscan_idx].mvx - bvx_offset;
    assert(0 <= val && val <= 127);
    val = pu_y == 0 ? bv[zscan_idx].mvy : -bv[zscan_idx].mvy;
    assert(0 <= val && val <= 3);

    if (direct == 0)  // 0不merge
    {
        if (is_bitcount)
        {
            bits += bvx_bits;
#if BVY_ZERO
            if (!bvy_zero_flag)
            {
                bits += 2;
            }
#elif !BVY_FIX_LENGTH
            bits += 2;
#endif
        }
        else
        {
            val = -bv[zscan_idx].mvx - bvx_offset;
            HLMC_ECD_PutShortBits(bs, val, bvx_bits, "bv_x");
#if BVY_ZERO
            if (!bvy_zero_flag)
            {
                val = HLM_ABS(bv[zscan_idx].mvy);
                HLMC_ECD_PutShortBits(bs, val, 2, "bv_y");
            }
#elif !BVY_FIX_LENGTH
            val = HLM_ABS(bv[zscan_idx].mvy);
            HLMC_ECD_PutShortBits(bs, val, 2, "bv_y");
#endif
        }
    }
    else if (direct == 1)  // 1向左
    {
        val = bv[zscan_idx].mvx - bv[zscan_idx - left_delta_idx].mvx;
        bits += HLMC_ECD_PutSeBits(bs, val, !is_bitcount, "bv_x");
#if BVY_ZERO
        if (!bvy_zero_flag)
        {
            val = bv[zscan_idx].mvy - bv[zscan_idx - left_delta_idx].mvy;
            bits += HLMC_ECD_PutSeBits(bs, val, !is_bitcount, "bv_y");
        }
#elif !BVY_FIX_LENGTH
        val = bv[zscan_idx].mvy - bv[zscan_idx - left_delta_idx].mvy;
        bits += HLMC_ECD_PutSeBits(bs, val, !is_bitcount, "bv_y");
#endif
    }
    else  // 2向上
    {
        val = bv[zscan_idx].mvx - bv[zscan_idx - 2].mvx;
        bits += HLMC_ECD_PutSeBits(bs, val, !is_bitcount, "bv_x");
#if BVY_ZERO
        if (!bvy_zero_flag)
        {
            val = bv[zscan_idx].mvy;
            bits += HLMC_ECD_PutSeBits(bs, val, !is_bitcount, "bv_y");
        }
#elif !BVY_FIX_LENGTH
        val = bv[zscan_idx].mvy - bv[zscan_idx - 2].mvy;
        bits += HLMC_ECD_PutSeBits(bs, val, !is_bitcount, "bv_y");
#endif
    }

#if BVY_FIX_LENGTH
    // 仅y分量
    if (is_bitcount)
    {
        bits += 2;
    }
    else
    {
        val = HLM_ABS(bv[zscan_idx].mvy);
        HLMC_ECD_PutShortBits(bs, val, 2, "bv_y");
    }
#endif

    return bits;
}

#if MIX_IBC
/***************************************************************************************************
* 功  能：混合ibc划分类型的编码和比特估计
* 参  数：*
*        part_type           -I                   划分类型
*        bs                  -O                   码流结构体
*        is_bitcount         -I                   是否做比特估计
* 返回值：比特长度
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_ECD_EncodePartType(HLM_IBC_PART_TYPE  part_type,
                                HLMC_BITSTREAM    *bs,
                                HLM_U08            is_bitcount)
{
    HLM_U32 bits      = 0;
    HLM_S32 i         = 0;
    HLM_S32 max_value = HLM_IBC_PART_NUM - 1;

    bits = HLM_MIN(part_type + 1, max_value);

    if (!is_bitcount)
    {
        for (i = 0; i < part_type; i++)
        {
            HLMC_ECD_PutShortBits(bs, 1, 1, "part_type");
        }
        if (part_type < max_value)
        {
            HLMC_ECD_PutShortBits(bs, 0, 1, "part_type");
        }
    }

    return bits;
}
#endif

/***************************************************************************************************
* 功  能：对CU内语法元素进行熵编码
* 参  数：*
*        cur_cu                 -I        当前宏块
*        nbi_info               -I        相邻块信息结构体
*        frame_type             -I        帧类型
*        yuv_comp               -I        分量个数
*        bs                     -O        写入码流的结构体
*        segment_enable_flag    -I        隔断参考开关
*        ibc_enable_flag        -I        ibc使能开关
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_CU(HLMC_CU_INFO            *cur_cu,
                     HLM_NEIGHBOR_INFO       *nbi_info,
                     HLM_FRAME_TYPE           frame_type,
                     HLM_U32                  yuv_comp,
                     HLMC_BITSTREAM          *bs,
                     HLM_U08                  segment_enable_flag,
                     HLM_U32                  segment_width_in_log2,
                     HLM_U08                  ibc_enable_flag)
{
    HLM_U08 mpm       = 0;
    HLM_U08 zscan_idx = 0;
    HLM_MV  mvd       = { 0 };
    HLM_U08 block_num = 0;
    HLM_S16 delta_qp  = 0;
    HLM_U08 pu_x      = 0;
    HLM_U08 pu_y      = 0;
#if MIX_IBC
    HLM_IBC_PU_INFO *pu_info = HLM_NULL;
    HLM_U08 i                = 0;
    HLM_U08 bvx_bits         = 0;
    HLM_U08 direct           = 0;
    HLM_S32 bvx_offset       = 0;
    HLM_U08 bvy_zero_flag    = cur_cu->com_cu_info.bvy_zero_flag;
    HLM_U08 mix_ibc_flag     = cur_cu->com_cu_info.mix_ibc_flag;
#endif

    if (frame_type == HLM_FRAME_TYPE_P)
    {
        HLMC_ECD_PutShortBits(bs, (cur_cu->com_cu_info.cu_type == HLM_P_SKIP), 1, "cu_skip");
        if ((cur_cu->com_cu_info.cu_type == HLM_P_SKIP))
        {
            return;
        }
    }

    HLMC_ECD_EncodeCuType(cur_cu->com_cu_info.cu_type, frame_type, 1, bs, ibc_enable_flag);
    if (!ibc_enable_flag)
    {
        assert(cur_cu->com_cu_info.cu_type != HLM_IBC_4x4);
    }

    if (cur_cu->com_cu_info.cu_type == HLM_I_16x8)
    {
        HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, 0, &mpm, PROC_BS);
        HLMC_ECD_write_pred_mode_i16x8(cur_cu->pu_info_enc[0].inter_pu_info.intra_pred_mode, mpm, bs);
    }
    else if (cur_cu->com_cu_info.cu_type == HLM_I_4x4)
    {
        block_num = cur_cu->com_cu_info.intra_8x8_enable_flag ? 2 : 8;
        for (zscan_idx = 0; zscan_idx < block_num; zscan_idx++)
        {
            pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
            pu_y = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
            HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, zscan_idx, &mpm, PROC_BS);
            HLMC_ECD_write_pred_mode_i4x4(cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode, mpm, bs);
        }
    }
#if LINE_BY_LINE
    if (cur_cu->com_cu_info.cu_type == HLM_I_LINE || cur_cu->com_cu_info.cu_type == HLM_I_ROW)
    {
#if LINE_BY_LINE_4x1
        if (cur_cu->com_cu_info.cu_type == HLM_I_LINE)
        {
            for (zscan_idx = 0; zscan_idx < 4; zscan_idx++)
                HLMC_ECD_PutShortBits(bs, cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[zscan_idx], 2, "intra_pred_mode");
        }
        else
        {
            for (zscan_idx = 0; zscan_idx < 2; zscan_idx++)
                HLMC_ECD_PutShortBits(bs, cur_cu->pu_info_enc[0].inter_pu_info.row_by_row_mode[zscan_idx], 2, "intra_pred_mode");
        }
#else
#if LINE_BY_LINE_ONE_MODE
        HLM_U08 predmode = cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[0] == 0 ? 0 : 1;
        HLMC_ECD_PutShortBits(bs, predmode, 1, "intra_pred_mode");
#else
        HLMC_ECD_PutShortBits(bs, cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[0], 3, "intra_pred_mode");
#endif

#endif
    }
#endif
    else if (cur_cu->com_cu_info.cu_type == HLM_IBC_4x4)
    {
        HLMC_ECD_PutShortBits(bs, cur_cu->com_cu_info.ts_flag, 1, "ts_flag");
        HLMC_ECD_PutShortBits(bs, cur_cu->com_cu_info.merge_flag, (HLM_BV_MERGE_NUM > 2 ? 2 : 1), "merge_flag");
#if MIX_IBC
#if FIRST_COLUMN_IBC
        if (!cur_cu->com_cu_info.first_column_ibc_flag)
#else
        assert(cur_cu->com_cu_info.first_column_ibc_flag == 0);
        if (1)
#endif
        {
            HLMC_ECD_PutShortBits(bs, bvy_zero_flag, 2, "bvy_zero_flag");
            HLMC_ECD_PutShortBits(bs, mix_ibc_flag, 2, "mix_ibc_flag");
        }
        else
        {
            assert(bvy_zero_flag == 3);
            assert(mix_ibc_flag == 3);
        }
        bvx_bits = HLM_COM_GetBvxLen(&cur_cu->com_cu_info, segment_enable_flag, segment_width_in_log2);
        for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
        {
            pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
            direct = tbl_merge_type[cur_cu->com_cu_info.merge_flag][zscan_idx];
            bvx_offset = ((zscan_idx & 1) + 1) << 2;
            pu_info = &cur_cu->com_cu_info.ibc_pu_info[cur_cu->com_cu_info.merge_flag][zscan_idx];
#if FIRST_COLUMN_IBC
            if (cur_cu->com_cu_info.first_column_ibc_flag)
#else
            if (0)
#endif
            {
                assert(pu_info->part_type == HLM_IBC_HOR_SYM4);
            }
            else if (zscan_idx < 4 ? (mix_ibc_flag >> 1) : (mix_ibc_flag & 1))
            {
                HLMC_ECD_EncodePartType(pu_info->part_type, bs, 0);
            }
            else
            {
                assert(pu_info->part_type == HLM_IBC_NO_SPLIT);
            }
            for (i = 0; i < pu_info->sub_pu_num; i++)
            {
                if (direct == 0 && i == 0)  // 定长码
                {
#if FIRST_COLUMN_IBC
                    if (cur_cu->com_cu_info.first_column_ibc_flag)
                    {
                        HLMC_ECD_PutShortBits(bs, (pu_x << 2) + pu_info->sub_bv[i].mvx, bvx_bits, "bv_x");
                    }
                    else
#endif
                    {
                        HLMC_ECD_PutShortBits(bs, -pu_info->sub_bv[i].mvx - bvx_offset, bvx_bits, "bv_x");
                    }
                }
                else
                {
                    HLMC_ECD_PutSeBits(bs, pu_info->sub_bvd[i].mvx, 1, "bv_x");
                }
            }
            // y定长
            for (i = 0; i < pu_info->sub_pu_num; i++)
            {
                if (zscan_idx < 4 ? (bvy_zero_flag >> 1) : (bvy_zero_flag & 1))
                {
                    assert(pu_info->sub_bv[i].mvy == 0);
                }
                else
                {
                    HLMC_ECD_PutShortBits(bs, HLM_ABS(pu_info->sub_bv[i].mvy), 2, "bv_y");
                }
            }
        }
#else
#if BVY_ZERO
        HLMC_ECD_PutShortBits(bs, cur_cu->com_cu_info.bvy_zero_flag, BVY_ZERO == 1 ? 1 : 2, "bvy_zero_flag");
#endif
        for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
        {
            HLMC_ECD_EncodeBV(segment_enable_flag ? cur_cu->com_cu_info.cu_x % (1 << (segment_width_in_log2 - HLM_LOG2_WIDTH_SIZE)) : cur_cu->com_cu_info.cu_x,
#if BVY_ZERO == 1
                cur_cu->com_cu_info.bvy_zero_flag,
#elif BVY_ZERO == 2
                (zscan_idx < 4) ? (cur_cu->com_cu_info.bvy_zero_flag >> 1) : (cur_cu->com_cu_info.bvy_zero_flag & 1),
#elif BVY_ZERO == 3
                ((zscan_idx >> 1) & 1) == 0 ? (cur_cu->com_cu_info.bvy_zero_flag >> 1) : (cur_cu->com_cu_info.bvy_zero_flag & 1),
#endif
                cur_cu->com_cu_info.merge_flag, zscan_idx, cur_cu->bv_enc[cur_cu->com_cu_info.merge_flag], bs, 0);
        }
#endif
    }

    //mvd_l0
    if (frame_type == HLM_FRAME_TYPE_P)
    {
        if (cur_cu->com_cu_info.cu_type == HLM_P_16x8)
        {
            mvd.mvx = cur_cu->pu_info_enc[0].inter_pu_info.inter_mv.mvx - cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp.mvx;
            mvd.mvy = cur_cu->pu_info_enc[0].inter_pu_info.inter_mv.mvy - cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp.mvy;
            HLMC_ECD_PutSeBits(bs, mvd.mvx, 1, "mvd_l0_x");
            HLMC_ECD_PutSeBits(bs, mvd.mvy, 1, "mvd_l0_y");
        }
        else if (cur_cu->com_cu_info.cu_type == HLM_P_8x8)
        {
            mvd.mvx = cur_cu->pu_info_enc[0].inter_pu_info.inter_mv.mvx - cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp.mvx;
            mvd.mvy = cur_cu->pu_info_enc[0].inter_pu_info.inter_mv.mvy - cur_cu->pu_info_enc[0].inter_pu_info.inter_mvp.mvy;
            HLMC_ECD_PutSeBits(bs, mvd.mvx, 1, "mvd_l0_x");
            HLMC_ECD_PutSeBits(bs, mvd.mvy, 1, "mvd_l0_y");

            mvd.mvx = cur_cu->pu_info_enc[1].inter_pu_info.inter_mv.mvx - cur_cu->pu_info_enc[1].inter_pu_info.inter_mvp.mvx;
            mvd.mvy = cur_cu->pu_info_enc[1].inter_pu_info.inter_mv.mvy - cur_cu->pu_info_enc[1].inter_pu_info.inter_mvp.mvy;
            HLMC_ECD_PutSeBits(bs, mvd.mvx, 1, "mvd_l0_x");
            HLMC_ECD_PutSeBits(bs, mvd.mvy, 1, "mvd_l0_y");
        }
    }

    //cbf
    HLMC_ECD_write_cbf(cur_cu->com_cu_info.cu_type, cur_cu->com_cu_info.cbf, yuv_comp, bs);

    //qp、coeff
    if (cur_cu->com_cu_info.cbf[0] || cur_cu->com_cu_info.cbf[1] || cur_cu->com_cu_info.cbf[2])
    {
        if (cur_cu->com_cu_info.cu_delta_qp_enable_flag)
        {
            delta_qp = (HLM_S16)cur_cu->com_cu_info.qp[0] - (HLM_S16)cur_cu->com_cu_info.last_code_qp;
            HLMC_ECD_PutSeBits(bs, delta_qp, 1, "luma_qp_delta");
            cur_cu->com_cu_info.last_code_qp = cur_cu->com_cu_info.qp[0];
            if (yuv_comp > 1)
            {
                if (cur_cu->com_cu_info.cbf[1] || cur_cu->com_cu_info.cbf[2])
                {
                    delta_qp = (HLM_S16)cur_cu->com_cu_info.qp[1] - (HLM_S16)cur_cu->com_cu_info.qp[0];
                    HLMC_ECD_PutSeBits(bs, delta_qp, 1, "chroma_qp_delta");
                }
            }
        }

        HLMC_ECD_write_coeff_16x8(cur_cu, nbi_info, HLM_LUMA_Y, bs);
        if (yuv_comp > 1)
        {
            HLMC_ECD_write_coeff_16x8(cur_cu, nbi_info, HLM_CHROMA_U, bs);
            HLMC_ECD_write_coeff_16x8(cur_cu, nbi_info, HLM_CHROMA_V, bs);
        }
    }
}
#if LINE_BY_LINE_4x1_RESI
HLM_VOID HLMC_ECD_resi_pred(HLMC_CU_INFO * cur_cu,
                            HLM_COMPONENT plane)
{
    HLM_COEFF tmp_coeff[16 * 8] = { 0 };
    HLM_S08 j = 0;
    HLM_S08 i = 0;
    HLM_COEFF left_coeff = 0;
    HLM_COEFF up_coeff = 0;
    HLM_COEFF down_coeff = 0;
    HLM_COEFF right_coeff = 0;
    HLM_COEFF upleft_coeff = 0;
    HLM_S64   a = 0;
    HLM_S64   Max = 100000000;
    HLM_S64   c = 0;
    HLM_S64   record[8] = { 0 };
    HLM_U32   mode = 0;
    HLM_S32 width = 1 << cur_cu->com_cu_info.cu_width[plane];
    HLM_S32 height = 1 << cur_cu->com_cu_info.cu_height[plane];
    memcpy(tmp_coeff, cur_cu->com_cu_info.cu_pred_info.coeff[plane], 16 * 8 * sizeof(HLM_COEFF));
#if LINE_BY_LINE_4x1_RESI_OPT
    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
#else
    for (j = 1; j < height; j++)
    {
        for (i = 1; i < width; i++)
#endif
        {
#if LINE_BY_LINE_4x1_RESI_OPT
            if (i == 0 && j == 0)
            {
                continue;
            }
            else if (i == 0)
            {
                up_coeff = tmp_coeff[i + (j - 1) * 16];
                left_coeff = up_coeff;
                upleft_coeff = up_coeff;
            }
            else if (j == 0)
            {
                left_coeff = tmp_coeff[i + j * 16 - 1];
                upleft_coeff = left_coeff;
                up_coeff = left_coeff;
            }
            else
            {
                up_coeff = tmp_coeff[i + (j - 1) * 16];
                left_coeff = tmp_coeff[i + j * 16 - 1];
                upleft_coeff = (tmp_coeff[i + (j - 1) * 16] + tmp_coeff[i + j * 16 - 1]) >> 1;
            }
#else
            up_coeff = tmp_coeff[i + (j - 1) * 16];
            left_coeff = tmp_coeff[i + j * 16 - 1];
            upleft_coeff = (tmp_coeff[i + (j - 1) * 16] + tmp_coeff[i + j * 16 - 1]) >> 1;
#endif

            record[0] += HLM_ABS(tmp_coeff[i + j * 16] - (up_coeff));
            record[1] += HLM_ABS(tmp_coeff[i + j * 16] - (left_coeff));
            record[2] += HLM_ABS(tmp_coeff[i + j * 16] - (upleft_coeff));
            record[3] += HLM_ABS(tmp_coeff[i + j * 16]);
            /*
            //qipange
            if ((i + j) % 2 == 1)
            {
            up_coeff = (j == 0) ? tmp_coeff[i + (j + 1) * 16] : tmp_coeff[i + (j - 1) * 16];
            down_coeff = (j == height - 1) ? tmp_coeff[i + (j - 1) * 16] : tmp_coeff[i + (j + 1) * 16];
            left_coeff = (i == 0) ? tmp_coeff[i + j * 16 + 1] : tmp_coeff[i + j * 16 - 1];
            right_coeff = (i == width - 1) ? tmp_coeff[i + j * 16 - 1] : tmp_coeff[i + j * 16 + 1];
            // if (plane == 0)
            {
            record[0] += HLM_ABS(tmp_coeff[i + j * 16] - (up_coeff));
            record[1] += HLM_ABS(tmp_coeff[i + j * 16] - (down_coeff));
            record[2] += HLM_ABS(tmp_coeff[i + j * 16] - (left_coeff));
            record[3] += HLM_ABS(tmp_coeff[i + j * 16] - (right_coeff));
            record[4] += HLM_ABS(tmp_coeff[i + j * 16] - ((up_coeff + down_coeff) >> 1));
            record[5] += HLM_ABS(tmp_coeff[i + j * 16] - ((left_coeff + right_coeff) >> 1));
            record[6] += HLM_ABS(tmp_coeff[i + j * 16]);
            }

            }
            */
        }
    }

    mode = 0;
    for (i = 0; i < 4; i++)
    {
        if ((record[i]) < Max)
        {
            Max = (record[i]);
            mode = i;
        }
    }

    cur_cu->com_cu_info.cu_line_resi[plane] = mode;
    if (mode == 3)
        return;
    for (j = 1; j < height; j++)
    {
        for (i = 1; i < width; i++)
        {
            up_coeff = tmp_coeff[i + (j - 1) * 16];
            left_coeff = tmp_coeff[i + j * 16 - 1];
            upleft_coeff = (tmp_coeff[i + (j - 1) * 16] + tmp_coeff[i + j * 16 - 1]) >> 1;
            switch (mode)
            {
            case 0:
                cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - up_coeff;
                break;
            case 1:
                cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - left_coeff;
                break;
            case 2:
                cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - upleft_coeff;
                break;
            default:
                printf("wrong mode");
            }

        }
    }
    /*
    for (j = 0; j < height; j++)
    {
    for (i = 0; i < width; i++)
    {
    up_coeff = (j == 0) ? tmp_coeff[i + (j + 1) * 16] : tmp_coeff[i + (j - 1) * 16];
    down_coeff = (j == height - 1) ? tmp_coeff[i + (j - 1) * 16] : tmp_coeff[i + (j + 1) * 16];
    left_coeff = (i == 0) ? tmp_coeff[i + j * 16 + 1] : tmp_coeff[i + j * 16 - 1];
    right_coeff = (i == width - 1) ? tmp_coeff[i + j * 16 - 1] : tmp_coeff[i + j * 16 + 1];
    if ((i + j) % 2 == 1)
    {
    switch (mode)
    {
    case 0:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - up_coeff;
    break;
    case 1:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - down_coeff;
    break;
    case 2:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - left_coeff;
    break;
    case 3:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - right_coeff;
    break;
    case 4:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - ((up_coeff + down_coeff) >> 1);
    break;
    case 5:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - ((left_coeff + right_coeff) >> 1);
    break;

    }
    }
    }
    }


    if ((cur_cu->com_cu_info.cu_type == HLM_I_LINE) || (cur_cu->com_cu_info.cu_type == HLM_I_ROW))
    {
    if (plane == 0)
    {
    if (cur_cu->com_cu_info.cu_x == 0 && cur_cu->com_cu_info.cu_y == 0)
    {
    if (cur_cu->com_cu_info.cu_type == HLM_I_LINE)
    {
    cur_cu->com_cu_info.cu_line_resi = 5;
    cur_cu->com_cu_info.cu_line_resi_last = 5;
    }
    else
    {
    cur_cu->com_cu_info.cu_row_resi = 4;
    cur_cu->com_cu_info.cu_row_resi_last = 4;
    }
    }
    else
    {
    cur_cu->com_cu_info.cu_line_resi = cur_cu->com_cu_info.cu_line_resi_last;
    cur_cu->com_cu_info.cu_row_resi = cur_cu->com_cu_info.cu_row_resi_last;
    }
    }
    mode = cur_cu->com_cu_info.cu_type == HLM_I_LINE ? cur_cu->com_cu_info.cu_line_resi : cur_cu->com_cu_info.cu_row_resi;
    memcpy(tmp_coeff, cur_cu->com_cu_info.cu_pred_info.coeff[plane], 16 * 8 * sizeof(HLM_COEFF));
    for (j = 0; j < height; j++)
    {
    for (i = 0; i < width; i++)
    {
    //qipange
    if ((i + j) % 2 == 1)
    {
    up_coeff = (j == 0) ? tmp_coeff[i + (j + 1) * 16] : tmp_coeff[i + (j - 1) * 16];
    down_coeff = (j == height - 1) ? tmp_coeff[i + (j - 1) * 16] : tmp_coeff[i + (j + 1) * 16];
    left_coeff = (i == 0) ? tmp_coeff[i + j * 16 + 1] : tmp_coeff[i + j * 16 - 1];
    right_coeff = (i == width - 1) ? tmp_coeff[i + j * 16 - 1] : tmp_coeff[i + j * 16 + 1];
    if (plane == 0)
    {
    record[0] += HLM_ABS(tmp_coeff[i + j * 16] - (up_coeff));
    record[1] += HLM_ABS(tmp_coeff[i + j * 16] - (down_coeff));
    record[2] += HLM_ABS(tmp_coeff[i + j * 16] - (left_coeff));
    record[3] += HLM_ABS(tmp_coeff[i + j * 16] - (right_coeff));
    record[4] += HLM_ABS(tmp_coeff[i + j * 16] - ((up_coeff + down_coeff) >> 1));
    record[5] += HLM_ABS(tmp_coeff[i + j * 16] - ((left_coeff + right_coeff) >> 1));
    }
    switch (mode)
    {
    case 0:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - up_coeff;
    break;
    case 1:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - down_coeff;
    break;
    case 2:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - left_coeff;
    break;
    case 3:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - right_coeff;
    break;
    case 4:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - ((up_coeff + down_coeff) >> 1);
    break;
    case 5:
    cur_cu->com_cu_info.cu_pred_info.coeff[plane][i + j * 16] = tmp_coeff[i + j * 16] - ((left_coeff + right_coeff) >> 1);
    break;
    }

    }

    }
    }
    if (plane == 0)
    {
    for (i = 0; i < 6; i++)
    {
    if ((record[i])<Max)
    {
    Max = (record[i]);
    mode = i;
    }
    }
    if (cur_cu->com_cu_info.cu_type == HLM_I_LINE)
    cur_cu->com_cu_info.cu_line_resi = mode;
    else
    cur_cu->com_cu_info.cu_row_resi = mode;
    }
    }
    */
}
#endif


/***************************************************************************************************
* 功  能：对16x8块残差系数进行比特预估
* 参  数：*
*        cur_cu            -I                   当前宏块
*        nbi_info          -I                   相邻块信息
*        plane             -I                   yuv分量
* 返回值：比特长度
* 备  注：
***************************************************************************************************/
HLM_U32 HLMC_ECD_EstimateCoeff16x8(HLMC_CU_INFO           *cur_cu,
                                   HLM_NEIGHBOR_INFO      *nbi_info,
                                   HLM_COMPONENT           plane)
{
    HLM_U32 bits_len             = 0;
    HLM_U08 cbf                  = 0;
    HLM_U08 i                    = 0;
    HLM_U08 block_type           = 0;
    HLM_U08 b8                   = 0;
    HLM_U32 y_cbf                = cur_cu->com_cu_info.cbf[0];
    HLM_U32 cb_cbf               = cur_cu->com_cu_info.cbf[1];
    HLM_U32 cr_cbf               = cur_cu->com_cu_info.cbf[2];
    HLM_U16 max_coeff_num        = 16 * 8;
    HLM_COEFF *praster           = HLM_NULL;
    HLM_COEFF tmp_level          = 0;
    HLM_COEFF tmp_level_for_bits = 0;
    HLM_U32 width                = 1 << cur_cu->com_cu_info.cu_width[plane];
    HLM_U32 height               = 1 << cur_cu->com_cu_info.cu_height[plane];

    cbf = (plane == 0) ? y_cbf : (plane == 1) ? cb_cbf : cr_cbf;
    if (!cbf)
    {
        return bits_len;
    }
#if ENTROPY_FAST_OPT
    bits_len = 1;  // 初始保证有1比特，减少全0系数的时候带来的误差
    praster = cur_cu->com_cu_info.cu_pred_info.coeff[plane];
    for (i = 0; i < max_coeff_num; i++)
    {
        tmp_level = praster[i];
        tmp_level_for_bits = 1 - tmp_level * 2;
        if (tmp_level_for_bits < 0)
        {
            tmp_level_for_bits = tmp_level * 2;
        }
        if (tmp_level_for_bits <= 1)  // 如果等于1，即等效于原始的系数tmp_level[i]为0，这时候不计算比特长度
        {
            continue;
        }
        else if (tmp_level_for_bits >= 256)
        {
            bits_len += 16;
            tmp_level_for_bits >>= 8;
        }
        bits_len += HLMC_UE_SIZE_TAB[(HLM_U08)(tmp_level_for_bits)];
    }
#else
    if (!cur_cu->com_cu_info.ts_flag)
    {
        HLMC_ECD_write_dc(cur_cu, nbi_info, plane, NULL, &bits_len, 0);
    }
    HLMC_ECD_write_ac(cur_cu, nbi_info, plane, NULL, &bits_len, 0);
#endif

    return bits_len;
}
