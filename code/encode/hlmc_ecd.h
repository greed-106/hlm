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

static const HLM_U08 scan_table_1[3][HLM_TU_4x4_NUMS << 4] =
{
    {
         0,  1,  2,  3, 16, 17, 18, 19, 32, 33, 34, 35, 48, 49, 50, 51,
         4,  5,  6,  7, 20, 21, 22, 23, 36, 37, 38, 39, 52, 53, 54, 55,
         8,  9, 10, 11, 24, 25, 26, 27, 40, 41, 42, 43, 56, 57, 58, 59,
        12, 13, 14, 15, 28, 29, 30, 31, 44, 45, 46, 47, 60, 61, 62, 63,
        64, 65, 66, 67, 80, 81, 82, 83, 96, 97, 98, 99,112,113,114,115,
        68, 69, 70, 71, 84, 85, 86, 87,100,101,102,103,116,117,118,119,
        72, 73, 74, 75, 88, 89, 90, 91,104,105,106,107,120,121,122,123,
        76, 77, 78, 79, 92, 93, 94, 95,108,109,110,111,124,125,126,127
    },
    {
         0,  1,  2,  3, 16, 17, 18, 19, 32, 33, 34, 35, 48, 49, 50, 51,
         4,  5,  6,  7, 20, 21, 22, 23, 36, 37, 38, 39, 52, 53, 54, 55,
        64, 65, 66, 67, 80, 81, 82, 83, 96, 97, 98, 99,112,113,114,115,
        68, 69, 70, 71, 84, 85, 86, 87,100,101,102,103,116,117,118,119
    },
    {
         0,  1,  2,  3, 16, 17, 18, 19, 32, 33, 34, 35, 48, 49, 50, 51,
         4,  5,  6,  7, 20, 21, 22, 23, 36, 37, 38, 39, 52, 53, 54, 55
    }
};
static const  HLM_U08 scan_table_2[3][HLM_TU_4x4_NUMS << 4] =
{
    {
         0,  8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96,104,112,120,
         1,  9, 17, 25, 33, 41, 49, 57, 65, 73, 81, 89, 97,105,113,121,
         2, 10, 18, 26, 34, 42, 50, 58, 66, 74, 82, 90, 98,106,114,122,
         3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 91, 99,107,115,123,
         4, 12, 20, 28, 36, 44, 52, 60, 68, 76, 84, 92,100,108,116,124,
         5, 13, 21, 29, 37, 45, 53, 61, 69, 77, 85, 93,101,109,117,125,
         6, 14, 22, 30, 38, 46, 54, 62, 70, 78, 86, 94,102,110,118,126,
         7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95,103,111,119,127
    },
    {
         0,  4,  8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
         1,  5,  9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61,
         2,  6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62,
         3,  7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63
    },
    {
         0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
         1,  3,  5,  7,  9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31
    }
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
                              HLM_U32              intra_16x1_2x8_enable_flag,
                              HLM_U32              ibc_enable_flag);

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
                     HLM_U08                  sub_ibc_enable_flag);

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
