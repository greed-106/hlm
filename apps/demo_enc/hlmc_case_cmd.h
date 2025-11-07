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
#ifndef _HLMC_CASE_CMD_H_
#define _HLMC_CASE_CMD_H_

#include "hlmc_case_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* 功  能：命令行参数传递给全局变量
* 参  数：*
*        cmd                      -I       命令行数据
*        parm                     -O       文件参数
*        coding_ctrl              -O       编码控制参数
*        rate_ctrl                -O       码控参数
*        dpb_ref_ctrl             -O       dpb参数
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_CMD_SetParameter(HLMC_DEMO_CMD        *cmd,
                               HLMC_CASE_CFG_PARAM  *parm,
                               HLMC_CODING_CTRL     *coding_ctrl,
                               HLMC_RATE_CTRL       *rate_ctrl,
                               HLMC_DPB_REF_CTRL    *dpb_ref_ctrl);

/***************************************************************************************************
* 功  能：设置命令行默认参数
* 参  数：*
*        cmd                      -I       命令行数据
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_CMD_SetDefaultCommand(HLMC_DEMO_CMD *cmd);

/***************************************************************************************************
* 功  能：解析命令行
* 参  数：*
*        agc                      -I       命令行参数个数
*        argv                     -I       命令行参数地址
*        cmd                      -O       命令行数据
* 返回值：0：成功；-1：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CMD_ParseCommand(int              argc,
                              char           **argv,
                              HLMC_DEMO_CMD   *cmd);

/***************************************************************************************************
* 功  能：校验命令行参数
* 参  数：*
*        cmd                      -I       命令行数据
* 返回值：0：成功；-1：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CMD_CheckCommand(HLMC_DEMO_CMD *cmd);

#ifdef __cplusplus
}
#endif

#endif // _HLMC_CASE_CMD_H_
