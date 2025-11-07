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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "hlmc_case_cmd.h"

// 从字符串中提取数字
HLM_S32 HLMC_CMD_extract_numbers(HLM_S08  *str,
                                 HLM_S32  *numbers)
{
    HLM_S08 *input_copy = HLM_NULL;
    HLM_S08 *start      = HLM_NULL;
    HLM_S08 *end        = HLM_NULL;
    HLM_S08 *content    = HLM_NULL;
    HLM_S08 *token      = HLM_NULL;
    HLM_S08 *ptr        = HLM_NULL;
    HLM_S32  count      = 0;
#ifdef _LINUX64
    input_copy = strdup(str);  // 创建可修改的副本
#else
    input_copy = _strdup(str);  // 创建可修改的副本
#endif

    if (!input_copy)
    {
        return count;
    }

    // 去除首尾的中括号
    start = strchr(input_copy, '[');
    end = strchr(input_copy, ']');
    if (!start || !end)
    {
        free(input_copy);
        return count;
    }
    *start = '\0';  // 临时截断字符串（保留左括号前的内容）
    content = start + 1;
    *end = '\0';    // 截断右括号后的内容

                    // 按逗号拆分
    token = strtok(content, ",");
    while (token && count < HLM_MAX_PATCH_NUM)
    {
        ptr = token;
        while (*ptr)
        {
            while (*ptr && isspace(*ptr)) ptr++;  // 跳过空格
            if (isdigit(*ptr))
            {
                numbers[count] = atoi(ptr);
                count++;
                break;  // 每个token只取一个数字
            }
            ptr++;
        }
        token = strtok(HLM_NULL, ",");
    }

    free(input_copy);
    return count;
}

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
                                HLMC_DPB_REF_CTRL    *dpb_ref_ctrl)
{
    HLM_S32 patch_x     [HLM_MAX_PATCH_NUM] = { 0 };
    HLM_S32 patch_y     [HLM_MAX_PATCH_NUM] = { 0 };
    HLM_S32 patch_width [HLM_MAX_PATCH_NUM] = { 0 };
    HLM_S32 patch_height[HLM_MAX_PATCH_NUM] = { 0 };
    HLM_S32 ele_num[4]                      = { 0 };
    HLM_S32 i                               = 0;

    if (cmd->file_input != HLM_NULL)
    {
        strcpy(parm->input_file, cmd->file_input);
    }
    if (cmd->file_output != HLM_NULL)
    {
        strcpy(parm->output_file, cmd->file_output);
    }
    if (cmd->file_rec != HLM_NULL)
    {
        strcpy(parm->rec_file, cmd->file_rec);
    }
    if (cmd->width != 0xffff)
    {
        coding_ctrl->width = cmd->width;
    }
    if (cmd->height != 0xffff)
    {
        coding_ctrl->height = cmd->height;
    }
    if (cmd->uniform_patch_split != 0xffff)
    {
        coding_ctrl->uniform_patch_split = cmd->uniform_patch_split;
    }
    if (cmd->patch_x != HLM_NULL)
    {
        strcpy(parm->patch_x, cmd->patch_x);
    }
    if (cmd->patch_y != HLM_NULL)
    {
        strcpy(parm->patch_y, cmd->patch_y);
    }
    if (cmd->patch_width != HLM_NULL)
    {
        strcpy(parm->patch_width, cmd->patch_width);
    }
    if (cmd->patch_height != HLM_NULL)
    {
        strcpy(parm->patch_height, cmd->patch_height);
    }

    // 拆分字符串
    ele_num[0] = HLMC_CMD_extract_numbers(parm->patch_x, patch_x);
    ele_num[1] = HLMC_CMD_extract_numbers(parm->patch_y, patch_y);
    ele_num[2] = HLMC_CMD_extract_numbers(parm->patch_width, patch_width);
    ele_num[3] = HLMC_CMD_extract_numbers(parm->patch_height, patch_height);
    if (coding_ctrl->uniform_patch_split)  // 均匀划分
    {
        assert(ele_num[2] > 0 && ele_num[3] > 0);  // 仅使用patch_width[0]和patch_height[0]
        coding_ctrl->patch_info.patch_num = 1;
        coding_ctrl->patch_info.patch_param[0].patch_width[0] = patch_width[0];
        coding_ctrl->patch_info.patch_param[0].patch_height[0] = patch_height[0];
    }
    else  // 非均匀划分
    {
        if (ele_num[0] == ele_num[1] && ele_num[1] == ele_num[2] && ele_num[2] == ele_num[3])
        {
            assert(0 < ele_num[0] && ele_num[0] < HLM_MAX_PATCH_NUM);
            coding_ctrl->patch_info.patch_num = ele_num[0];
            for (i = 0; i < coding_ctrl->patch_info.patch_num; i++)
            {
                coding_ctrl->patch_info.patch_param[i].patch_x = patch_x[i];
                coding_ctrl->patch_info.patch_param[i].patch_y = patch_y[i];
                coding_ctrl->patch_info.patch_param[i].patch_width[0] = patch_width[i];
                coding_ctrl->patch_info.patch_param[i].patch_height[0] = patch_height[i];
                coding_ctrl->patch_info.patch_param[i].patch_coded_width[0] = HLM_SIZE_ALIGN_16(patch_width[i]);
                coding_ctrl->patch_info.patch_param[i].patch_coded_height[0] = HLM_SIZE_ALIGN_8(patch_height[i]);
            }
            if (0 == HLM_COM_CheckPatchSplit(coding_ctrl->width, coding_ctrl->height, &coding_ctrl->patch_info))
            {
                printf("Patch划分信息无法拼成完整图像!\n");
                assert(0);
            }
        }
        else
        {
            printf("输入的Patch_X/Y/W/H个数不一致，分别是%d,%d,%d,%d!\n", ele_num[0], ele_num[1], ele_num[2], ele_num[3]);
            assert(0);
        }
    }

    if (cmd->i_frame_enable_ibc != 0xffff)
    {
        coding_ctrl->i_frame_enable_ibc = cmd->i_frame_enable_ibc;
    }
    if (cmd->p_frame_enable_ibc != 0xffff)
    {
        coding_ctrl->p_frame_enable_ibc = cmd->p_frame_enable_ibc;
    }
    if (cmd->mv_ref_cross_patch != 0xffff)
    {
        coding_ctrl->mv_ref_cross_patch = cmd->mv_ref_cross_patch;
    }
    if (cmd->mv_search_width != 0xffff)
    {
        coding_ctrl->mv_search_width = cmd->mv_search_width;
    }
    if (cmd->mv_search_height != 0xffff)
    {
        coding_ctrl->mv_search_height = cmd->mv_search_height;
    }
    if (cmd->bitdepth != 0xffff)
    {
        coding_ctrl->bitdepth = cmd->bitdepth;
    }
    if (cmd->img_format != 0xffff)
    {
        coding_ctrl->img_format = cmd->img_format;
    }
    if (cmd->intra_8x8_enable_flag != 0xffff)
    {
        coding_ctrl->intra_8x8_enable_flag = cmd->intra_8x8_enable_flag;
    }
    if (cmd->frame_num != 0xffff)
    {
        parm->total_frames = cmd->frame_num;
    }
    if (cmd->frame_rate_num != 0xffff)
    {
        coding_ctrl->frame_rate_num = cmd->frame_rate_num;
    }
    if (cmd->frame_rate_den != 0xffff)
    {
        coding_ctrl->frame_rate_denom = cmd->frame_rate_den;
    }
    if (cmd->intra_period != 0xffff)
    {
        dpb_ref_ctrl->intra_period = cmd->intra_period;
    }
    if (cmd->initqp != 0xffff)
    {
        rate_ctrl->rc_cbr_ctrl.init_qp = cmd->initqp;
    }
    if (cmd->rc_mode != 0xffff)
    {
        rate_ctrl->rate_ctrl_mode = (HLMC_RATECTRL_TYPE)cmd->rc_mode;
    }
    if (cmd->maxqp != 0xffff)
    {
        rate_ctrl->rc_cbr_ctrl.qp_max = cmd->maxqp;
    }
    if (cmd->minqp != 0xffff)
    {
        rate_ctrl->rc_cbr_ctrl.qp_min = cmd->minqp;
    }
    if (cmd->imaxqp != 0xffff)
    {
        rate_ctrl->rc_cbr_ctrl.qp_max_i = cmd->imaxqp;
    }
    if (cmd->iminqp != 0xffff)
    {
        rate_ctrl->rc_cbr_ctrl.qp_min_i = cmd->iminqp;
    }
    if (cmd->bpp_i != 0xffff)
    {
        rate_ctrl->rc_cbr_ctrl.bpp_i = cmd->bpp_i;
    }
    if (cmd->bpp_p != 0xffff)
    {
        rate_ctrl->rc_cbr_ctrl.bpp_p = cmd->bpp_p;
    }
    if (cmd->rc_buffer_size_log2 != 0xffff)
    {
        rate_ctrl->rc_cbr_ctrl.rc_buffer_size_log2 = cmd->rc_buffer_size_log2;
    }
    if (cmd->rec_on != 0xffff)
    {
        parm->rec_flag = cmd->rec_on;
    }
    if (cmd->qp_i != 0xffff)
    {
        rate_ctrl->rc_fixqp_ctrl.qp_i = cmd->qp_i;
    }
    if (cmd->qp_p != 0xffff)
    {
        rate_ctrl->rc_fixqp_ctrl.qp_p = cmd->qp_p;
    }
    if (coding_ctrl->mv_ref_cross_patch == 1)
    {
        for (i = 0; i < coding_ctrl->patch_info.patch_num; i++)
        {
            assert(coding_ctrl->patch_info.patch_param[i].patch_width[0] % 16 == 0);
            assert(coding_ctrl->patch_info.patch_param[i].patch_height[0] % 8 == 0);
        }
    }

#if WRITE_PARAMETERS
    strcpy(parm->param_file, parm->rec_file);
    strcat(parm->param_file, "_param.txt");
#endif

    return;
}

