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
#ifndef _HLMC_ECD_H_
#define _HLMC_ECD_H_

#include "hlmc_defs.h"

// 无符号指数哥伦布的码长
static const HLM_U08 HLMC_UE_SIZE_TAB[256] =
{
     1, 1, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7,
     9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
};

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
                                HLMC_BITSTREAM  *bs);

/***************************************************************************************************
* 功  能：编码四字节的起始码0x00 00 00 01
* 参  数：*
*         bs                -I        码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_PutStartCode4Byte(HLMC_BITSTREAM *bs);

#if START_CODE_FIX
/***************************************************************************************************
* 功  能：编码三字节的起始码0x00 00 01
* 参  数：*
*         bs                -I        码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_PutStartCode3Byte(HLMC_BITSTREAM *bs);
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
HLM_VOID HLMC_ECD_PutNaluHeader(HLM_NAL_UNIT_TYPE    nal_unit_type,
                                HLM_U32              nal_ref_idc,
                                HLMC_BITSTREAM      *bs);

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
                               const HLM_S08   *syntax_element);

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
                              const HLM_S08   *syntax_element);

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
                           const HLM_S08   *syntax_element);

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
                           const HLM_S08   *syntax_element);

/***************************************************************************************************
* 功  能：拖尾比特编码
* 参  数：*
*         bs                -I         码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_RbspTrailingBits(HLMC_BITSTREAM *bs);

/***************************************************************************************************
* 功  能：在整个码流中添加防竞争码
* 参  数：*
*         src_bs                -I           不包含防竞争码的码流信息结构体
*         dst_bs                -I           包含防竞争码的码流信息结构体
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_ECD_WriteEmulaPreventBytes(HLMC_BITSTREAM *src_bs,
                                         HLMC_BITSTREAM *dst_bs);

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
                              HLM_U32              ibc_enable_flag);

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
                          HLM_U08            is_bitcount);

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
                                HLM_U08            is_bitcount);
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
                     HLM_U08                  ibc_enable_flag);

#if LINE_BY_LINE_4x1_RESI
HLM_VOID HLMC_ECD_resi_pred(HLMC_CU_INFO           *cur_cu,
                            HLM_COMPONENT           plane);
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
                                   HLM_COMPONENT           plane);

#endif // _HLMC_ECD_H_
