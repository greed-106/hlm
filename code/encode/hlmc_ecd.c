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
#if FIX_2
    HLM_U32 val       = 0;
#endif

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
#if FIX_2
             // len_bits为剩余的有效位数，len_tmp为当前待写入的位数
             val = code << (32 - len_bits);  // 把无效的高位顶掉
             val = val  >> (32 - len_tmp);   // 把待写入的移到低位
             HLMC_ECD_PutShortBits(bs, val, len_tmp, HLM_NULL);
#else
             HLMC_ECD_PutShortBits(bs, code >> len_bytes, len_tmp, HLM_NULL);
#endif
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
                            HLMC_ECD_PutLongBits(bs, value[i + j] == 1, 1, "group_value");
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
                            HLMC_ECD_PutLongBits(bs, value[i + j] == 1, 1, "group_value");
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
                        HLMC_ECD_PutLongBits(bs, value[i + j] == 1, 1, "group_value");
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
                    HLMC_ECD_PutLongBits(bs, value[i + j] == 1, 1, "group_value");
                }
            }

        }
    }
    return cost;
}

// 系数二值化，获取gt0、gt1、符号、剩余值
HLM_VOID HLMC_ECD_coeff_binary(HLM_S16         *gt0,
                               HLM_S16         *gt1,
                               HLM_S16         *sign,
                               HLM_COEFF       *remain,
                               HLMC_BITSTREAM  *bs,
                               HLM_U08          write_flag,
                               HLM_U32         *cost_total,
                               HLM_COEFF       *tmp_value,
                               HLM_U32         *num_zero,
                               HLM_U32         *num_sign,
                               HLM_U32         *num_one,
                               HLM_U32         *num_two,
                               HLM_U32          num_value)
{
    HLM_U08 i          = 0;
    HLM_U08 j          = 0;
    HLM_U08 k          = 0;
    HLM_S08 number_1   = 1;
    HLM_S08 gt1_temp   = 1;
    HLM_U08 end        = 0;
    HLM_U32 cost_bit_3 = 0;
    HLM_U32 cost_bit_5 = 0;
    HLM_U08 best_level = 3;
    HLM_U32 cost       = 0;
    HLM_U32 group_len  = 32;

    for (i = 0; i < num_value; i += group_len)
    {
        end         = (i + group_len) > num_value ? num_value : i + group_len;
        cost        = 0;
        num_zero[0] = 0;
        num_sign[0] = 0;
        num_one[0]  = 0;
        num_two[0]  = 0;
        best_level  = 3;
        group_len   = end - i;
        for (j = i; j < end; j++)
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

        cost_bit_3 = HLMC_ECD_write_gfl(bs, gt0, 3, 0, group_len, 0, 0);
        cost_bit_5 = HLMC_ECD_write_gfl(bs, gt0, 5, 0, group_len, 0, 0);
        if (cost_bit_5 <= cost_bit_3)
        {
            best_level = 5;
        }
        cost += 1;
        if (write_flag)
        {
            HLMC_ECD_PutShortBits(bs, (cost_bit_5 <= cost_bit_3), 1, "gt0_level");
        }
        cost += HLMC_ECD_write_gfl(bs, gt0, best_level, 0, group_len, write_flag, 0);
        cost += HLMC_ECD_write_gfl(bs, gt1, best_level, 0, num_one[0], write_flag, 0);
        cost += HLMC_ECD_write_gfl(bs, sign, best_level, 0, num_one[0], write_flag, 1);
        for (k = 0; k < num_two[0]; k++)
        {
            cost += HLMC_ECD_PutUeBits(bs, remain[k], write_flag, "remain");
        }
        cost_total[0] += cost;
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
    HLM_U08 max_coeff_num                       = 16;
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
    total_bit[0] += 1;
    num_zero = num_sign = num_one = num_two = 0;
    HLMC_ECD_coeff_binary(gt0, gt1, sign, remain,
        bs, 0, &cost_tmp, tmp_value, &num_zero, &num_sign, &num_one, &num_two, max_coeff_num * DC_number);
    num_zero = num_sign = num_one = num_two = 0;
    HLMC_ECD_coeff_binary(gt0, gt1, sign, remain,
        bs, 0, &cost_tmp_1, tmp_value_1, &num_zero, &num_sign, &num_one, &num_two, max_coeff_num * DC_number);
    if (write_flag)
    {
        HLMC_ECD_PutLongBits(bs, (cost_tmp <= cost_tmp_1), 1, "scan_flag");
    }
    tmp_p = cost_tmp <= cost_tmp_1 ? &tmp_value[0] : &tmp_value_1[0];
    num_zero = num_sign = num_one = num_two = 0;
    HLMC_ECD_coeff_binary(gt0, gt1, sign, remain,
        bs, write_flag, total_bit, tmp_p, &num_zero, &num_sign, &num_one, &num_two, max_coeff_num * DC_number);
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
                              HLM_U32              intra_16x1_2x8_enable_flag,
                              HLM_U32              ibc_enable_flag)
{
    HLM_U32 bits_len        = 0;
    HLM_U08 intra_flag      = 0;
    HLM_U08 ibc_flag        = 0;
    HLM_U08 partition_flag  = 0;
    HLM_U08 intra_pred_mode = cu_type;

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
                if (intra_16x1_2x8_enable_flag)
                {
                    HLMC_ECD_PutShortBits(bs, intra_pred_mode > 1, 1, "partition_flag");
                }
                HLMC_ECD_PutShortBits(bs, intra_pred_mode%2, 1, "partition_flag");
            }
            bits_len += 1+ intra_16x1_2x8_enable_flag;
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
                     HLM_U32                  intra_16x1_2x8_enable_flag,
                     HLM_U08                  ibc_enable_flag,
                     HLM_U08                  sub_ibc_enable_flag)
{
    HLM_U08 mpm              = 0;
    HLM_U08 zscan_idx        = 0;
    HLM_MV  mvd              = { 0 };
    HLM_U08 block_num        = 0;
    HLM_S16 delta_qp         = 0;
    HLM_U08 pu_x             = 0;
    HLM_U08 pu_y             = 0;
    HLM_IBC_PU_INFO *pu_info = HLM_NULL;
    HLM_U08 i                = 0;
    HLM_U08 bvx_bits         = 0;
    HLM_U08 direct           = 0;
    HLM_S32 bvx_offset       = 0;
    HLM_U08 bvy_zero_flag    = cur_cu->com_cu_info.bvy_zero_flag;
    HLM_U08 mix_ibc_flag     = cur_cu->com_cu_info.mix_ibc_flag;
    HLM_U08 pu_mode          = 0;
    HLM_U08 *pu_mode_index   = NULL;
    HLM_U32 total_bit        = 0;

    if (frame_type == HLM_FRAME_TYPE_P)
    {
        HLMC_ECD_PutShortBits(bs, (cur_cu->com_cu_info.cu_type == HLM_P_SKIP), 1, "cu_skip");
        if ((cur_cu->com_cu_info.cu_type == HLM_P_SKIP))
        {
            return;
        }
    }

    HLMC_ECD_EncodeCuType(cur_cu->com_cu_info.cu_type, frame_type, 1, bs, intra_16x1_2x8_enable_flag, ibc_enable_flag);
    if (!ibc_enable_flag)
    {
        assert(cur_cu->com_cu_info.cu_type != HLM_IBC_4x4);
    }

    if (cur_cu->com_cu_info.cu_type == HLM_I_16x8)
    {
        HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, 0, &mpm, PROC_BS);
#if INTRA_CHROMA_MODE_SEPARATE
        HLMC_ECD_write_pred_mode_i16x8(cur_cu->pu_info_enc[0].inter_pu_info.intra_pred_mode[0], mpm, bs);

#if INTRA_CHROMA_MODE_SEPARATE
        if (yuv_comp > 1 && cur_cu->com_cu_info.intra_chroma_mode_enable_flag)
            HLMC_ECD_PutShortBits(bs, cur_cu->pu_info_enc[0].inter_pu_info.intra_pred_mode[1], 2, "intra_pred_mode");
#endif
#else
        HLMC_ECD_write_pred_mode_i16x8(cur_cu->pu_info_enc[0].inter_pu_info.intra_pred_mode, mpm, bs);
#endif
    }
    else if (cur_cu->com_cu_info.cu_type == HLM_I_4x4)
    {
        block_num = cur_cu->com_cu_info.intra_8x8_enable_flag ? 2 : 8;
        for (zscan_idx = 0; zscan_idx < block_num; zscan_idx++)
        {
            pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
            pu_y = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
            HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, zscan_idx, &mpm, PROC_BS);
#if INTRA_CHROMA_MODE_SEPARATE
            HLMC_ECD_write_pred_mode_i4x4(cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode[0], mpm, bs);
            if ((cur_cu->com_cu_info.intra_sub_chroma_mode_enable_flag) && (yuv_comp > 1))
            {
                pu_mode = cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode[1];
                HLMC_ECD_PutShortBits(bs, pu_mode != cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode[0], 1, "intra_pred_mode_dm");
                if (pu_mode != cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode[0])
                {
                    HLMC_ECD_PutShortBits(bs, pu_mode != 0, 1, "intra_pred_mode_0");
                    if (pu_mode != 0)
                    {
                        HLMC_ECD_PutShortBits(bs, (pu_mode - 1) != 0, 1, "intra_pred_mode_1");
                    }
                }
            }
#else
            HLMC_ECD_write_pred_mode_i4x4(cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode, mpm, bs);
#endif
        }
    }
    if (cur_cu->com_cu_info.cu_type == HLM_I_LINE || cur_cu->com_cu_info.cu_type == HLM_I_ROW)
    {
        if (cur_cu->com_cu_info.cu_type == HLM_I_LINE)
        {
            pu_mode = cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode_cu;
            pu_mode_index = &cur_cu->pu_info_enc[0].inter_pu_info.line_by_line_mode[0];
        }
        else
        {
            pu_mode = cur_cu->pu_info_enc[0].inter_pu_info.row_by_row_mode_cu;
            pu_mode_index = &cur_cu->pu_info_enc[0].inter_pu_info.row_by_row_mode[0];
        }
        HLMC_ECD_PutShortBits(bs, pu_mode < 3, 1, "PU_merge_flag");
        if (pu_mode < 3)
        {
            HLMC_ECD_PutShortBits(bs, pu_mode != 0, 1, "pu0_mode");
            if (pu_mode != 0)
                HLMC_ECD_PutShortBits(bs, pu_mode - 1, 1, "pu0_mode");
        }
        if (pu_mode == 3)
        {
            for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
            {
                HLMC_ECD_PutShortBits(bs, pu_mode_index[zscan_idx] != 0, 1, "i_th_line_equal_mode0");
                if (pu_mode_index[zscan_idx] >= 1)
                    HLMC_ECD_PutShortBits(bs, pu_mode_index[zscan_idx] - 1, 1, "i_th_line_not_equal_mode0");
            }
        }
    }
    else if (cur_cu->com_cu_info.cu_type == HLM_IBC_4x4)
    {
        HLMC_ECD_PutShortBits(bs, cur_cu->com_cu_info.ts_flag, 1, "ts_flag");
        HLMC_ECD_PutShortBits(bs, cur_cu->com_cu_info.merge_flag, (HLM_BV_MERGE_NUM > 2 ? 2 : 1), "merge_flag");
        HLMC_ECD_PutShortBits(bs, bvy_zero_flag >> 1, 1, "bvy_zero_flag[0]");
        HLMC_ECD_PutShortBits(bs, bvy_zero_flag & 1,  1, "bvy_zero_flag[1]");
        if (sub_ibc_enable_flag)
        {
            HLMC_ECD_PutShortBits(bs, mix_ibc_flag >> 1, 1, "sub_ibc_flag[0]");
            HLMC_ECD_PutShortBits(bs, mix_ibc_flag & 1,  1, "sub_ibc_flag[1]");
        }
        else
        {
            assert(mix_ibc_flag == 0);
        }
        bvx_bits = HLM_COM_GetBvxLen(&cur_cu->com_cu_info, segment_enable_flag, segment_width_in_log2);
        for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
        {
            pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
            direct = tbl_merge_type[cur_cu->com_cu_info.merge_flag][zscan_idx];
            bvx_offset = ((zscan_idx & 1) + 1) << 2;
            pu_info = &cur_cu->com_cu_info.ibc_pu_info[cur_cu->com_cu_info.merge_flag][zscan_idx];
            if (zscan_idx < 4 ? (mix_ibc_flag >> 1) : (mix_ibc_flag & 1))
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
                    HLMC_ECD_PutShortBits(bs, -pu_info->sub_bv[i].mvx - bvx_offset, bvx_bits, "bv_x");
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

        for (i = 0; i < yuv_comp; i++)
        {
            if (cur_cu->com_cu_info.cbf[i])
            {
                HLMC_ECD_write_ac(cur_cu, nbi_info, i, bs, &total_bit, 1);
            }
        }
    }
}

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
    HLM_U32 bits_len                          = 0;
    HLM_U08 i                                 = 0;
    HLM_U08 j                                 = 0;
    HLM_U08 block_type                        = cur_cu->com_cu_info.cu_type;
    HLM_U08 b8                                = 0;
    HLM_U32 cbf                               = cur_cu->com_cu_info.cbf[plane];
    HLM_COEFF *praster                        = HLM_NULL;
    HLM_COEFF tmp_level                       = 0;
    HLM_COEFF tmp_level_for_bits              = 0;
    HLM_U32 width                             = 1 << cur_cu->com_cu_info.cu_width[plane];
    HLM_U32 height                            = 1 << cur_cu->com_cu_info.cu_height[plane];
    HLM_U16 max_coeff_num                     = width * height;
    HLM_COEFF tmp_value[HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_U16 format                            = HLM_LOG2_WIDTH_SIZE + HLM_LOG2_HEIGHT_SIZE
                                              - cur_cu->com_cu_info.cu_width[plane] - cur_cu->com_cu_info.cu_height[plane];

    if (!cbf)
    {
        return bits_len;
    }
    HLMC_ECD_write_ac(cur_cu, nbi_info, plane, NULL, &bits_len, 0);

    return bits_len;
}
