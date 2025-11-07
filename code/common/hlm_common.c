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
#include "hlm_common.h"

// 对数表
static const HLM_U08 HLM_UTILS_LOG2_TAB[256] =
{
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

/***************************************************************************************************
* 功  能：计算对数
* 参  数：
*        v                        -I       待求对数的值
* 返回值：计算对数后向下取整的结果
* 备  注：
***************************************************************************************************/
HLM_S32 HLM_COM_Log2(HLM_U32 v)
{
    HLM_S32 n = 0;

    //如果v的高16位有值，则其log2对数一定大于等于16，将低16位移除后继续判断
    if (v & 0xffff0000)
    {
        v >>= 16;
        n += 16;
    }

    //同理，如果操作后的v的高8位有值，则操作后的log2对数一定大于等于8，将低8位移除后继续判断
    if (v & 0xff00)
    {
        v >>= 8;
        n += 8;
    }

    //最后的8比特中，其对数通过查表直接获得，n表示v的取值在2^n~2^(n+1)之间
    n += HLM_UTILS_LOG2_TAB[v];

    return n;
}

// MD5转换
HLM_VOID HLM_COM_md5_trans(HLM_U32 *buf,
                           HLM_U32 *msg)
{
    register HLM_U32 a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5FUNC(FF, a, b, c, d, msg[0],   7, 0xd76aa478); // 1
    MD5FUNC(FF, d, a, b, c, msg[1],  12, 0xe8c7b756); // 2
    MD5FUNC(FF, c, d, a, b, msg[2],  17, 0x242070db); // 3
    MD5FUNC(FF, b, c, d, a, msg[3],  22, 0xc1bdceee); // 4
    MD5FUNC(FF, a, b, c, d, msg[4],   7, 0xf57c0faf); // 5
    MD5FUNC(FF, d, a, b, c, msg[5],  12, 0x4787c62a); // 6
    MD5FUNC(FF, c, d, a, b, msg[6],  17, 0xa8304613); // 7
    MD5FUNC(FF, b, c, d, a, msg[7],  22, 0xfd469501); // 8
    MD5FUNC(FF, a, b, c, d, msg[8],   7, 0x698098d8); // 9
    MD5FUNC(FF, d, a, b, c, msg[9],  12, 0x8b44f7af); // 10
    MD5FUNC(FF, c, d, a, b, msg[10], 17, 0xffff5bb1); // 11
    MD5FUNC(FF, b, c, d, a, msg[11], 22, 0x895cd7be); // 12
    MD5FUNC(FF, a, b, c, d, msg[12],  7, 0x6b901122); // 13
    MD5FUNC(FF, d, a, b, c, msg[13], 12, 0xfd987193); // 14
    MD5FUNC(FF, c, d, a, b, msg[14], 17, 0xa679438e); // 15
    MD5FUNC(FF, b, c, d, a, msg[15], 22, 0x49b40821); // 16

    // Round 2
    MD5FUNC(GG, a, b, c, d, msg[1],   5, 0xf61e2562); // 17
    MD5FUNC(GG, d, a, b, c, msg[6],   9, 0xc040b340); // 18
    MD5FUNC(GG, c, d, a, b, msg[11], 14, 0x265e5a51); // 19
    MD5FUNC(GG, b, c, d, a, msg[0],  20, 0xe9b6c7aa); // 20
    MD5FUNC(GG, a, b, c, d, msg[5],   5, 0xd62f105d); // 21
    MD5FUNC(GG, d, a, b, c, msg[10],  9, 0x2441453 ); // 22
    MD5FUNC(GG, c, d, a, b, msg[15], 14, 0xd8a1e681); // 23
    MD5FUNC(GG, b, c, d, a, msg[4],  20, 0xe7d3fbc8); // 24
    MD5FUNC(GG, a, b, c, d, msg[9],   5, 0x21e1cde6); // 25
    MD5FUNC(GG, d, a, b, c, msg[14],  9, 0xc33707d6); // 26
    MD5FUNC(GG, c, d, a, b, msg[3],  14, 0xf4d50d87); // 27
    MD5FUNC(GG, b, c, d, a, msg[8],  20, 0x455a14ed); // 28
    MD5FUNC(GG, a, b, c, d, msg[13],  5, 0xa9e3e905); // 29
    MD5FUNC(GG, d, a, b, c, msg[2],   9, 0xfcefa3f8); // 30
    MD5FUNC(GG, c, d, a, b, msg[7],  14, 0x676f02d9); // 31
    MD5FUNC(GG, b, c, d, a, msg[12], 20, 0x8d2a4c8a); // 32

    // Round 3
    MD5FUNC(HH, a, b, c, d, msg[5],   4, 0xfffa3942); // 33
    MD5FUNC(HH, d, a, b, c, msg[8],  11, 0x8771f681); // 34
    MD5FUNC(HH, c, d, a, b, msg[11], 16, 0x6d9d6122); // 35
    MD5FUNC(HH, b, c, d, a, msg[14], 23, 0xfde5380c); // 36
    MD5FUNC(HH, a, b, c, d, msg[1],   4, 0xa4beea44); // 37
    MD5FUNC(HH, d, a, b, c, msg[4],  11, 0x4bdecfa9); // 38
    MD5FUNC(HH, c, d, a, b, msg[7],  16, 0xf6bb4b60); // 39
    MD5FUNC(HH, b, c, d, a, msg[10], 23, 0xbebfbc70); // 40
    MD5FUNC(HH, a, b, c, d, msg[13],  4, 0x289b7ec6); // 41
    MD5FUNC(HH, d, a, b, c, msg[0],  11, 0xeaa127fa); // 42
    MD5FUNC(HH, c, d, a, b, msg[3],  16, 0xd4ef3085); // 43
    MD5FUNC(HH, b, c, d, a, msg[6],  23, 0x4881d05 ); // 44
    MD5FUNC(HH, a, b, c, d, msg[9],   4, 0xd9d4d039); // 45
    MD5FUNC(HH, d, a, b, c, msg[12], 11, 0xe6db99e5); // 46
    MD5FUNC(HH, c, d, a, b, msg[15], 16, 0x1fa27cf8); // 47
    MD5FUNC(HH, b, c, d, a, msg[2],  23, 0xc4ac5665); // 48

    // Round 4
    MD5FUNC(II, a, b, c, d, msg[0],   6, 0xf4292244); // 49
    MD5FUNC(II, d, a, b, c, msg[7],  10, 0x432aff97); // 50
    MD5FUNC(II, c, d, a, b, msg[14], 15, 0xab9423a7); // 51
    MD5FUNC(II, b, c, d, a, msg[5],  21, 0xfc93a039); // 52
    MD5FUNC(II, a, b, c, d, msg[12],  6, 0x655b59c3); // 53
    MD5FUNC(II, d, a, b, c, msg[3],  10, 0x8f0ccc92); // 54
    MD5FUNC(II, c, d, a, b, msg[10], 15, 0xffeff47d); // 55
    MD5FUNC(II, b, c, d, a, msg[1],  21, 0x85845dd1); // 56
    MD5FUNC(II, a, b, c, d, msg[8],   6, 0x6fa87e4f); // 57
    MD5FUNC(II, d, a, b, c, msg[15], 10, 0xfe2ce6e0); // 58
    MD5FUNC(II, c, d, a, b, msg[6],  15, 0xa3014314); // 59
    MD5FUNC(II, b, c, d, a, msg[13], 21, 0x4e0811a1); // 60
    MD5FUNC(II, a, b, c, d, msg[4],   6, 0xf7537e82); // 61
    MD5FUNC(II, d, a, b, c, msg[11], 10, 0xbd3af235); // 62
    MD5FUNC(II, c, d, a, b, msg[2],  15, 0x2ad7d2bb); // 63
    MD5FUNC(II, b, c, d, a, msg[9],  21, 0xeb86d391); // 64

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

// MD5更新
HLM_VOID HLM_COM_md5_update(HLM_MD5  *md5,
                            HLM_VOID *buf_t,
                            HLM_U32   len)
{
    HLM_U08 *buf      = HLM_NULL;
    HLM_U32  i        = 0;
    HLM_U32  idx      = 0;
    HLM_U32  part_len = 0;

    buf = (HLM_U08*)buf_t;
    idx = (HLM_U32)((md5->bits[0] >> 3) & 0x3f);
    md5->bits[0] += (len << 3);

    if (md5->bits[0] < (len << 3))
    {
        (md5->bits[1])++;
    }
    md5->bits[1] += (len >> 29);
    part_len = 64 - idx;

    if (len >= part_len)
    {
        memcpy(md5->msg + idx, buf, part_len);
        HLM_COM_md5_trans(md5->h, (HLM_U32 *)md5->msg);
        for (i = part_len; i + 63 < len; i += 64)
        {
            HLM_COM_md5_trans(md5->h, (HLM_U32 *)(buf + i));
        }
        idx = 0;
    }
    else
    {
        i = 0;
    }
    if (len - i > 0)
    {
        memcpy(md5->msg + idx, buf + i, len - i);
    }
}

// MD5结尾
HLM_VOID HLM_COM_md5_finish(HLM_MD5 *md5,
                            HLM_U08  digest[16])
{
    HLM_U08 *pos = HLM_NULL;
    HLM_S32  cnt = 0;

    cnt = (md5->bits[0] >> 3) & 0x3F;
    pos = md5->msg + cnt;
    *pos++ = 0x80;
    cnt = 64 - 1 - cnt;
    if (cnt < 8)
    {
        memset(pos, 0, cnt);
        HLM_COM_md5_trans(md5->h, (HLM_U32 *)md5->msg);
        memset(md5->msg, 0, 56);
    }
    else
    {
        memset(pos, 0, cnt - 8);
    }
    memcpy((md5->msg + 14 * sizeof(HLM_U32)), &md5->bits[0], sizeof(HLM_U32));
    memcpy((md5->msg + 15 * sizeof(HLM_U32)), &md5->bits[1], sizeof(HLM_U32));
    HLM_COM_md5_trans(md5->h, (HLM_U32 *)md5->msg);
    memcpy(digest, md5->h, 16);
    memset(md5, 0, sizeof(HLM_MD5));
}

/***************************************************************************************************
* 功  能：计算MD5
* 参  数：*
*        yuv_buf                  -I       图像数据
*        digest                   -O       MD5值
*        luma_width               -I       图像亮度宽度
*        luma_height              -I       图像亮度高度
*        chroma_width             -I       图像色度宽度
*        chroma_height            -I       图像色度高度
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_GetMd5(HLM_U16 *y_buf,
                        HLM_U16 *cb_buf,
                        HLM_U16 *cr_buf,
                        HLM_U08  digest[16],
                        HLM_U32  luma_width,
                        HLM_U32  luma_height,
                        HLM_U32  chroma_width,
                        HLM_U32  chroma_height)
{
    HLM_MD5 md5 = { 0 };
    HLM_U32 j   = 0;

    md5.h[0] = 0x67452301;
    md5.h[1] = 0xefcdab89;
    md5.h[2] = 0x98badcfe;
    md5.h[3] = 0x10325476;
    md5.bits[0] = 0;
    md5.bits[1] = 0;

    //luma
    for (j = 0; j < luma_height; j++)
    {
        HLM_COM_md5_update(&md5,
            (HLM_U08 *)y_buf + j * luma_width * sizeof(HLM_U16),
            luma_width * sizeof(HLM_U16));
    }

    //chroma
    for (j = 0; j < chroma_height; j++)
    {
        HLM_COM_md5_update(&md5,
            (HLM_U08 *)cb_buf + j * chroma_width * sizeof(HLM_U16),
            chroma_width * sizeof(HLM_U16));
    }
    for (j = 0; j < chroma_height; j++)
    {
        HLM_COM_md5_update(&md5,
            (HLM_U08 *)cr_buf + j * chroma_width * sizeof(HLM_U16),
            chroma_width * sizeof(HLM_U16));
    }

    HLM_COM_md5_finish(&md5, digest);
}

/***************************************************************************************************
* 功  能：CSC 颜色空间转换（RGB -> YCbCr）
* 参  数：*
*        src0                     -IO      第一分量数据
*        src1                     -IO      第二分量数据
*        src2                     -IO      第三分量数据
*        width                    -I       宽度
*        height                   -I       高度
*        bitdepth                 -I       比特深度
* 返回值：无
* 备  注：
*      Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
*      Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + 128
*      Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + 128
***************************************************************************************************/
HLM_VOID HLM_COM_Rgb_To_YCbCr(HLM_U16 *src0,
                              HLM_U16 *src1,
                              HLM_U16 *src2,
                              HLM_S32  width,
                              HLM_S32  height,
                              HLM_U08  bitdepth)
{
    HLM_U32  x         = 0;
    HLM_F64  tmp0      = 0;
    HLM_F64  tmp1      = 0;
    HLM_F64  tmp2      = 0;
    HLM_U32  numPx     = width * height;
    HLM_U16 *p_r       = src0;
    HLM_U16 *p_g       = src1;
    HLM_U16 *p_b       = src2;
    HLM_U16 *p_y       = src0;
    HLM_U16 *p_u       = src1;
    HLM_U16 *p_v       = src2;
    HLM_U16  offset    = 1 << (bitdepth - 1);
    HLM_U16  max_value = (1 << bitdepth) - 1;

    for (x = 0; x < numPx; x++)
    {
#if COLOR_TYPE == 1        //BT.601
        tmp0 =   p_r[x] * 0.2990 + p_g[x] * 0.5870 + p_b[x] * 0.1140;
        tmp1 = - p_r[x] * 0.1687 - p_g[x] * 0.3313 + p_b[x] * 0.5000 + offset;
        tmp2 =   p_r[x] * 0.5000 - p_g[x] * 0.4187 - p_b[x] * 0.0813 + offset;
#elif COLOR_TYPE == 2     //BT.709
        tmp0 =   p_r[x] * 0.2126 + p_g[x] * 0.7152 + p_b[x] * 0.0722;
        tmp1 = - p_r[x] * 0.1146 - p_g[x] * 0.3854 + p_b[x] * 0.5000 + offset;
        tmp2 =   p_r[x] * 0.5000 - p_g[x] * 0.4542 - p_b[x] * 0.0458 + offset;
#elif COLOR_TYPE == 3     //BT.2020
        tmp0 =   p_r[x] * 0.2627 + p_g[x] * 0.6780 + p_b[x] * 0.0593;
        tmp1 = - p_r[x] * 0.1396 - p_g[x] * 0.3604 + p_b[x] * 0.5000 + offset;
        tmp2 =   p_r[x] * 0.5000 - p_g[x] * 0.4598 - p_b[x] * 0.0402 + offset;
#endif
        p_y[x] = HLM_CLIP((HLM_S32)HLM_ROUND(tmp0), 0, max_value);
        p_u[x] = HLM_CLIP((HLM_S32)HLM_ROUND(tmp1), 0, max_value);
        p_v[x] = HLM_CLIP((HLM_S32)HLM_ROUND(tmp2), 0, max_value);
    }
}

/***************************************************************************************************
* 功  能：CSC 颜色空间转换（YCbCr -> RGB）
* 参  数：*
*        src0                     -IO      第一分量数据
*        src1                     -IO      第二分量数据
*        src2                     -IO      第三分量数据
*        width                    -I       宽度
*        height                   -I       高度
*        bitdepth                 -I       比特深度
* 返回值：无
* 备  注：
*      R  =  Y + 1.402 * (Cr - 128)
*      G  =  Y - 0.344 * (Cb - 128) - 0.714 *( Cr - 128)
*      B  =  Y + 1.772 * (Cb - 128)
***************************************************************************************************/
HLM_VOID HLM_COM_YCbCr_To_Rgb(HLM_U16 *src0,
                              HLM_U16 *src1,
                              HLM_U16 *src2,
                              HLM_S32  width,
                              HLM_S32  height,
                              HLM_U08  bitdepth)
{
    HLM_S32 x         = 0;
    HLM_F64 tmp0      = 0;
    HLM_F64 tmp1      = 0;
    HLM_F64 tmp2      = 0;
    HLM_S32 numPx     = width * height;
    HLM_U16 *p_y      = src0;
    HLM_U16 *p_u      = src1;
    HLM_U16 *p_v      = src2;
    HLM_U16 *p_r      = src0;
    HLM_U16 *p_g      = src1;
    HLM_U16 *p_b      = src2;
    HLM_U16 offset    = 1 << (bitdepth - 1);
    HLM_U16 max_value = (1 << bitdepth) - 1;

    for (x = 0; x < numPx; x++)
    {
#if COLOR_TYPE == 1        //BT.601
        tmp0 = p_y[x]                               + (p_v[x] - offset) * 1.402;
        tmp1 = p_y[x] - (p_u[x] - offset) * 0.34414 - (p_v[x] - offset) * 0.7141;
        tmp2 = p_y[x] + (p_u[x] - offset) * 1.772;
#elif COLOR_TYPE == 2     //BT.709
        tmp0 = p_y[x] + (p_v[x] - offset) * 1.5748;
        tmp1 = p_y[x] - (p_u[x] - offset) * 0.1873 - (p_v[x] - offset) * 0.4681;
        tmp2 = p_y[x] + (p_u[x] - offset) * 1.8556;
#elif COLOR_TYPE == 3     //BT.2020
        tmp0 = p_y[x] + (p_v[x] - offset) * 1.4746;
        tmp1 = p_y[x] - (p_u[x] - offset) * 0.1646 - (p_v[x] - offset) * 0.5714;
        tmp2 = p_y[x] + (p_u[x] - offset) * 1.8814;
#endif

        p_r[x] = HLM_CLIP((HLM_S32)HLM_ROUND(tmp0), 0, max_value);
        p_g[x] = HLM_CLIP((HLM_S32)HLM_ROUND(tmp1), 0, max_value);
        p_b[x] = HLM_CLIP((HLM_S32)HLM_ROUND(tmp2), 0, max_value);
    }
}

/***************************************************************************************************
* 功  能：获取每个4x4块对应的系数位置
* 参  数：*
*        num_4x4                  -I       4x4块个数
*        idx_4                    -I       4x4块顺序
*        pos_x                    -IO      横坐标
*        pos_y                    -IO      纵坐标
*        com_cu_info              -IO      当前CU的信息
*        yuv_idx                  -I       分量索引
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_4x4_Block_To_Coeff_Pos(HLM_U08          num_4x4,
                                        HLM_S32          idx_4,
                                        HLM_U08         *pos_x,
                                        HLM_U08         *pos_y,
                                        HLM_CU_INFO     *com_cu_info,
                                        HLM_U08          yuv_idx)
{
    HLM_U32 pu_w        = 0;
    HLM_U32 pu_h        = 0;
    HLM_U32 i           = 0;
    HLM_U32 j           = 0;
    HLM_U32 pu_pos      = 0;
    HLM_COEFF *coef_src = HLM_NULL;

    switch (num_4x4)
    {
    case 8:
        pos_x[0] = HLM_B4_X_8[idx_4];
        pos_y[0] = HLM_B4_Y_8[idx_4];
        break;
    case 4:
        pos_x[0] = HLM_B4_X_4[idx_4];
        pos_y[0] = HLM_B4_Y_4[idx_4];
        break;
    case 2:
        pos_x[0] = HLM_B4_X_2[idx_4];
        pos_y[0] = HLM_B4_Y_2[idx_4];
        break;
    default:
        assert(0);
    }

    com_cu_info->coeffs_num[yuv_idx][pos_y[0]][pos_x[0]] = 0;
    pu_w = (com_cu_info->cu_type == HLM_IBC_4x4 && com_cu_info->ts_flag == 1) ? 1 << com_cu_info->cu_width[yuv_idx] >> 2 : 4;   // 水平4个PU
    pu_h = (com_cu_info->cu_type == HLM_IBC_4x4 && com_cu_info->ts_flag == 1) ? 1 << com_cu_info->cu_height[yuv_idx] >> 1 : 4;  // 垂直2个PU
    pu_pos = (pos_y[0] * pu_h) * HLM_WIDTH_SIZE + (pos_x[0] * pu_w);
    coef_src = com_cu_info->cu_pred_info.coeff[yuv_idx] + pu_pos;
    for (i = 0; i < pu_h; i++)
    {
        for (j = 0; j < pu_w; j++)
        {
            if (coef_src[j] != 0)
                com_cu_info->coeffs_num[yuv_idx][pos_y[0]][pos_x[0]]++;
        }
        coef_src += HLM_WIDTH_SIZE;
    }
}

/***************************************************************************************************
* 功  能：获取不同格式下、不同分量的水平方向和垂直方向的采样率
* 参  数：*
*        image_format             -I       图像格式
*        hor_shift                -O       水平采样率
*        ver_shift                -O       垂直采样率
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_COM_GetFormatShift(HLM_U32  image_format,
                                HLM_S08  hor_shift[3],
                                HLM_S08  ver_shift[3])
{
    memset(hor_shift, 0, 3 * sizeof(HLM_S08));
    memset(ver_shift, 0, 3 * sizeof(HLM_S08));

    if (image_format == HLM_IMG_YUV_420)
    {
        hor_shift[1] = hor_shift[2] = 1;
        ver_shift[1] = ver_shift[2] = 1;
    }
    else if (image_format == HLM_IMG_YUV_422)
    {
        hor_shift[1] = hor_shift[2] = 1;
        ver_shift[1] = ver_shift[2] = 0;
    }
}

/***************************************************************************************************
* 功  能：获取src上的(blk_x,blk_y)位置处的宽高为blk_w和blk_h的像素块
* 参  数：*
*        src                      -I       数据源指针
*        src_stride               -I       数据源stride
*        blk                      -O       像素块指针
*        blk_stride               -I       像素块stride
*        blk_x                    -I       像素块左上角点的横坐标
*        blk_y                    -I       像素块左上角点的纵坐标
*        blk_w                    -I       像素块宽度
*        blk_h                    -I       像素块高度
* 返回值：无
***************************************************************************************************/
HLM_VOID HLM_COM_GetBlock(HLM_U16  *src,
                          HLM_U32   src_stride,
                          HLM_U16  *blk,
                          HLM_U32   blk_stride,
                          HLM_U32   blk_x,
                          HLM_U32   blk_y,
                          HLM_U32   blk_w,
                          HLM_U32   blk_h)
{
    HLM_U16 *s = src + blk_y * src_stride + blk_x;
    HLM_U16 *d = blk;
    HLM_U32  i = 0;

    for (i = 0; i < blk_h; i++)
    {
        memcpy(d, s, blk_w * sizeof(HLM_U16));
        s += src_stride;
        d += blk_stride;
    }
}

/***************************************************************************************************
* 功  能：校验Patch划分能否拼成完整图像
* 参  数：*
*        pic_width                -I       图像宽度
*        pic_height               -I       图像高度
*        patch_info               -I       patch信息
* 返回值：校验是否通过(0/1)
* 备  注：
***************************************************************************************************/
HLM_S32 HLM_COM_CheckPatchSplit(HLM_S32            pic_width,
                                HLM_S32            pic_height,
                                HLM_PATCH_INFO    *patch_info)
{
    HLM_U32 i               = 0;
    HLM_U32 j               = 0;
    HLM_U32 sum             = 0;
    HLM_U32 hor_non_overlap = 0;
    HLM_U32 ver_non_overlap = 0;
    HLM_U32 patch_num       = patch_info->patch_num;

    // 检查总面积
    for (i = 0; i < patch_num; i++)
    {
        sum += patch_info->patch_param[i].patch_width[0] * patch_info->patch_param[i].patch_height[0];
    }
    if (sum != pic_width * pic_height)
    {
        return 0;
    }

    // 检查边界
    for (i = 0; i < patch_num; i++)
    {
        if (patch_info->patch_param[i].patch_x < 0 ||
            patch_info->patch_param[i].patch_y < 0 ||
            patch_info->patch_param[i].patch_x + patch_info->patch_param[i].patch_width[0] >(HLM_U32)pic_width ||
            patch_info->patch_param[i].patch_y + patch_info->patch_param[i].patch_height[0] >(HLM_U32)pic_height)
        {
            return 0;
        }
    }

    // 检查重叠
    for (i = 0; i < patch_num; i++)
    {
        for (j = i + 1; j < patch_num; j++)
        {
            hor_non_overlap = PATCH_HOR_OVERLAP(patch_info->patch_param[i].patch_x,
                patch_info->patch_param[i].patch_width[0],
                patch_info->patch_param[j].patch_x,
                patch_info->patch_param[j].patch_width[0]);
            ver_non_overlap = PATCH_HOR_OVERLAP(patch_info->patch_param[i].patch_y,
                patch_info->patch_param[i].patch_height[0],
                patch_info->patch_param[j].patch_y,
                patch_info->patch_param[j].patch_height[0]);
            if (!hor_non_overlap && !ver_non_overlap)  // 水平和垂直都有重叠
            {
                return 0;
            }
        }
    }

    return 1;
}
