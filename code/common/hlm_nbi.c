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
#include "hlm_nbi.h"

// 更新帧内预测模式
HLM_VOID HLM_NBI_update_intra_pred_mode(HLM_CU_INFO         *com_cu_info,
                                        HLM_NEIGHBOR_INFO   *nbi_info,
                                        HLM_PU_INFO         *blk8_pu0_info)
{
    HLM_U08 i = 0;

    for (i = 0; i < 2; i++)
    {
        if (HLM_I_4x4 == com_cu_info->cu_type)
        {
            nbi_info->intra_pred_mode_left[i] = com_cu_info->intra_8x8_enable_flag
#if INTRA_CHROMA_MODE_SEPARATE
                                                ? com_cu_info->cu_pred_info.pu_info[1].intra_pred_mode[0]
                                                : com_cu_info->cu_pred_info.pu_info[(i << 2) + 3].intra_pred_mode[0];
#else
                                                ? com_cu_info->cu_pred_info.pu_info[1].intra_pred_mode
                                                : com_cu_info->cu_pred_info.pu_info[(i << 2) + 3].intra_pred_mode;
#endif
        }
        else if (HLM_I_16x8 == com_cu_info->cu_type)
        {
#if INTRA_CHROMA_MODE_SEPARATE
            nbi_info->intra_pred_mode_left[i] = blk8_pu0_info->intra_pred_mode[0];
#else
            nbi_info->intra_pred_mode_left[i] = blk8_pu0_info->intra_pred_mode;
#endif
        }
    }
    for (i = 0; i < 4; i++)
    {
        if (HLM_I_4x4 == com_cu_info->cu_type)
        {
            nbi_info->intra_pred_mode_up[(com_cu_info->cu_x << 2) + i] = com_cu_info->intra_8x8_enable_flag
#if INTRA_CHROMA_MODE_SEPARATE
                                                                       ? com_cu_info->cu_pred_info.pu_info[i >> 1].intra_pred_mode[0]
                                                                       : com_cu_info->cu_pred_info.pu_info[4 + i].intra_pred_mode[0];
#else
                                                                       ? com_cu_info->cu_pred_info.pu_info[i >> 1].intra_pred_mode
                                                                       : com_cu_info->cu_pred_info.pu_info[4 + i].intra_pred_mode;
#endif
        }
        else if (HLM_I_16x8 == com_cu_info->cu_type)
        {
#if INTRA_CHROMA_MODE_SEPARATE
            nbi_info->intra_pred_mode_up[(com_cu_info->cu_x << 2) + i] = blk8_pu0_info->intra_pred_mode[0];
#else
            nbi_info->intra_pred_mode_up[(com_cu_info->cu_x << 2) + i] = blk8_pu0_info->intra_pred_mode;
#endif
        }
    }
}

// 更新cu_type
HLM_VOID HLM_NBI_update_pred_type(HLM_CU_INFO         *com_cu_info,
                                  HLM_NEIGHBOR_INFO   *nbi_info)
{
    HLM_U08 i = 0;

    if (com_cu_info->cu_x > 0 && com_cu_info->cu_y > 0)
    {
        nbi_info->pred_type_upleft = nbi_info->pred_type_up[(com_cu_info->cu_x << 2) + 3];
    }
    for (i = 0; i < 2; i++)
    {
        nbi_info->pred_type_left[i] = com_cu_info->cu_type;
    }
    for (i = 0; i < 4; i++)
    {
        nbi_info->pred_type_up[(com_cu_info->cu_x << 2) + i] = com_cu_info->cu_type;
    }
}

// 更新帧间mv
HLM_VOID HLM_NBI_update_inter_mv(HLM_CU_INFO         *com_cu_info,
                                 HLM_NEIGHBOR_INFO   *nbi_info,
                                 HLM_PU_INFO         *blk8_pu0_info,
                                 HLM_PU_INFO         *blk8_pu1_info)
{
    HLM_U08 i      = 0;
    HLM_MV zero_mv = { 0 };

    if (com_cu_info->cu_x > 0 && com_cu_info->cu_y > 0)
    {
        nbi_info->inter_mv_upleft = nbi_info->inter_mv_up[(com_cu_info->cu_x << 2) + 3];
    }
    if (HLM_P_16x8 == com_cu_info->cu_type || HLM_P_SKIP == com_cu_info->cu_type)
    {
        for (i = 0; i < 2; i++)
        {
            nbi_info->inter_mv_left[i] = blk8_pu0_info->inter_mv;
        }
        for (i = 0; i < 4; i++)
        {
            nbi_info->inter_mv_up[(com_cu_info->cu_x << 2) + i] = blk8_pu0_info->inter_mv;
        }
    }
    else if (HLM_P_8x8 == com_cu_info->cu_type)
    {
        for (i = 0; i < 2; i++)
        {
            nbi_info->inter_mv_left[i] = blk8_pu1_info->inter_mv;
        }
        nbi_info->inter_mv_up[(com_cu_info->cu_x << 2) + 0] = nbi_info->inter_mv_up[(com_cu_info->cu_x << 2) + 1] = blk8_pu0_info->inter_mv;
        nbi_info->inter_mv_up[(com_cu_info->cu_x << 2) + 2] = nbi_info->inter_mv_up[(com_cu_info->cu_x << 2) + 3] = blk8_pu1_info->inter_mv;
    }
    else  // intra模式存零矢量
    {
        for (i = 0; i < 2; i++)
        {
            nbi_info->inter_mv_left[i] = zero_mv;
        }
        for (i = 0; i < 4; i++)
        {
            nbi_info->inter_mv_up[(com_cu_info->cu_x << 2) + i] = zero_mv;
        }
    }
}

