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
#include "hlmc_rc.h"
#include "hlmc_common.h"

// 计算复杂度
HLM_U08 HLMC_RC_get_complex(HLM_U32 input_satd,
                            HLM_U32 vf_th,
                            HLM_U32 f_th,
                            HLM_U32 c_th,
                            HLM_U32 vc_th)
{
    if (input_satd < vf_th)
    {
        return VERY_FLAT;
    }
    else if (input_satd < f_th)
    {
        return FLAT;
    }
    else if (input_satd > vc_th)
    {
        return VERY_COMPLEX;
    }
    else if (input_satd > c_th)
    {
        return COMPLEX;
    }
    else
    {
        return MIDDLE;
    }
}

// 模块所需状态内存计算和分配
HLM_STATUS HLMC_RC_alloc_status_buffer(HLMC_RC_SPEC  *spec,
                                       HLM_U08       *status_buf,
                                       HLM_SZT       *status_size)
{
    HLM_SZT  size      = 0;
    HLM_SZT  used_size = 0;

    // 真正需要申请status_buf时才需要下面这句校验
    HLM_CHECK_ERROR((HLM_NULL == status_buf), HLM_STS_ERR_NULL_PTR);

    //跳过spec结构体，在外面已分配，只计算内存大小
    size = sizeof(HLMC_RC_SPEC);
    size = HLM_SIZE_ALIGN_16(size);
    used_size += size;

    *status_size = used_size;

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：模块所需内存计算
* 参  数：*
*         status_size        -O         状态内存大小
*         work_size          -O         工作内存大小
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_RC_GetMemSize(HLM_SZT *status_size,
                              HLM_SZT *work_size)
{
    HLM_STATUS   sts       = HLM_STS_ERR;
    HLMC_RC_SPEC spec      = { 0 };
    HLM_SZT      status_sz = 0;
    HLM_SZT      work_sz   = 0;
    HLM_U08     *buf       = (HLM_U08 *)&spec;

    HLM_CHECK_ERROR((HLM_NULL == status_size), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((HLM_NULL == work_size), HLM_STS_ERR_NULL_PTR);

    memset(&spec, 0, sizeof(HLMC_RC_SPEC));

    sts = HLMC_RC_alloc_status_buffer(&spec, buf, &status_sz);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    if ((status_sz + work_sz) > HLM_MAX_MEM_SIZE)
    {
        return HLM_STS_ERR_OVER_MAX_MEM;
    }

    *status_size = status_sz;
    *work_size = work_sz;

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：模块创建（状态内存初始化，工作内存分配）
* 参  数：*
*         status_buf         -I        状态内存
*         work_buf           -I        工作内存
*         handle             -O        模块实例句柄
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_RC_Create(HLM_U08     *status_buf,
                          HLM_U08     *work_buf,
                          HLM_VOID   **handle)
{
    HLM_STATUS     sts         = HLM_STS_ERR;
    HLMC_RC_SPEC  *spec        = HLM_NULL;
    HLM_SZT        status_size = 0;
    HLM_SZT        work_size   = 0;

    HLM_CHECK_ERROR((HLM_NULL == handle), HLM_STS_ERR_NULL_PTR);

    // 初始化spec结构体中能力集参数
    spec = (HLMC_RC_SPEC *)status_buf;
    memset(spec, 0, sizeof(HLMC_RC_SPEC));

    // 分配内存
    sts = HLMC_RC_alloc_status_buffer(spec, status_buf, &status_size);
    HLM_CHECK_ERROR((HLM_STS_OK != sts), sts);

    *handle = (HLM_VOID *)spec;

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：模块获取帧级QP
* 参  数：*
*         handle                -IO        模块实例句柄
*         patch_type            -I         当前待编码patch类型，等同于帧类型
*         rc_regs               -O         码控信息
*         bitdepth              -I         比特位宽
* 返回值：状态码
* 备  注：
***************************************************************************************************/
HLM_STATUS HLMC_RC_Process(HLM_VOID                 *handle,
                           HLMC_PATCH_REF_TYPE       patch_type,
                           VENC_RATE_CTRL_OUT_REGS  *rc_regs,
                           HLM_U32                   bitdepth)
{
    HLMC_RC_SPEC *spec = (HLMC_RC_SPEC *)handle;

    HLM_CHECK_ERROR((HLM_NULL == handle) || (HLM_NULL == rc_regs), HLM_STS_ERR_NULL_PTR);
    HLM_CHECK_ERROR((patch_type > HLMC_VIRTUAL_I), HLM_STS_ERR_PRC_SIZE);

    spec->patchtype = patch_type;
    if (spec->rate_ctrl.rate_ctrl_mode == HLMC_RC_CBR)
    {
        rc_regs->reg_patch_qp = spec->rate_ctrl.rc_cbr_ctrl.init_qp;
        if (patch_type == HLMC_BASE_IDRPATCH)
        {
            rc_regs->reg_qpmin = spec->rate_ctrl.rc_cbr_ctrl.qp_min_i;
            rc_regs->reg_qpmax = spec->rate_ctrl.rc_cbr_ctrl.qp_max_i;
            rc_regs->enc_pic_target_bits = spec->m_targetRate_i;
        }
        else
        {
            rc_regs->reg_qpmin = spec->rate_ctrl.rc_cbr_ctrl.qp_min;
            rc_regs->reg_qpmax = spec->rate_ctrl.rc_cbr_ctrl.qp_max;
            rc_regs->enc_pic_target_bits = spec->m_targetRate_p;
        }
    }
    else  // HLMC_RC_FIXQP
    {
        if (patch_type == HLMC_BASE_IDRPATCH)
        {
            rc_regs->reg_patch_qp = spec->rate_ctrl.rc_fixqp_ctrl.qp_i;
        }
        else
        {
            rc_regs->reg_patch_qp = spec->rate_ctrl.rc_fixqp_ctrl.qp_p;
        }
        rc_regs->reg_qpmin = 4;
        rc_regs->reg_qpmax = HLM_MAX_QP(bitdepth);
    }

    return HLM_STS_OK;
}

/***************************************************************************************************
* 功  能：初始化QPG
* 参  数：*
*        rc_qpg                  -IO   QPG码控结构体
*        regs                    -I    寄存器参数结构体
*        cu_cols                 -I    当前帧cu列数
*        cu_rows                 -I    当前帧cu行数
* 返回值：void
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_RC_InitQpg(HLMC_QPG      *rc_qpg,
                         HLMC_REGS     *regs,
                         HLM_S32        cu_cols,
                         HLM_S32        cu_rows)
{
    HLM_U08 complex_level  = 0;
    HLM_U08 i              = ((regs->bitdepth - 8) >> 1);  // 8bit:0,  10bit:1,  12bit 2,  14bit 3,  16bit 4
    HLM_S32 rc_buffer_size = 1 << log2_rc_buffer_size[i];

    if (rc_qpg->rc_buffer_size_log2 != 0)
    {
        rc_buffer_size = 1 << (rc_qpg->rc_buffer_size_log2);
    }

    rc_qpg->cu_rows            = cu_rows;
    rc_qpg->cu_cols            = cu_cols;
    rc_qpg->pic_base_qp        = regs->enc_pic_qp;
    rc_qpg->pic_target_bits    = rc_qpg->patch_target_bits - (rc_buffer_size >> 1);
    rc_qpg->i_sum_row_qp       = 0;
    rc_qpg->p_sum_row_qp       = 0;
    rc_qpg->cur_cu_qp          = rc_qpg->pic_base_qp;
    rc_qpg->cur_cu_target_bits = 0;
    rc_qpg->total_bits_encoded = 0;
    rc_qpg->complex_level      = 0;
    rc_qpg->target_bits_cu[0]  = (HLM_S32)((HLM_F32)rc_qpg->pic_target_bits / (cu_cols * cu_rows));
    rc_qpg->target_bits_cu[1]  = rc_qpg->target_bits_cu[0];
    rc_qpg->target_bits_cu[2]  = rc_qpg->target_bits_cu[0];
    rc_qpg->target_bits_cu[3]  = rc_qpg->target_bits_cu[0];
    rc_qpg->target_bits_cu[4]  = rc_qpg->target_bits_cu[0];
    rc_qpg->total_diff_bits    = (rc_buffer_size >> 1) - (rc_qpg->pic_target_bits - rc_qpg->target_bits_cu[0] * (cu_cols * cu_rows));
    rc_qpg->cur_cu_target_bits = rc_qpg->target_bits_cu[0];
    rc_qpg->avg_B              = B_avg_init[i];

    for (complex_level = 0; complex_level < 5; complex_level++)
    {
        rc_qpg->B_lossless[complex_level] = (HLM_S32)B_lossless_init[i][complex_level];
    }
}

/***************************************************************************************************
* 功  能：更新码控参数QPG
* 参  数：*
*        rc_qpg                  -IO   QPG码控结构体
*        mix_flag                -I    混合复杂度标记
*        actual_bits             -I    实际的比特
*        luma_qp                 -I    亮度qp
*        chroma_qp               -I    色度qp
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_RC_UpdateQpg(HLMC_QPG      *rc_qpg,
                           HLM_U08        mix_flag,
                           HLM_S32        actual_bits,
                           HLM_S32        luma_qp,
                           HLM_S32        chroma_qp)
{
    HLM_S32 complex_level   = rc_qpg->complex_level;
    HLM_U08 luma_complex    = rc_qpg->complex_level_comp[0];
    HLM_U08 chroma_complex  = (rc_qpg->complex_level_comp[1] + rc_qpg->complex_level_comp[2] + 1) >> 1;
    HLM_U08 luma_qp_level   = (luma_qp - 4) / 6;
    HLM_U08 chroma_qp_level = (chroma_qp - 4) / 6;
    HLM_S32 real_B          = actual_bits + (((luma_qp - 4) + 2 * (chroma_qp - 4)) * HLM_CU_SIZE) / 6;

    if (rc_qpg->total_diff_bits < 0)    //水位太空或太满时，更新速率要加快
    {
        rc_qpg->avg_B = (HLM_S32)(rc_qpg->avg_B * 0.8 + real_B * 0.2);
    }
    else
    {
        rc_qpg->avg_B = (HLM_S32)(rc_qpg->avg_B * 0.99 + real_B * 0.01);
    }

    if (mix_flag == 0)
    {
        rc_qpg->B_lossless[complex_level] = (rc_qpg->B_lossless[complex_level] * 7 + real_B + 4) >> 3;
    }

    rc_qpg->total_bits_encoded += actual_bits;
    rc_qpg->total_diff_bits += (actual_bits - rc_qpg->cur_cu_target_bits);
}

/***************************************************************************************************
* 功  能：计算复杂度等级
* 参  数：*
*        rc_qpg                  -IO   QPG码控结构体
*        cur_best_satd           -I    最优的三分量satd
*        satd_comp_cur           -I    当前块各分量satd
*        bitdepth                -I    比特深度
*        yuv_comp                -I    分量个数
* 返回值：void
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_RC_CalComplexLevel(HLMC_QPG *rc_qpg,
                                 HLM_U32   cur_best_satd,
                                 HLM_U32   satd_comp_cur[3],
                                 HLM_U08   bitdepth,
#if  IBC_SCALE
                                 HLM_CU_TYPE   cu_type,
#endif
                                 HLM_U32   yuv_comp)
{
    HLM_U08 yuv_idx         = 0;
    HLM_U32 very_flat_th    = (HLM_U32)(HLM_CU_SIZE * (1 << (bitdepth - 8)) * RC_COMP_TH_VF);
    HLM_U32 flat_th         = (HLM_U32)(HLM_CU_SIZE * (1 << (bitdepth - 8)) * RC_COMP_TH_F);
    HLM_U32 complex_th      = (HLM_U32)(HLM_CU_SIZE * (1 << (bitdepth - 8)) * RC_COMP_TH_C);
    HLM_U32 very_complex_th = (HLM_U32)(HLM_CU_SIZE * (1 << (bitdepth - 8)) * RC_COMP_TH_VC);

#if  IBC_SCALE
    if (cu_type == HLM_IBC_4x4)
    {
        very_flat_th    >>= 2;
        flat_th         >>= 2;
        complex_th      >>= 2;
        very_complex_th >>= 2;
    }
#endif
    for (yuv_idx = HLM_LUMA_Y; yuv_idx < yuv_comp; yuv_idx++)
    {
        rc_qpg->complex_level_comp[yuv_idx] = HLMC_RC_get_complex(satd_comp_cur[yuv_idx],
            very_flat_th, flat_th, complex_th, very_complex_th);
    }
    rc_qpg->complex_level = HLMC_RC_get_complex(cur_best_satd,
        3 * very_flat_th, 3 * flat_th, 3 * complex_th, 3 * very_complex_th);
}

/***************************************************************************************************
* 功  能：计算混合复杂度块的混合等级
* 参  数：*
*        satd                 -I         亮度的8个4x4子块的satd
*        bitdepth             -I         比特深度
* 返回值：混合等级，0(非混合)、1(简单和复杂的混合)、2(非常简单和非常复杂的混合)
* 备  注：
***************************************************************************************************/
HLM_U08 HLMC_RC_CalMixFlag(HLM_U32  satd[8],
                           HLM_U08  bitdepth)
{
    HLM_U32 i      = 0;
    HLM_U32 thr_vf = (HLM_U32)(HLM_CU_SIZE * (1 << (bitdepth - 8)) * RC_COMP_TH_VF);
    HLM_U32 thr_f  = (HLM_U32)(HLM_CU_SIZE * (1 << (bitdepth - 8)) * RC_COMP_TH_F);
    HLM_U32 thr_c  = (HLM_U32)(HLM_CU_SIZE * (1 << (bitdepth - 8)) * RC_COMP_TH_C);
    HLM_U32 thr_vc = (HLM_U32)(HLM_CU_SIZE * (1 << (bitdepth - 8)) * RC_COMP_TH_VC);
    HLM_U32 satd2[10] = {
        satd[0] + satd[1],  // 第一行
        satd[1] + satd[2],
        satd[2] + satd[3],
        satd[4] + satd[5],  // 第二行
        satd[5] + satd[6],
        satd[6] + satd[7],
        satd[0] + satd[4],  // 四列
        satd[1] + satd[5],
        satd[2] + satd[6],
        satd[3] + satd[7],
    };
    HLM_U32 satd4[6] = {
        satd[0] + satd[1] + satd[2] + satd[3],  // 上
        satd[4] + satd[5] + satd[6] + satd[7],  // 下
        satd[0] + satd[1] + satd[4] + satd[5],  // 左
        satd[2] + satd[3] + satd[6] + satd[7],  // 右
        satd[1] + satd[2] + satd[5] + satd[6],  // 中间
        satd[0] + satd[3] + satd[4] + satd[7],  // 两边
    };

    HLM_U32 pu8_satd     = satd[0] + satd[1] + satd[2] + satd[3] + satd[4] + satd[5] + satd[6] + satd[7];
    HLM_U32 pu4_satd_min = HLM_MAX_U32;
    HLM_U32 pu2_satd_min = HLM_MAX_U32;
    HLM_U32 pu4_satd_max = 0;
    HLM_U32 pu2_satd_max = 0;
    HLM_S08 pu8_comp     = 0;
    HLM_S08 pu4_comp_max = 0;
    HLM_S08 pu4_comp_min = 0;
    HLM_S08 pu2_comp_max = 0;
    HLM_S08 pu2_comp_min = 0;
    HLM_U08 mix_flag_pu4 = 0;
    HLM_U08 mix_flag_pu2 = 0;

    for (i = 0; i < 6; i++)
    {
        pu4_satd_min = HLM_MIN(pu4_satd_min, satd4[i]);
        pu4_satd_max = HLM_MAX(pu4_satd_max, satd4[i]);
    }
    for (i = 0; i < 10; i++)
    {
        pu2_satd_min = HLM_MIN(pu2_satd_min, satd2[i]);
        pu2_satd_max = HLM_MAX(pu2_satd_max, satd2[i]);
    }

    pu8_comp     = HLMC_RC_get_complex(pu8_satd     << 0, thr_vf, thr_f, thr_c, thr_vc);
    pu4_comp_min = HLMC_RC_get_complex(pu4_satd_min << 1, thr_vf, thr_f, thr_c, thr_vc);
    pu4_comp_max = HLMC_RC_get_complex(pu4_satd_max << 1, thr_vf, thr_f, thr_c, thr_vc);
    pu2_comp_min = HLMC_RC_get_complex(pu2_satd_min << 2, thr_vf, thr_f, thr_c, thr_vc);
    pu2_comp_max = HLMC_RC_get_complex(pu2_satd_max << 2, thr_vf, thr_f, thr_c, thr_vc);

    if (pu8_comp <= 2)
    {
        return 0;
    }
    else  // 整体复杂
    {
        switch (pu4_comp_max - pu4_comp_min)
        {
        case 4:
            mix_flag_pu4 = 3;
            break;
        case 3:
            mix_flag_pu4 = 2;
            break;
        default:
            mix_flag_pu4 = 0;
            break;
        }

        switch (pu2_comp_max - pu2_comp_min)
        {
        case 4:
            mix_flag_pu2 = 2;
            break;
        case 3:
            mix_flag_pu2 = 1;
            break;
        default:
            mix_flag_pu2 = 0;
            break;
        }

        return HLM_MAX(mix_flag_pu4, mix_flag_pu2);
    }
}
