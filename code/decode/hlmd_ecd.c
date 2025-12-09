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
#include "hlmd_ecd.h"

// 指数哥伦布码长度表
static const HLM_U08 HLMD_GOLOMB_VLC_LEN[512] =
{
    19,17,15,15,13,13,13,13,11,11,11,11,11,11,11,11, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,  //golomb_vlc_len
     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,  //golomb_vlc_len
     5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,  //golomb_vlc_len
     5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,  //golomb_vlc_len
     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //golomb_vlc_len
     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //golomb_vlc_len
     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //golomb_vlc_len
     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //golomb_vlc_len
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //golomb_vlc_len
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //golomb_vlc_len
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //golomb_vlc_len
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //golomb_vlc_len
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //golomb_vlc_len
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //golomb_vlc_len
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //golomb_vlc_len
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1   //golomb_vlc_len
};

// 无符号指数哥伦布码字表
static const HLM_U08 HLMD_UE_GOLOMB_VLC_CODE[512] =
{
    32,32,32,32,32,32,32,32,31,32,32,32,32,32,32,32,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,  //golomb_vlc_code_UE
     7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,  //golomb_vlc_code_UE
     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //golomb_vlc_code_UE
     5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,  //golomb_vlc_code_UE
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //golomb_vlc_code_UE
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //golomb_vlc_code_UE
     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  //golomb_vlc_code_UE
     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  //golomb_vlc_code_UE
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //golomb_vlc_code_UE
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //golomb_vlc_code_UE
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //golomb_vlc_code_UE
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //golomb_vlc_code_UE
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //golomb_vlc_code_UE
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //golomb_vlc_code_UE
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //golomb_vlc_code_UE
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   //golomb_vlc_code_UE
};

// 有符号指数哥伦布码字表
static const HLM_S08 HLMD_SE_GOLOMB_VLC_CODE[512] =
{
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     8, -8,  9, -9, 10,-10, 11,-11, 12,-12, 13,-13, 14,-14, 15,-15,  //golomb__vlc_code_SE
     4,  4,  4,  4, -4, -4, -4, -4,  5,  5,  5,  5, -5, -5, -5, -5,  //golomb__vlc_code_SE
     6,  6,  6,  6, -6, -6, -6, -6,  7,  7,  7,  7, -7, -7, -7, -7,  //golomb__vlc_code_SE
     2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  //golomb__vlc_code_SE
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  //golomb__vlc_code_SE
     3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  //golomb__vlc_code_SE
    -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,  //golomb__vlc_code_SE
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  //golomb__vlc_code_SE
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  //golomb__vlc_code_SE
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  //golomb__vlc_code_SE
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  //golomb__vlc_code_SE
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //golomb__vlc_code_SE
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //golomb__vlc_code_SE
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //golomb__vlc_code_SE
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //golomb__vlc_code_SE
};