// 打印help信息
HLM_VOID HLMC_CMD_help()
{
    fprintf(stdout, "Usage: TestHLME -c [cfg_file] -l [log_file] -x -x ...\n");
    fprintf(stdout, "'cfg_file' and 'log_file' is necessary\n");
    fprintf(stdout, "-i                            input file\n");
    fprintf(stdout, "-o                            output file\n");
    fprintf(stdout, "-recon                        write recon file or not\n");
    fprintf(stdout, "-rec                          recon file\n");
    fprintf(stdout, "-fn                           num of frames to be encoded\n");
    fprintf(stdout, "-wdt                          source picture width\n");
    fprintf(stdout, "-hgt                          source picture height\n");
    fprintf(stdout, "-fr                           frame rate\n");
    fprintf(stdout, "-I                            intra period\n");
    fprintf(stdout, "-rc                           mode of rate control\n");
    fprintf(stdout, "                              0 = cbr\n");
    fprintf(stdout, "                              1 = vbr\n");
    fprintf(stdout, "                              2 = fix_qp\n");
    fprintf(stdout, "-rccu                         cu level qp adjust , 0 : on , 1 : off\n");
    fprintf(stdout, "-iminqp                       qpmin of I frame\n");
    fprintf(stdout, "-imaxqp                       qpmin of I frame\n");
    fprintf(stdout, "-pminqp                       qpmax of P frame\n");
    fprintf(stdout, "-pmaxqp                       qpmax of P frame\n");
    fprintf(stdout, "-iqp                          qp of I frmae when fix_qp\n");
    fprintf(stdout, "-pqp                          qp of P frmae when fix_qp\n");
    fprintf(stdout, "-b                            target bit rate，bit/s\n");
    fprintf(stdout, "-initqp                       init qp\n");
    fprintf(stdout, "-s                            statetime of rc\n");
    fprintf(stdout, "-help                         This help.\n");

    return;
}

