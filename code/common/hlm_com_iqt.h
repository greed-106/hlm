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
#ifndef _HIM_COM_IQT_H_
#define _HIM_COM_IQT_H_

#include "hlm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HLM_QUANT_SHIFT                (9)       // 量化移位，qp=0~51，qp/6 <= 8

// 像素域量化表，Qstep = 2^( (QP-4)/6 )
static const HLM_U16 tbl_pixel_quant_scale[6]   = { 813, 724, 645, 575, 512, 456 };
static const HLM_U16 tbl_pixel_dequant_scale[6] = { 323, 362, 406, 456, 512, 575 };

// 频域量化表
static const HLM_S32 HLM_DEQUANT_V[6][3] =
{
    { 10, 13, 16 },
    { 11, 14, 18 },
    { 13, 16, 20 },
    { 14, 18, 23 },
    { 16, 20, 25 },
    { 18, 23, 29 }
};

/***************************************************************************************
* 功  能：对非零残差系数进行反量化
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
                              HLM_U08     qp);

/***************************************************************************************
* 功  能：对量化后/反变换后系数进行反量化
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
                            HLM_COEFF   *dst);

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
                         HLM_S16         bitdepth);

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
                        HLM_U32         res_stride);

#ifdef __cplusplus
}
#endif

#endif //_HIM_COM_IQT_H_