/***************************************************************************************************
* 功  能：读取码流中的n个比特无符号值
* 参  数：*
*         bs       -IO   指向码流的指针
*         n        -I    所要读取的比特位数
*         
* 返回值：n个bit所表示的值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMD_ECD_ReadBits(HLM_VOID *bs_in,
                          HLM_U32 n)
{
    HLMD_BITSTREAM *bs = (HLMD_BITSTREAM *)bs_in;
    HLM_U32 bits_cnt   = bs->bits_cnt;
    HLM_U32 tmp        = *(HLM_U32 *)(bs->init_buf + (bits_cnt >> 3));
    HLM_U32 tmp2       = 0;

    HLMD_BSWAP(tmp);
    tmp <<= bits_cnt & 7;

    // tmp总共32位，无效位数为bits_cnt & 7 = 0~7，有效位数为32 - (bits_cnt & 7) = 25~32
    // 如果要读的比特数n超过有效位数，则再读"bits_cnt & 7"比特，凑足32个有效位
    if (n > 32 - (bits_cnt & 7))
    {
        tmp2 = *(HLM_U32 *)(bs->init_buf + (bits_cnt >> 3) + 4);
        HLMD_BSWAP(tmp2);
        tmp2 >>= (32 - (bits_cnt & 7));
        tmp += tmp2;
    }

    tmp >>= (32 - n);
    bs->bits_cnt += n;

    return tmp;
}

/***************************************************************************************************
* 功  能：解码哥伦布编码无符号数值
* 参  数：*
*         bs_in      -IO   码流指针
* 返回值：无符号数值
* 备  注：
***************************************************************************************************/
HLM_U32 HLMD_ECD_ReadUeGolomb(HLM_VOID *bs_in)
{
    HLMD_BITSTREAM *bs = (HLMD_BITSTREAM *)bs_in;
    HLM_S32 len        = 0;
    HLM_U32 bits_cnt   = bs->bits_cnt;
    HLM_U32 tmp        = *(HLM_U32 *)(bs->init_buf + (bits_cnt >> 3));
    HLM_S32 log        = 0;

    HLMD_BSWAP(tmp);

    tmp <<= (bits_cnt & 7);
    if (tmp & 0xf8000000)
    {
        //当码长小于等于9比特时，先将buf右移32-9=23比特，然后查表确定
        tmp >>= (32 - 9);
        if (bs->bits_cnt < bs->max_bits_num + 32)
        {
            bs->bits_cnt += HLMD_GOLOMB_VLC_LEN[tmp];
        }
        return HLMD_UE_GOLOMB_VLC_CODE[tmp];
    }
    else
    {
        /*当码字长度大于9比特且小于32比特时，前缀0的个数为prefix_zeros=31-log2(tmp),
        所以由指数哥伦布编码规则知道：码字长度len=2*prefix_zeros+1，所以需要右移的位数
        log=32-len=32 - (2*prefix_zeros+1) = 32 - (62 - 2* log2(tmp) + 1)
        = 2* log2(tmp)-31。码字长度也等于32-log。最后将tmp减一主要是由于k阶指数哥伦布
        编码会加上2^k，这里是0阶，所以在生成指数哥伦布码时加上了一个1，减去1才是真实的值。*/
        if ((HLM_U32)((31 - HLM_COM_Log2(tmp)) << 1) + 1 < (HLM_U32)(32 - (bits_cnt & 7)))
        {
            log = (HLM_COM_Log2(tmp) << 1) - 31;
            bs->bits_cnt += (32 - log);
            tmp >>= log;
            tmp--;
            return tmp;
        }
        else
        {
            tmp = HLMD_ECD_ReadBits(bs, 1);
            while (!tmp)
            {
                tmp = HLMD_ECD_ReadBits(bs, 1);
                len++;
            }
            tmp = HLMD_ECD_ReadBits(bs, len);
            tmp += 1 << len;
            tmp--;
            return tmp;
        }
    }
}

/***************************************************************************************************
* 功  能：解码哥伦布编码有符号数值
* 参  数：*
*        bs_in       -IO   指向码流的指针
* 返回值：有符号数值
* 备  注：
***************************************************************************************************/
HLM_S32 HLMD_ECD_ReadSeGolomb(HLM_VOID *bs_in)
{
    HLMD_BITSTREAM *bs = (HLMD_BITSTREAM *)bs_in;
    HLM_U32 bits_cnt   = bs->bits_cnt;
    HLM_U32 tmp        = *(HLM_U32 *)(bs->init_buf + (bits_cnt >> 3));
    HLM_S32 sign       = 0;
    HLM_S32 len        = 0;

    HLMD_BSWAP(tmp);

    tmp <<= (bits_cnt & 7);
    if (tmp & 0xf8000000)
    {
        //当码长小于等于9比特时，先将buf右移32-9=23比特，然后查表确定码字长度和对应的码字
        tmp >>= 32 - 9;
        if (bs->bits_cnt < bs->max_bits_num + 32)
        {
            bs->bits_cnt += HLMD_GOLOMB_VLC_LEN[tmp];
        }
        return HLMD_SE_GOLOMB_VLC_CODE[tmp];
    }
    else
    {
        //当码字长度大于9比特且小于32比特时，通过计算前缀0的个数来计算码字的value
        len  = 2 * HLM_COM_Log2(tmp) - 31;
        tmp >>= len;
        if (bs->bits_cnt < bs->max_bits_num + 32)
        {
            bs->bits_cnt += 32 - len;
        }
        sign = 0 - (tmp & 1);
        tmp  = ((tmp >> 1) ^ sign) - sign;
        return tmp;
    }
}

// 解析intra16x8预测模式
HLM_U32 HLMD_ECD_parse_pred_mode_i16x8(HLMD_BITSTREAM *bs,
                                       HLM_U08         mpm)
{
    HLM_U32 pred_mode = 0;
    HLM_U32 bin       = 0;

    bin = HLMD_ECD_ReadBits(bs, 1);
    if (bin == 1)
    {
        pred_mode = mpm;
    }
    else
    {
        bin = HLMD_ECD_ReadBits(bs, 1);
        if (bin == 0)
        {
            pred_mode = 0;
        }
        else
        {
            pred_mode = bin + HLMD_ECD_ReadBits(bs, 1);
        }
        if (pred_mode >= mpm)
        {
            pred_mode += 1;
        }
    }

    return pred_mode;
}