/***************************************************************************************************
* 功  能：相邻信息管理
* 参  数：*
*        cur_cu                 -I      当前CU信息
*        nbi_info               -IO     相邻信息数据
*        frame_type             -I      当前帧类型
*        blk8_pu0_info          -I      8x8划分下第一个PU的信息
*        blk8_pu1_info          -I      8x8划分下第二个PU的信息
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_NBI_Process(HLM_CU_INFO         *com_cu_info,
                         HLM_NEIGHBOR_INFO   *nbi_info,
                         HLM_U32              frame_type,
                         HLM_PU_INFO         *blk8_pu0_info,
                         HLM_PU_INFO         *blk8_pu1_info)
{
    HLM_NBI_update_intra_pred_mode(com_cu_info, nbi_info, blk8_pu0_info);
    HLM_NBI_update_pred_type(com_cu_info, nbi_info);
    if (HLM_FRAME_TYPE_P == frame_type)
    {
        HLM_NBI_update_inter_mv(com_cu_info, nbi_info, blk8_pu0_info, blk8_pu1_info);
    }
}

/***************************************************************************************************
* 功  能：更新相邻的重建像素
* 参  数：*
*        com_cu_info            -I      当前CU信息
*        nbi_info               -IO     相邻信息数据
*        recon_y_base           -I      亮度重建首地址
*        recon_cb_base          -I      色度重建首地址
*        recon_cr_base          -I      色度重建首地址
*        luma_stride            -I      亮度步长
*        chroma_stride          -I      色度步长
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLM_NBI_UpdateRec(HLM_CU_INFO         *com_cu_info,
                           HLM_NEIGHBOR_INFO   *nbi_info,
                           HLM_U16             *recon_y_base,
                           HLM_U16             *recon_cb_base,
                           HLM_U16             *recon_cr_base,
                           HLM_U32              luma_stride,
                           HLM_U32              chroma_stride)
{
    HLM_U08 i        = 0;
    HLM_U08 j        = 0;
    HLM_U16 *rec_adr = HLM_NULL;

    // Y 分量
    rec_adr = recon_y_base + (com_cu_info->cu_y << HLM_LOG2_HEIGHT_SIZE) * luma_stride + (com_cu_info->cu_x << 4);
    for (i = 0; i < HLM_HEIGHT_SIZE; i++)
    {
        nbi_info->intra_rec_left_y[i] = rec_adr[(HLM_WIDTH_SIZE - 1) + i * luma_stride];
    }
    if (com_cu_info->cu_y)
    {
        nbi_info->up_left_y = nbi_info->intra_rec_up_y[(com_cu_info->cu_x << 4) + 15];
    }
    for (i = 0; i < 16; i++)
    {
        nbi_info->intra_rec_up_y[(com_cu_info->cu_x << 4) + i] = rec_adr[(HLM_HEIGHT_SIZE - 1)* luma_stride + i];
    }

    // U 分量
    rec_adr = recon_cb_base + (com_cu_info->cu_y << com_cu_info->cu_height[1]) * chroma_stride + (com_cu_info->cu_x << com_cu_info->cu_width[1]);
    for (i = 0; i < (1 << com_cu_info->cu_height[1]); i++)
    {
        nbi_info->intra_rec_left_u[i] = rec_adr[((1 << com_cu_info->cu_width[1]) - 1) + i * chroma_stride];
    }
    if (com_cu_info->cu_y)
    {
        nbi_info->up_left_u = nbi_info->intra_rec_up_u[(com_cu_info->cu_x << com_cu_info->cu_width[1]) + (1 << com_cu_info->cu_width[1]) - 1];
    }
    for (i = 0; i < (1 << com_cu_info->cu_width[1]); i++)
    {
        nbi_info->intra_rec_up_u[(com_cu_info->cu_x << com_cu_info->cu_width[1]) + i] = rec_adr[((1 << com_cu_info->cu_height[1]) - 1)* chroma_stride + i];
    }

    // V 分量
    rec_adr = recon_cr_base + (com_cu_info->cu_y << com_cu_info->cu_height[2]) * chroma_stride + (com_cu_info->cu_x << com_cu_info->cu_width[2]);
    for (i = 0; i < (1 << com_cu_info->cu_height[2]); i++)
    {
        nbi_info->intra_rec_left_v[i] = rec_adr[((1 << com_cu_info->cu_width[2]) - 1) + i * chroma_stride];
    }
    if (com_cu_info->cu_y)
    {
        nbi_info->up_left_v = nbi_info->intra_rec_up_v[(com_cu_info->cu_x << com_cu_info->cu_width[2]) + (1 << com_cu_info->cu_width[2]) - 1];
    }
    for (i = 0; i < (1 << com_cu_info->cu_width[2]); i++)
    {
        nbi_info->intra_rec_up_v[(com_cu_info->cu_x << com_cu_info->cu_width[2]) + i] = rec_adr[((1 << com_cu_info->cu_height[2]) - 1)* chroma_stride + i];
    }
}
