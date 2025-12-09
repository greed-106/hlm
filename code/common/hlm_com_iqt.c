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
#include "hlm_com_iqt.h"

/***************************************************************************************
* 功  能：对像素域的残差系数进行反量化
* 参  数：*
*         src           -I      输入数据指针
*         dst           -O      反量化数据指针
*         blk_w         -I      块宽度
*         blk_h         -I      块高度
*         src_stride    -I      源地址stride
*         dst_stride    -I      目标地址stride
*         qp            -I      量化参数
* 返回值：无
* 备  注：
****************************************************************************************/
HLM_VOID HLM_COM_PixelDequant(HLM_COEFF  *src,
                              HLM_COEFF  *dst,
                              HLM_U32     blk_w,
                              HLM_U32     blk_h,
                              HLM_U32     src_stride,
                              HLM_U32     dst_stride,
#if QT_CLIP
                              HLM_U08     bitdepth,
#endif
                              HLM_U08     qp)
{
    HLM_S32 scale   = tbl_pixel_dequant_scale[qp % 6];
    HLM_S32 shift   = HLM_QUANT_SHIFT - (qp / 6);
    HLM_S32 offset  = shift > 0 ? (1 << (shift - 1)) : 0;
    HLM_S32 src_val = 0;
    HLM_S32 tmp     = 0;
    HLM_U32 x       = 0;
    HLM_U32 y       = 0;
#if QT_CLIP
    HLM_COEFF max_val = (1 << bitdepth) - 1;
#endif

    for (y = 0; y < blk_h; y++)
    {
        for (x = 0; x < blk_w; x++)
        {
            src_val = src[y * src_stride + x];
            tmp = HLM_ABS(src_val);
            if (shift > 0)
            {
                tmp = (tmp * scale + offset) >> shift;
            }
            else  // 支持更大的qp
            {
                tmp = (tmp * scale) << (-shift);
            }
            dst[y * dst_stride + x] = src_val < 0 ? (HLM_COEFF)(-tmp) : (HLM_COEFF)tmp;
#if QT_CLIP
            dst[y * dst_stride + x] = HLM_CLIP(dst[y * dst_stride + x], -max_val - 1, max_val);
#endif
        }
    }
}

/***************************************************************************************
* 功  能：对频域的残差系数进行反量化
* 参  数：*
*         src           -I      输入数据指针
*         qp            -I      量化参数
*         blk_w         -I      块宽度
*         blk_h         -I      块高度
*         src_stride    -I      源地址stride
*         dst_stride    -I      目标地址stride
*         v             -I      反量化过程的参数
*         dst           -O      反量化数据指针
* 返回值：无
* 备  注：
****************************************************************************************/
HLM_VOID HLM_COM_Dequant4x4(HLM_COEFF   *src,
                            HLM_U08      qp,
                            HLM_U32      blk_w,
                            HLM_U32      blk_h,
                            HLM_U32      src_stride,
                            HLM_U32      dst_stride,
                            HLM_S32     *v,
#if QT_CLIP
                            HLM_U08      bitdepth,
#endif
                            HLM_COEFF   *dst)
{
    HLM_S32 tmp_coeff = 0;
    HLM_U08 i         = 0;
    HLM_U08 j         = 0;
#if QT_CLIP
    HLM_COEFF max_val = bitdepth == 8 ? ((1 << (bitdepth + 6)) - 1) : ((1 << (bitdepth + 5)) - 1);
#endif

    for (j = 0; j < blk_h; j++)
    {
        for (i = 0; i < blk_w; i++)
        {
            if (qp >= 24)
            {
                tmp_coeff = (src[i] * v[i]) << ((qp / 6) - 4);
            }
            else
            {
                tmp_coeff = (src[i] * v[i] + (1 << (3 - (qp / 6)))) >> (4 - (qp / 6));
            }
#if QT_CLIP
            dst[i] = HLM_CLIP((HLM_COEFF)tmp_coeff, -max_val - 1, max_val);
#else
            dst[i] = (HLM_COEFF)tmp_coeff;
#endif
        }
        src += src_stride;
        dst += dst_stride;
        v += blk_w;
    }
}