/***************************************************************************************************
* 功  能：设置命令行默认参数
* 参  数：*
*        cmd                      -I       命令行数据
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_CMD_SetDefaultCommand(HLMC_DEMO_CMD *cmd)
{
    memset(cmd, 0, sizeof(HLMC_DEMO_CMD));

    cmd->file_cfg                 = "../../../data/config/encode_AI.cfg";
    cmd->file_log                 = "../../../data/output/enc_log.txt";
    cmd->file_input               = HLM_NULL;
    cmd->file_output              = HLM_NULL;
    cmd->file_rec                 = HLM_NULL;
    cmd->width                    = 0xffff;
    cmd->height                   = 0xffff;
    cmd->uniform_patch_split      = 0xffff;
    cmd->patch_x                  = HLM_NULL;
    cmd->patch_y                  = HLM_NULL;
    cmd->patch_width              = HLM_NULL;
    cmd->patch_height             = HLM_NULL;
    cmd->i_frame_enable_ibc       = 0xffff;
    cmd->p_frame_enable_ibc       = 0xffff;
    cmd->mv_ref_cross_patch       = 0xffff;
    cmd->mv_search_width          = 0xffff;
    cmd->mv_search_height         = 0xffff;
    cmd->frame_num                = 0xffff;
    cmd->frame_rate_num           = 0xffff;
    cmd->frame_rate_den           = 0xffff;
    cmd->intra_period             = 0xffff;
    cmd->initqp                   = 0xffff;
    cmd->rc_mode                  = 0xffff;
    cmd->bpp_i                    = 0xffff;
    cmd->bpp_p                    = 0xffff;
    cmd->rc_buffer_size_log2      = 0xffff;
    cmd->minqp                    = 0xffff;
    cmd->maxqp                    = 0xffff;
    cmd->iminqp                   = 0xffff;
    cmd->imaxqp                   = 0xffff;
    cmd->qp_i                     = 0xffff;
    cmd->qp_p                     = 0xffff;
    cmd->rec_on                   = 0xffff;
    cmd->bitdepth                 = 0xffff;
    cmd->img_format               = 0xffff;
    cmd->intra_8x8_enable_flag    = 0xffff;

    return;
}

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
                              HLMC_DEMO_CMD   *cmd)
{
    HLM_S32 i = 0;

    // 遍历command line
    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-c"))
        {
            cmd->file_cfg = argv[++i];
        }
        else if (!strcmp(argv[i], "-l"))
        {
            cmd->file_log = argv[++i];
        }
        else if (!strcmp(argv[i], "-i"))
        {
            cmd->file_input = argv[++i];
        }
        else if (!strcmp(argv[i], "-o"))
        {
            cmd->file_output = argv[++i];
        }
        else if (!strcmp(argv[i], "-recon"))
        {
            cmd->rec_on = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-rec"))
        {
            cmd->file_rec = argv[++i];
        }
        else if (!strcmp(argv[i], "-hgt"))
        {
            cmd->height = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-wdt"))
        {
            cmd->width = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-uniformpatch"))
        {
            cmd->uniform_patch_split = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-patchx"))
        {
            cmd->patch_x = argv[++i];
        }
        else if (!strcmp(argv[i], "-patchy"))
        {
            cmd->patch_y = argv[++i];
        }
        else if (!strcmp(argv[i], "-patchwidth"))
        {
            cmd->patch_width = argv[++i];
        }
        else if (!strcmp(argv[i], "-patchheight"))
        {
            cmd->patch_height = argv[++i];
        }
        else if (!strcmp(argv[i], "-iframeibc"))
        {
            cmd->i_frame_enable_ibc = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-pframeibc"))
        {
            cmd->p_frame_enable_ibc = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-mvrefcrosspatch"))
        {
            cmd->mv_ref_cross_patch = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-mvsearchwidth"))
        {
            cmd->mv_search_width = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-mvsearchheight"))
        {
            cmd->mv_search_height = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-bitdepth"))
        {
            cmd->bitdepth = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-format"))
        {
            cmd->img_format = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-intra8x8"))
        {
            cmd->intra_8x8_enable_flag = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-fn"))
        {
            cmd->frame_num = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-fr"))
        {
            cmd->frame_rate_num = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-fd"))
        {
            cmd->frame_rate_den = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-I"))
        {
            cmd->intra_period = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-rc"))
        {
            cmd->rc_mode = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-bppi"))
        {
            cmd->bpp_i = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-bppp"))
        {
            cmd->bpp_p = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-rcbuffersizelog2"))
        {
            cmd->rc_buffer_size_log2 = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-iminqp"))
        {
            cmd->iminqp = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-imaxqp"))
        {
            cmd->imaxqp = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-pminqp"))
        {
            cmd->minqp = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-pmaxqp"))
        {
            cmd->maxqp = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-iqp"))
        {
            cmd->qp_i = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-pqp"))
        {
            cmd->qp_p = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-initqp"))
        {
            cmd->initqp = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-help"))
        {
            fprintf(stderr, "-------------------------------------\n");
            HLMC_CMD_help();
            fprintf(stderr, "-------------------------------------\n");
        }
        else
        {
            fprintf(stderr, "unsupported command line  %s\n", argv[i]);
            fprintf(stderr, "-------------------------------------\n");
            HLMC_CMD_help();
            fprintf(stderr, "-------------------------------------\n");
            return HLM_STS_ERR; 
        }
    }

    return 0;
}

/***************************************************************************************************
* 功  能：校验命令行参数
* 参  数：*
*        cmd                      -I       命令行数据
* 返回值：0：成功；-1：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CMD_CheckCommand(HLMC_DEMO_CMD *cmd)
{
    FILE *fp_tmp = HLM_NULL;

    if ((HLM_NULL == cmd->file_cfg) || (HLM_NULL == cmd->file_log))
    {
        fprintf(stderr, "Both cfg file name and log file name need to be initialized.\n");
        return -1;
    }

    fp_tmp = fopen(cmd->file_cfg, "rb");
    if (HLM_NULL == fp_tmp)
    {
        fprintf(stderr, "Unable to open cfg file %s.\n", cmd->file_cfg);
        return -1;
    }
    cmd->fp_cfg = fp_tmp;

    fp_tmp = fopen(cmd->file_log, "w");
    if (HLM_NULL == fp_tmp)
    {
        fprintf(stderr, "Unable to open log file %s.\n", cmd->file_log);
        return -1;
    }
    cmd->fp_log = fp_tmp;

    return 0;
}