// 解析part_type
HLM_IBC_PART_TYPE HLMD_ECD_parse_part_type(HLMD_BITSTREAM  *bs)
{
    HLM_IBC_PART_TYPE part_type = 0;
    HLM_S32 last                = 0;
    HLM_S32 max_value           = HLM_IBC_PART_NUM - 1;

    for (part_type = 0; part_type < max_value; part_type++)
    {
        last = HLMD_ECD_ReadBits(bs, 1);
        if (last == 0)
        {
            break;
        }
    }

    return part_type;
}

// 解析cbf
HLM_VOID HLMD_ECD_parse_cbf(HLMD_BITSTREAM       *bs,
                            HLM_S32               cu_type,
                            HLM_U08               cbf[3],
                            HLM_U32               yuv_comp)
{
    HLM_U08 all_one_flag = 0;

    if (yuv_comp > 1)
    {
        all_one_flag = HLMD_ECD_ReadBits(bs, 1);
        if (all_one_flag)
        {
            cbf[0] = 1;
            cbf[1] = 1;
            cbf[2] = 1;
        }
        else
        {
            cbf[0] = HLMD_ECD_ReadBits(bs, 1);
            cbf[1] = HLMD_ECD_ReadBits(bs, 1);
            if ((cbf[0] && cbf[1]) == 0)
            {
                cbf[2] = HLMD_ECD_ReadBits(bs, 1);
            }
            else
            {
                cbf[2] = 0;
            }
        }
    }
    else
    {
        cbf[0] = HLMD_ECD_ReadBits(bs, 1);
        cbf[1] = 0;
        cbf[2] = 0;
    }
}

// 解析分组定长码
HLM_S32 HLMD_ECD_parse_gfl(HLM_VOID  *bs,
                           HLM_S16   *value,
                           HLM_U32    level,
                           HLM_U32    start,
                           HLM_U32    end,
                           HLM_U32    skip_flag)
{
    HLM_U32 step          = 1 << level;
    HLM_U32 ruiBits       = 0;
    HLM_U32 temp_0        = 0;
    HLM_U32 temp_1        = 0;
    HLM_S32 number_of_one = 0;
    HLM_U32 i             = 0;
    HLM_U32 j             = 0;
    HLM_U32 half          = 0;

    for (i = start; i < end; i += step)
    {
        temp_0 = 0;
        temp_1 = 0;
        step = (end + 1) > i + step + step ? step : end - i;
        if (!skip_flag && step > 3)
        {
            ruiBits = HLMD_ECD_ReadBits(bs, 1);
            if (ruiBits == 0)
            {
                for (j = 0; j < step; j++)
                {
                    value[i + j] = 0;
                }
            }
            else
            {
                half = step >> 1;
                ruiBits = HLMD_ECD_ReadBits(bs, 1);
                if (ruiBits == 1)
                {
                    ruiBits = HLMD_ECD_ReadBits(bs, 1);
                    if (ruiBits == 1)
                    {
                        for (j = 0; j < half; j++)
                        {
                            value[i + j] = 0;
                        }
                        for (j = half; j < step; j++)
                        {
                            ruiBits = HLMD_ECD_ReadBits(bs, 1);
                            value[i + j] = ruiBits;
                            number_of_one += ruiBits;
                        }
                    }
                    else
                    {
                        for (j = 0; j < half; j++)
                        {
                            ruiBits = HLMD_ECD_ReadBits(bs, 1);
                            value[i + j] = ruiBits;
                            number_of_one += ruiBits;
                        }
                        for (j = half; j < step; j++)
                        {
                            value[i + j] = 0;
                        }
                    }
                }
                else
                {
                    for (j = 0; j < step; j++)
                    {
                        ruiBits = HLMD_ECD_ReadBits(bs, 1);
                        value[i + j] = ruiBits;
                        number_of_one += ruiBits;
                    }
                }
            }
        }
        else
        {
            for (j = 0; j < step; j++)
            {
                ruiBits = HLMD_ECD_ReadBits(bs, 1);
                value[i + j] = ruiBits;
                number_of_one += ruiBits;
            }
        }
    }

    return number_of_one;
}