/***************************************************************************************
* 功  能：对反量化后数据进行逆DCT
* 参  数：*
*         src           -I      输入数据指针
*         dst           -O      反量化数据指针
*         blk_w         -I      块宽度
*         blk_h         -I      块高度
*         src_stride    -I      源地址stride
*         dst_stride    -I      目标地址stride
*         bitdepth      -I      比特位宽
* 返回值：无
* 备  注：
****************************************************************************************/
HLM_VOID HLM_COM_Idct4x4(HLM_COEFF      *src,
                         HLM_COEFF      *dst,
                         HLM_U32         blk_w,
                         HLM_U32         blk_h,
                         HLM_U32         src_stride,
                         HLM_U32         dst_stride,
                         HLM_S16         bitdepth)
{
    HLM_COEFF tmp[16]  = { 0 };
    HLM_U32   i        = 0;
    HLM_COEFF p0       = 0;
    HLM_COEFF p1       = 0;
    HLM_COEFF p2       = 0;
    HLM_COEFF p3       = 0;
    HLM_COEFF t0       = 0;
    HLM_COEFF t1       = 0;
    HLM_COEFF t2       = 0;
    HLM_COEFF t3       = 0;
    HLM_COEFF *src_tmp = src;
#if QT_CLIP
    HLM_S32   shift   = bitdepth == 8 ? 6 : 5;
    HLM_U08   offset  = 1 << (shift - 1);
    HLM_COEFF max_val = (1 << bitdepth) - 1;
#else
    HLM_S32   shift = bitdepth == 8 ? 6 + bitdepth : 5 + bitdepth;
    HLM_COEFF max_val = (1 << shift) - 1;
#endif

    // 水平
    for (i = 0; i < blk_w; i++)
    {
        t0 = src_tmp[0];
        t1 = src_tmp[1];
        t2 = src_tmp[2];
        t3 = src_tmp[3];

        p0 = t0 + t2;
        p1 = t0 - t2;
        p2 = (t1 >> 1) - t3;
        p3 = t1 + (t3 >> 1);

        src_tmp += src_stride;

        tmp[0 + (i << 2)] = p0 + p3;
        tmp[1 + (i << 2)] = p1 + p2;
        tmp[2 + (i << 2)] = p1 - p2;
        tmp[3 + (i << 2)] = p0 - p3;
    }

    // 垂直
    for (i = 0; i < blk_h; i++)
    {
        t0 = tmp[(0 << 2) + i];
        t1 = tmp[(1 << 2) + i];
        t2 = tmp[(2 << 2) + i];
        t3 = tmp[(3 << 2) + i];

        p0 = t0 + t2;
        p1 = t0 - t2;
        p2 = (t1 >> 1) - t3;
        p3 = t1 + (t3 >> 1);

#if QT_CLIP
        dst[(0 * dst_stride) + i] = HLM_CLIP(((p0 + p3) + offset) >> shift, -max_val - 1, max_val);
        dst[(1 * dst_stride) + i] = HLM_CLIP(((p1 + p2) + offset) >> shift, -max_val - 1, max_val);
        dst[(2 * dst_stride) + i] = HLM_CLIP(((p1 - p2) + offset) >> shift, -max_val - 1, max_val);
        dst[(3 * dst_stride) + i] = HLM_CLIP(((p0 - p3) + offset) >> shift, -max_val - 1, max_val);
#else
        dst[(0 * dst_stride) + i] = HLM_CLIP(p0 + p3, -max_val - 1, max_val);
        dst[(1 * dst_stride) + i] = HLM_CLIP(p1 + p2, -max_val - 1, max_val);
        dst[(2 * dst_stride) + i] = HLM_CLIP(p1 - p2, -max_val - 1, max_val);
        dst[(3 * dst_stride) + i] = HLM_CLIP(p0 - p3, -max_val - 1, max_val);
#endif
    }
}

/***************************************************************************************************
* 功  能：一个block加上残差
* 参  数：*
*         block0          -O    第一块block的起始地址
*         block1          -I    第二块block的起始地址
*         res             -I    残差的地址
*         bitdepth        -I    比特位宽
*         block_width     -I    块宽度
*         block_height    -I    块高度
*         block0_stride   -I    第一块block的跨度
*         block1_stride   -I    第二块block的跨度
*         res_stride      -I    残差的跨度
* 返回值：
* 备  注：block0 = block1 + res
***************************************************************************************************/
HLM_VOID HLM_COM_AddRes(HLM_U16        *block0,
                        HLM_U16        *block1,
                        HLM_COEFF      *res,
                        HLM_U08         bitdepth,
                        HLM_U32         block_width,
                        HLM_U32         block_height,
                        HLM_U32         block0_stride,
                        HLM_U32         block1_stride,
                        HLM_U32         res_stride)
{
    HLM_U32  i          = 0;
    HLM_U32  j          = 0;
    HLM_COEFF *res_tmp  = res;
    HLM_U16 *block0_tmp = block0;
    HLM_U16 *block1_tmp = block1;
    HLM_U16  max_val    = (1 << bitdepth) - 1;

    for (i = 0; i < block_height; i++)
    {
        for (j = 0; j < block_width; j++)
        {
            block0_tmp[j] = HLM_CLIP(block1_tmp[j] + res_tmp[j], 0, max_val);
        }
        res_tmp    += res_stride;
        block0_tmp += block0_stride;
        block1_tmp += block1_stride;
    }
}