// 计算系数，反二值化
HLM_VOID HLMD_ECD_cal_coeff(HLM_COEFF *value,
                            HLM_S16   *gt0,
                            HLM_S16   *gt1,
                            HLM_S16   *sign,
                            HLM_COEFF *remain,
                            HLM_U32    coeff_num)
{
    HLM_U32 num_one  = 0;
    HLM_U32 num_sign = 0;
    HLM_U32 num_two  = 0;
    HLM_U32 i        = 0;

    for (i = 0; i < coeff_num; i++)
    {
        if (gt0[i] != 0)
        {
            if (gt1[num_one++] != 0)
            {
                value[i] = 2 + remain[num_two++];
            }
            else
            {
                value[i] = 1;
            }
            if (sign[num_sign++] != 0)
            {
                value[i] = -value[i];
            }
        }
    }
}

// 解析ac
HLM_VOID HLMD_ECD_parse_ac(HLMD_CU_INFO          *cur_cu,
                           HLM_NEIGHBOR_INFO     *nbi_info,
                           HLM_COMPONENT          plane,
                           HLMD_BITSTREAM        *bs)
{
    HLM_COEFF level   [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_COEFF remain  [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_S16 gt0       [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_S16 sign      [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_S16 gt1       [HLM_TU_4x4_NUMS << 4] = { 0 };
    HLM_U32 level_last                       = 0;
    HLM_U32 gt1_last                         = 0;
    HLM_U32 remians_last                     = 0;
    HLM_U32 group_len                        = 32;
    HLM_U32 num_one                          = 0;
    HLM_U32 num_sign                         = 0;
    HLM_U32 num_two                          = 0;
    HLM_U32 ruiBits                          = 0;
    HLM_U08 i                                = 0;
    HLM_U32 start_t                          = 0;
    HLM_U08 j                                = 0;
    HLM_U08 k                                = 0;
    HLM_U08 x                                = 0;
    HLM_U08 y                                = 0;
    HLM_U08 max_coeff_num                    = 16;
    HLM_COEFF *praster                       = HLM_NULL;
    HLM_U32 scan_flag                        = 0;
    HLM_U16 dc_number                        = 1 << (cur_cu->com_cu_info.cu_height[plane] + cur_cu->com_cu_info.cu_width[plane] - 4);
    HLM_U32 gt0_num                          = dc_number * max_coeff_num;
    HLM_U16 start_pos                        = 0;
    HLM_U16 end_pos                          = 0;
    HLM_U32 group                            = (gt0_num + 16) >> 5;
    HLM_U32 pu_width                         = 4;
    HLM_U32 pu_height                        = 4;
    HLM_U16 block_4x4                        = 0;

    if (num_one == 0)
    {
        scan_flag = HLMD_ECD_ReadBits(bs, 1);
        for (j = 0; j < gt0_num; j += group_len)
        {
            end_pos    = j + group_len > gt0_num ? gt0_num : j + group_len;
            ruiBits    = HLMD_ECD_ReadBits(bs, 1);
            level_last = (ruiBits == 1) ? 5 : 3;
            num_one   += HLMD_ECD_parse_gfl(bs, gt0, level_last, j, end_pos, 0);
            num_two   += HLMD_ECD_parse_gfl(bs, gt1, level_last, gt1_last, num_one, 0);
            num_sign  += HLMD_ECD_parse_gfl(bs, sign, level_last, gt1_last, num_one, 1);
            for (i = remians_last; i < num_two; i++)
            {
                ruiBits = HLMD_ECD_ReadUeGolomb(bs);
                remain[i] = ruiBits;
            }
            gt1_last = num_one;
            remians_last = num_two;
        }
        HLMD_ECD_cal_coeff(level, gt0, gt1, sign, remain, gt0_num);
    }

    praster = cur_cu->com_cu_info.cu_pred_info.coeff[plane];
    for (i = 0; i < (1 << cur_cu->com_cu_info.cu_height[plane]); i += pu_height)
    {
        for (j = 0; j < (1 << cur_cu->com_cu_info.cu_width[plane]); j += pu_width)
        {
            block_4x4 = ((i / pu_height) * (1 << (cur_cu->com_cu_info.cu_width[plane] - 2)) + j / pu_width);
            k = 0;
            for (y = 0; y < pu_height; y++)
            {
                for (x = 0; x < pu_width; x++)
                {
                    if (scan_flag == 1)
                    {
                        praster[(i + y) * HLM_WIDTH_SIZE + j + x] = level[block_4x4 * max_coeff_num + k];
                    }
                    else
                    {
                        praster[(i + y) * HLM_WIDTH_SIZE + j + x] = level[k * dc_number + block_4x4];
                    }
                    k++;
                }
            }
        }
    }
}

// 解析dc
HLM_VOID HLMD_ECD_parse_dc(HLMD_CU_INFO          *cur_cu,
                           HLM_NEIGHBOR_INFO     *nbi_info,
                           HLM_COMPONENT          plane,
                           HLMD_BITSTREAM        *bs)
{
    HLM_COEFF level [HLM_TU_4x4_NUMS] = { 0 };
    HLM_COEFF remain[HLM_TU_4x4_NUMS] = { 0 };
    HLM_S16 gt0     [HLM_TU_4x4_NUMS] = { 0 };
    HLM_S16 sign    [HLM_TU_4x4_NUMS] = { 0 };
    HLM_S16 gt1     [HLM_TU_4x4_NUMS] = { 0 };
    HLM_S32 num_one                   = 0;
    HLM_S32 num_sign                  = 0;
    HLM_S32 num_two                   = 0;
    HLM_U32 ruiBits                   = 0;
    HLM_S32 i                         = 0;
    HLM_S32 j                         = 0;
    HLM_U08 max_coeff_num             = 1 << (cur_cu->com_cu_info.cu_height[plane] + cur_cu->com_cu_info.cu_width[plane] - 4);
    HLM_COEFF *praster                = cur_cu->dc_coeffs[plane];

    num_one = HLMD_ECD_parse_gfl(bs, gt0, 3, 0, max_coeff_num, 0);
    num_two = HLMD_ECD_parse_gfl(bs, gt1, 2, 0, num_one, 1);
    num_sign = HLMD_ECD_parse_gfl(bs, sign, 2, 0, num_one, 1);
    for (i = 0; i < num_two; i++)
    {
        ruiBits = HLMD_ECD_ReadUeGolomb(bs);
        remain[i] = ruiBits;
    }
    HLMD_ECD_cal_coeff(praster, gt0, gt1, sign, remain, max_coeff_num);

    praster = cur_cu->com_cu_info.cu_pred_info.coeff[plane];
    for (i = 0; i < max_coeff_num; i++)
    {
        if (max_coeff_num == HLM_TU_4x4_NUMS)
        {
            level[HLM_INTRA_RASTER_TO_ZSCAN[i]] = cur_cu->dc_coeffs[plane][i];
        }
        else
        {
            level[i] = cur_cu->dc_coeffs[plane][i];
        }
    }
    for (i = 0; i < 1 << (cur_cu->com_cu_info.cu_height[plane]); i += 4)
    {
        for (j = 0; j < 1 << (cur_cu->com_cu_info.cu_width[plane]); j += 4)
        {
            praster[j + (i)*HLM_WIDTH_SIZE] = level[((i >> 2) *(1 << (cur_cu->com_cu_info.cu_width[plane] - 2))) + (j >> 2)];
            if (level[i])
            {
                num_one++;
            }
        }
    }

    cur_cu->dc_coeffs_num[plane] = num_one;
    memset(cur_cu->dc_coeffs[plane], 0, sizeof(HLM_COEFF) * HLM_TU_4x4_NUMS);
}

/***************************************************************************************************
* 功  能：对CU内语法元素进行熵解码
* 参  数：*
*         cur_cu         -IO        当前宏块信息
*         nbi_info       -I         邻域信息
*         patch_type     -I         帧类型
*         bs             -I         码流
*         regs           -I         寄存器
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_VOID HLMD_ECD_CU(HLMD_CU_INFO         *cur_cu,
                     HLM_NEIGHBOR_INFO    *nbi_info,
                     HLM_U32               patch_type,
                     HLMD_BITSTREAM       *bs,
                     HLMD_REGS            *regs)
{
    HLM_S32 luma_pred_mode    = 0;
    HLM_S32 luma_delta_qp     = 0;
    HLM_S32 chroma_delta_qp   = 0;
    HLM_U08 qp                = 0;
    HLM_S32 i                 = 0;
    HLM_S32 yuv_comp          = regs->image_format == HLM_IMG_YUV_400 ? 1 : 3;
    HLM_U08 mpm               = 0;
    HLM_U08 block_num         = cur_cu->com_cu_info.intra_8x8_enable_flag ? 2 : 8;
    HLM_S32 val               = 0;
    HLM_U08 zscan_idx         = 0;
    HLM_U08 pu_x              = 0;
    HLM_U08 pu_y              = 0;
    HLM_U08 skip_flag         = 0;
    HLM_U08 intra_flag        = 0;
    HLM_U08 ibc_flag          = 0;
    HLM_U08 partition_flag    = 0;
    HLM_U08 ibc_enable_flag   = (patch_type == HLM_FRAME_TYPE_I)
                              ? regs->patch_ctx->patch_header.sps.i_frame_enable_ibc
                              : regs->patch_ctx->patch_header.sps.p_frame_enable_ibc;
    HLM_IBC_PU_INFO *pu_info  = HLM_NULL;
    HLM_U08 bvx_bits          = 0;
    HLM_U08 direct            = 0;
    HLM_S32 bvx_offset        = 0;
    HLM_U08 bvy_zero_flag     = 0;
    HLM_U08 mix_ibc_flag      = 0;
    HLM_U08 pu_merge_flag     = 0;

    // 初始化
    cur_cu->com_cu_info.ts_flag = 0;
    for (i = 0; i < yuv_comp; i++)
    {
        memset(cur_cu->dc_coeffs[i], 0, sizeof(HLM_COEFF) * HLM_TU_4x4_NUMS);
        memset(cur_cu->com_cu_info.cu_pred_info.coeff[i], 0, sizeof(HLM_COEFF) * HLM_CU_SIZE);
    }

    // 解析cu_type
    if (patch_type == HLM_FRAME_TYPE_P)
    {
        skip_flag = HLMD_ECD_ReadBits(bs, 1);
        if (skip_flag)
        {
            cur_cu->com_cu_info.cbf[0] = 0;
            cur_cu->com_cu_info.cbf[1] = 0;
            cur_cu->com_cu_info.cbf[2] = 0;
            cur_cu->com_cu_info.cu_type = HLM_P_SKIP;
        }
        else
        {
            intra_flag = HLMD_ECD_ReadBits(bs, 1);
        }
    }
    else  // I帧
    {
        skip_flag = 0;
        intra_flag = 1;
    }
    if (!skip_flag)
    {
        if (intra_flag)
        {
            if (ibc_enable_flag)
            {
                ibc_flag = HLMD_ECD_ReadBits(bs, 1);
            }
            else
            {
                ibc_flag = 0;
            }
            if (ibc_flag)
            {
                cur_cu->com_cu_info.cu_type = HLM_IBC_4x4;
            }
            else
            {
                partition_flag = 0;
                if (cur_cu->com_cu_info.intra_16x1_2x8_enable_flag)
                {
                    partition_flag = HLMD_ECD_ReadBits(bs, 1) << 1;
                }
                partition_flag += HLMD_ECD_ReadBits(bs, 1);
                cur_cu->com_cu_info.cu_type = partition_flag;
            }
        }
        else
        {
            partition_flag = HLMD_ECD_ReadBits(bs, 1);
            cur_cu->com_cu_info.cu_type = (partition_flag == 0) ? HLM_P_16x8 : HLM_P_8x8;
        }
    }

    // 解析预测信息
    if (HLM_I_4x4 == cur_cu->com_cu_info.cu_type)
    {
        for (zscan_idx = 0; zscan_idx < block_num; zscan_idx++)
        {
            pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
            pu_y = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
            HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, zscan_idx, &mpm, PROC_BS);
            val = HLMD_ECD_ReadBits(bs, 1);
            if (val == 1)
            {
                luma_pred_mode = mpm;
            }
            else
            {
                val = HLMD_ECD_ReadBits(bs, 3);
                luma_pred_mode = val + (val >= mpm);
            }
#if INTRA_CHROMA_MODE_SEPARATE
            cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode[0] = luma_pred_mode;
            if ((cur_cu->com_cu_info.intra_sub_chroma_mode_enable_flag) && (yuv_comp > 1))
            {
                luma_pred_mode = HLMD_ECD_ReadBits(bs, 1);
                if (!luma_pred_mode)
                {
                    luma_pred_mode = cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode[0];
                }
                else
                {
                    luma_pred_mode = HLMD_ECD_ReadBits(bs, 1);
                    if (luma_pred_mode)
                    {
                        luma_pred_mode = HLMD_ECD_ReadBits(bs, 1) + 1;
                    }
                    else
                        luma_pred_mode = 0;
                }
            }
            cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode[1] = luma_pred_mode;
#else
            cur_cu->com_cu_info.cu_pred_info.pu_info[(pu_y << 2) + pu_x].intra_pred_mode = luma_pred_mode;
#endif
        }
    }
    else if (HLM_I_16x8 == cur_cu->com_cu_info.cu_type)
    {
        HLM_COM_GetMpm(nbi_info, &cur_cu->com_cu_info, 0, &mpm, PROC_BS);
        luma_pred_mode = HLMD_ECD_parse_pred_mode_i16x8(bs, mpm);
#if INTRA_CHROMA_MODE_SEPARATE
        cur_cu->com_cu_info.cu_pred_info.pu_info[0].intra_pred_mode[0] = luma_pred_mode;
#if INTRA_CHROMA_MODE_SEPARATE
        if (yuv_comp > 1 && cur_cu->com_cu_info.intra_chroma_mode_enable_flag)
            luma_pred_mode = HLMD_ECD_ReadBits(bs, 2);
        cur_cu->com_cu_info.cu_pred_info.pu_info[0].intra_pred_mode[1] = luma_pred_mode;
#endif
#else
        cur_cu->com_cu_info.cu_pred_info.pu_info[0].intra_pred_mode = luma_pred_mode;
#endif
    }
    else if (cur_cu->com_cu_info.cu_type == HLM_I_LINE || cur_cu->com_cu_info.cu_type == HLM_I_ROW)
    {
        cur_cu->com_cu_info.ts_flag = 1;
        pu_merge_flag = HLMD_ECD_ReadBits(bs, 1);
        if (pu_merge_flag ==1)
        {
            cur_cu->com_cu_info.line_row_pred_mode_cu = HLMD_ECD_ReadBits(bs, 1);
            if (cur_cu->com_cu_info.line_row_pred_mode_cu != 0)
            {
                cur_cu->com_cu_info.line_row_pred_mode_cu = HLMD_ECD_ReadBits(bs, 1) + 1;
            }
            for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
            {
                cur_cu->com_cu_info.line_row_pred_mode[zscan_idx] = cur_cu->com_cu_info.line_row_pred_mode_cu;
            }
        }
        else
        {
            cur_cu->com_cu_info.line_row_pred_mode_cu = 3;
            for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
            {
                luma_pred_mode = HLMD_ECD_ReadBits(bs, 1);
                if (luma_pred_mode != 0)
                {
                    luma_pred_mode = 1 + HLMD_ECD_ReadBits(bs, 1);
                }
                cur_cu->com_cu_info.line_row_pred_mode[zscan_idx] = luma_pred_mode;
            }
        }
    }
    else if (HLM_IBC_4x4 == cur_cu->com_cu_info.cu_type)
    {
        cur_cu->com_cu_info.ts_flag = HLMD_ECD_ReadBits(bs, 1);
        cur_cu->com_cu_info.merge_flag = HLMD_ECD_ReadBits(bs, (HLM_BV_MERGE_NUM > 2 ? 2 : 1));
        val = HLMD_ECD_ReadBits(bs, 1);
        bvy_zero_flag = (val << 1) + HLMD_ECD_ReadBits(bs, 1);
        if (regs->sub_ibc_enable_flag)
        {
            val = HLMD_ECD_ReadBits(bs, 1);
            mix_ibc_flag = (val << 1) + HLMD_ECD_ReadBits(bs, 1);
        }
        else
        {
            mix_ibc_flag = 0;
        }
        cur_cu->com_cu_info.bvy_zero_flag = bvy_zero_flag;
        cur_cu->com_cu_info.mix_ibc_flag = mix_ibc_flag;
        bvx_bits = HLM_COM_GetBvxLen(&cur_cu->com_cu_info, regs->segment_enable_flag, regs->segment_width_in_log2);
        for (zscan_idx = 0; zscan_idx < 8; zscan_idx++)
        {
            pu_x = HLM_INTRA_ZSCAN_TO_PELX[zscan_idx];
            pu_y = HLM_INTRA_ZSCAN_TO_PELY[zscan_idx];
            direct = tbl_merge_type[cur_cu->com_cu_info.merge_flag][zscan_idx];
            bvx_offset = ((zscan_idx & 1) + 1) << 2;
            pu_info = &cur_cu->com_cu_info.ibc_pu_info[cur_cu->com_cu_info.merge_flag][zscan_idx];
            if (zscan_idx < 4 ? (mix_ibc_flag >> 1) : (mix_ibc_flag & 1))
            {
                pu_info->part_type = HLMD_ECD_parse_part_type(bs);
            }
            else
            {
                pu_info->part_type = HLM_IBC_NO_SPLIT;
            }
            HLM_COM_GetIbcPuInfo(pu_info->part_type, pu_info);
            for (i = 0; i < pu_info->sub_pu_num; i++)
            {
                if (direct == 0 && i == 0)  // 定长码
                {
                    val = HLMD_ECD_ReadBits(bs, bvx_bits);
                    pu_info->sub_bv[i].mvx = -val - bvx_offset;
                }
                else
                {
                    pu_info->sub_bvd[i].mvx = HLMD_ECD_ReadSeGolomb(bs);
                }
            }
            // y定长
            for (i = 0; i < pu_info->sub_pu_num; i++)
            {
                if (zscan_idx < 4 ? (bvy_zero_flag >> 1) : (bvy_zero_flag & 1))
                {
                    pu_info->sub_bv[i].mvy = 0;
                }
                else
                {
                    val = HLMD_ECD_ReadBits(bs, 2);
                    pu_info->sub_bv[i].mvy = pu_y == 0 ? val : -val;
                }
            }
        }
    }
    else if (HLM_P_8x8 == cur_cu->com_cu_info.cu_type)
    {
        cur_cu->mvd_l0[0].mvx = HLMD_ECD_ReadSeGolomb(bs);
        cur_cu->mvd_l0[0].mvy = HLMD_ECD_ReadSeGolomb(bs);
        cur_cu->mvd_l0[1].mvx = HLMD_ECD_ReadSeGolomb(bs);
        cur_cu->mvd_l0[1].mvy = HLMD_ECD_ReadSeGolomb(bs);
    }
    else if (HLM_P_16x8 == cur_cu->com_cu_info.cu_type)
    {
        cur_cu->mvd_l0[0].mvx = HLMD_ECD_ReadSeGolomb(bs);
        cur_cu->mvd_l0[0].mvy = HLMD_ECD_ReadSeGolomb(bs);
    }
    else
    {
        assert(HLM_P_SKIP == cur_cu->com_cu_info.cu_type);
    }

    // 解析cbf
    if (HLM_P_SKIP != cur_cu->com_cu_info.cu_type)
    {
        HLMD_ECD_parse_cbf(bs, cur_cu->com_cu_info.cu_type, cur_cu->com_cu_info.cbf, yuv_comp);
    }

    // 解析qp和系数
    if (cur_cu->com_cu_info.cbf[0] || cur_cu->com_cu_info.cbf[1] || cur_cu->com_cu_info.cbf[2])
    {
        if (cur_cu->com_cu_info.cu_delta_qp_enable_flag)
        {
            luma_delta_qp = HLMD_ECD_ReadSeGolomb(bs);
            cur_cu->com_cu_info.qp[0] = cur_cu->com_cu_info.qp[0] + luma_delta_qp;
            if (yuv_comp > 1)
            {
                if (cur_cu->com_cu_info.cbf[1] || cur_cu->com_cu_info.cbf[2])
                {
                    chroma_delta_qp = HLMD_ECD_ReadSeGolomb(bs);
                    cur_cu->com_cu_info.qp[1] = cur_cu->com_cu_info.qp[0] + chroma_delta_qp;
                    cur_cu->com_cu_info.qp[2] = cur_cu->com_cu_info.qp[1];
                }
                else
                {
                    cur_cu->com_cu_info.qp[1] = cur_cu->com_cu_info.qp[0];
                    cur_cu->com_cu_info.qp[2] = cur_cu->com_cu_info.qp[1];
                }
            }
        }
        else
        {
            cur_cu->com_cu_info.qp[0] = regs->patch_ctx->patch_header.pps.pic_luma_qp;
            cur_cu->com_cu_info.qp[1] = regs->patch_ctx->patch_header.pps.pic_luma_qp
                                      + regs->patch_ctx->patch_header.pps.pic_chroma_delta_qp;
            cur_cu->com_cu_info.qp[2] = cur_cu->com_cu_info.qp[1];
        }
        cur_cu->com_cu_info.qp[0] = HLM_CLIP(cur_cu->com_cu_info.qp[0], 0, (HLM_U08)HLM_MAX_QP(regs->dec_pic_luma_bitdepth));
        cur_cu->com_cu_info.qp[1] = HLM_CLIP(cur_cu->com_cu_info.qp[1], 0, (HLM_U08)HLM_MAX_QP(regs->dec_pic_chroma_bitdepth));
        cur_cu->com_cu_info.qp[2] = HLM_CLIP(cur_cu->com_cu_info.qp[2], 0, (HLM_U08)HLM_MAX_QP(regs->dec_pic_chroma_bitdepth));

        for (i = 0; i < yuv_comp; i++)
        {
            if (cur_cu->com_cu_info.cbf[i] != 0)
            {
                HLMD_ECD_parse_ac(cur_cu, nbi_info, i, bs);
            }
        }
    }
}
