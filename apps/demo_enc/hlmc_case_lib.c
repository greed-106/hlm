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
#include "hlmc_case_lib.h"
#include "hlmc_case_cmd.h"
#include <math.h>
#include <stdlib.h>
#if defined(_MSC_VER)
#include <windows.h>
#elif defined(__GNUC__)
#include <sys/time.h>
#endif

HLMC_CASE_CFG_PARAM  case_cfg_param = { 0 };      // 用户可配置的测试用例参数
HLMC_ABILITY         ability        = { 0 };      // 能力集参数
HLMC_CODING_CTRL     coding_ctrl    = { 0 };      // 序列级编码控制参数
HLMC_RATE_CTRL       rate_ctrl      = { 0 };      // 码率控制参数
HLMC_DPB_REF_CTRL    dpb_ref_ctrl   = { 0 };      // 参考配置参数
HLMC_PROCESS_IN      prc_in         = { 0 };      // 输入信息结构体
HLMC_PROCESS_OUT     prc_out        = { 0 };      // 输出信息结构体

static HLMC_CFG_TOKEN_MAPPING token_map[HLMC_CFG_MAX_ITEMS] = 
{
    {(signed char *)"InputFile",              &case_cfg_param.input_file,                  0},
    {(signed char *)"OutputFile",             &case_cfg_param.output_file,                 0},
    {(signed char *)"RecFile",                &case_cfg_param.rec_file,                    0},
    {(signed char *)"FrameNumber",            &case_cfg_param.total_frames,                sizeof(case_cfg_param.total_frames)},
    {(signed char *)"RecFlag",                &case_cfg_param.rec_flag,                    sizeof(case_cfg_param.rec_flag)},

    // 能力集参数
    {(signed char *)"MaxWidth",               &ability.max_width,                          sizeof(ability.max_width)},
    {(signed char *)"MaxHeight",              &ability.max_height,                         sizeof(ability.max_height)},

    // 序列级编码控制参数
    {(signed char *)"SrcWidth",               &coding_ctrl.width,                          sizeof(coding_ctrl.width)},
    {(signed char *)"SrcHeight",              &coding_ctrl.height,                         sizeof(coding_ctrl.height)},
    {(signed char *)"UniformPatchSplit",      &coding_ctrl.uniform_patch_split,            sizeof(coding_ctrl.uniform_patch_split)},
    {(signed char *)"PatchX",                 &case_cfg_param.patch_x,                     0},
    {(signed char *)"PatchY",                 &case_cfg_param.patch_y,                     0},
    {(signed char *)"PatchWidth",             &case_cfg_param.patch_width,                 0},
    {(signed char *)"PatchHeight",            &case_cfg_param.patch_height,                0},
    {(signed char *)"IFrameIBC",              &coding_ctrl.i_frame_enable_ibc,             sizeof(coding_ctrl.i_frame_enable_ibc)},
    {(signed char *)"PFrameIBC",              &coding_ctrl.p_frame_enable_ibc,             sizeof(coding_ctrl.p_frame_enable_ibc)},
    {(signed char *)"SubIBC",                 &coding_ctrl.sub_ibc_enable_flag,            sizeof(coding_ctrl.sub_ibc_enable_flag)},
    {(signed char *)"MvRefCrossPatch",        &coding_ctrl.mv_ref_cross_patch,             sizeof(coding_ctrl.mv_ref_cross_patch)},
    {(signed char *)"MVSearchWidth",          &coding_ctrl.mv_search_width,                sizeof(coding_ctrl.mv_search_width)},
    {(signed char *)"MVSearchHeight",         &coding_ctrl.mv_search_height,               sizeof(coding_ctrl.mv_search_height)},
    {(signed char *)"InputBitdepth",          &coding_ctrl.bitdepth,                       sizeof(coding_ctrl.bitdepth)},
    {(signed char *)"ImageFormat",            &coding_ctrl.img_format,                     sizeof(coding_ctrl.img_format)},
    {(signed char *)"Intra8x8Enable",         &coding_ctrl.intra_8x8_enable_flag,          sizeof(coding_ctrl.intra_8x8_enable_flag)},
    {(signed char *)"Intra16x1And2x8",        &coding_ctrl.intra_16x1_2x8_enable_flag,     sizeof(coding_ctrl.intra_16x1_2x8_enable_flag)},
#if INTRA_CHROMA_MODE_SEPARATE
    {(signed char *)"IntraChromaPmEnable",    &coding_ctrl.intra_chroma_mode_enable_flag,     sizeof(coding_ctrl.intra_chroma_mode_enable_flag) },
    {(signed char *)"IntraSubChromaPmEnable", &coding_ctrl.intra_sub_chroma_mode_enable_flag, sizeof(coding_ctrl.intra_sub_chroma_mode_enable_flag) },
#endif
    {(signed char *)"FrameRateNum",           &coding_ctrl.frame_rate_num,                 sizeof(coding_ctrl.frame_rate_num)},
    {(signed char *)"FrameRateDenom",         &coding_ctrl.frame_rate_denom,               sizeof(coding_ctrl.frame_rate_denom)},
    {(signed char *)"Profile",                &coding_ctrl.profile,                        sizeof(coding_ctrl.profile)},
#if FIX_CFG_ENC
    {(signed char *)"ChromaQpOffset",         &coding_ctrl.chroma_qp_offset,               sizeof(coding_ctrl.chroma_qp_offset)},
    {(signed char *)"SegmentEnable",          &coding_ctrl.segment_enable_flag,            sizeof(coding_ctrl.segment_enable_flag)},
    {(signed char *)"SegmentWidthLog",        &coding_ctrl.segment_width_in_log2,          sizeof(coding_ctrl.segment_width_in_log2)},
    {(signed char *)"SegmentHeightLog",       &coding_ctrl.segment_height_in_log2,         sizeof(coding_ctrl.segment_height_in_log2)},
    {(signed char *)"PatchExtraParamPresent", &coding_ctrl.patch_extra_params_present_flag,sizeof(coding_ctrl.patch_extra_params_present_flag)},
#endif

    // 码控的参数解析
    {(signed char *)"RateCtrlMode",           &rate_ctrl.rate_ctrl_mode,                   sizeof(rate_ctrl.rate_ctrl_mode)},
    {(signed char *)"IFrameBpp",              &rate_ctrl.rc_cbr_ctrl.bpp_i,                sizeof(rate_ctrl.rc_cbr_ctrl.bpp_i)},
    {(signed char *)"PFrameBpp",              &rate_ctrl.rc_cbr_ctrl.bpp_p,                sizeof(rate_ctrl.rc_cbr_ctrl.bpp_p)},
    {(signed char *)"InitQp",                 &rate_ctrl.rc_cbr_ctrl.init_qp,              sizeof(rate_ctrl.rc_cbr_ctrl.init_qp)},
    {(signed char *)"RcBufferSizeLog2",       &rate_ctrl.rc_cbr_ctrl.rc_buffer_size_log2,  sizeof(rate_ctrl.rc_cbr_ctrl.rc_buffer_size_log2)},

    // FIXQP的参数解析
    {(signed char *)"IFrameQp",               &rate_ctrl.rc_fixqp_ctrl.qp_i,               sizeof(rate_ctrl.rc_fixqp_ctrl.qp_i)},
    {(signed char *)"PFrameQp",               &rate_ctrl.rc_fixqp_ctrl.qp_p,               sizeof(rate_ctrl.rc_fixqp_ctrl.qp_p)},

    // 参考配置参数
    {(signed char *)"IntraPeriod",            &dpb_ref_ctrl.intra_period,                  sizeof(dpb_ref_ctrl.intra_period)},

    {(signed char *)"\0",                     HLM_NULL,                                    0} // end
};

// 从测试用例配置文件中读取内容到缓存buf中
HLM_S08 *HLMC_CFG_get_cfg_content(FILE *fp_cfg,
                                  FILE *fp_log)
{
    HLM_S32  file_len = 0;
    HLM_S08 *buf_cfg  = HLM_NULL;

    if (0 != fseek(fp_cfg, 0, SEEK_END))
    {
        fprintf(stderr, "Unable to seek end of config file, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "Unable to seek end of config file, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        return HLM_NULL;
    }
    file_len = ftell(fp_cfg);
    if (file_len < 0)
    {
        fprintf(stderr, "Config File Size %d Error, in Func <%s>, Line <%d>\n", file_len, __FUNCTION__, __LINE__);
        fprintf(fp_log, "Config File Size %d Error, in Func <%s>, Line <%d>\n", file_len, __FUNCTION__, __LINE__);
        return HLM_NULL;
    }
    if (0 != fseek(fp_cfg, 0, SEEK_SET))
    {
        fprintf(stderr, "Unable to seek start of config file, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "Unable to seek start of config file, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        return HLM_NULL;
    }
    buf_cfg = (HLM_S08 *)malloc(file_len + 1024);
    if (HLM_NULL == buf_cfg)
    {
        fprintf(stderr, "No Enough memory for Config Buffer, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "No Enough memory for Config Buffer, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        return HLM_NULL;
    }
    if (file_len != fread(buf_cfg, 1, file_len, fp_cfg))
    {
        fprintf(stderr, "Failed to read cfg file, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "Failed to read cfg file, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        free(buf_cfg);
        return HLM_NULL;
    }

    buf_cfg[file_len] = '\0';  // put end of string marker

    return buf_cfg;
}

// 从测试用例配置文件中读取内容到缓存buf中
HLM_S32 HLMC_CFG_param_parse(HLM_S08 *s)
{
    HLM_S32 i = 0;

    while (token_map[i].token_name != HLM_NULL)
    {
        if (0 == strcmp((const char*)token_map[i].token_name, (const char *)s))
        {
            return i;
        }
        else
        {
            i++;
        }
    }

    return -1;
}

// 解析测试用例配置文件内容
HLM_S32 HLMC_CFG_parse_content(HLM_S08 *cfg_buf,
                               HLM_SZT  cfg_len,
                               FILE    *fp_log)
{
    /*--------------------------------------------------------------------------*/
    /* Total number of parameter initialisation lines in the file this          */
    /* excludes comment lines and blank lines if any                            */
    /*--------------------------------------------------------------------------*/
    HLM_S08 *items[HLMC_CFG_MAX_ITEMS] = {0};
    HLM_S32  item                       = 0;

    /*--------------------------------------------------------------------------*/
    /* Index into the token map array - into which the corresponding            */
    /* parameter values needs to be initialised from the param file             */
    /*--------------------------------------------------------------------------*/
    HLM_S32 MapIdx = 0;

    /*--------------------------------------------------------------------------*/
    /* Flags which indicate the current state of the state machine              */
    /* InString - indicates that buffer pointer is currently in between         */
    /* a valid string, which inturn can contain multiple items.                 */
    /* InItem - indicates that buffer pointer is within a valid item            */
    /*--------------------------------------------------------------------------*/
    HLM_S32 InString = 0;
    HLM_S32 InItem   = 0;

    /*--------------------------------------------------------------------------*/
    /* Dynamic pointer movign allong the param file buffer byte-by-byte         */
    /*--------------------------------------------------------------------------*/
    HLM_S08 *p = cfg_buf;

    /*--------------------------------------------------------------------------*/   
    /* end of buffer - used for terminating the parse loop                      */
    /*--------------------------------------------------------------------------*/   
    HLM_S08 *bufend = &cfg_buf[cfg_len];

    /*--------------------------------------------------------------------------*/   
    /* Parameter value read from the file and loop counter                      */
    /*--------------------------------------------------------------------------*/   
    HLM_S32 IntContent = 0;
    HLM_S32 i          = 0;
    /*--------------------------------------------------------------------------*/
    /*                                STAGE ONE                                 */
    /* Generate an argc/argv-type list in items[], without comments and         */
    /* whitespace. This is context insensitive and could be done most easily    */
    /* with lex(1).                                                             */
    /*--------------------------------------------------------------------------*/
    while (p < bufend)
    {
        switch (*p)
        {
            /*----------------------------------------------------------------------*/
            /* blank space - skip the character and go to next                      */
            /*----------------------------------------------------------------------*/
        case 13:
            p++;
            break;
            /*----------------------------------------------------------------------*/
            /* Found a comment line skip all characters until end-of-line is found  */
            /*----------------------------------------------------------------------*/
        case '#': 
            /*--------------------------------------------------------------------*/
            /* Replace '#' with '\0' in case of comment immediately following     */
            /* integer or string Skip till EOL or EOF, whichever comes first      */
            /*--------------------------------------------------------------------*/
            *p = '\0';
            while (*p != '\n' && p < bufend)
            {
                p++;
            }
            InString = 0;
            InItem = 0;
            break;
            /*----------------------------------------------------------------------*/
            /* end of line - skip the character and go to next,reset the InItem flag*/
            /* to indicate that we are not in any valid item                        */
            /*----------------------------------------------------------------------*/
        case '\n':
            InItem = 0;
            InString = 0;
            *p++='\0';
            break;
            /*----------------------------------------------------------------------*/
            /* Whitespaces and tabs indicate end of an item                         */
            /*----------------------------------------------------------------------*/
        case ' ':
        case '\t':
            /*--------------------------------------------------------------------*/
            /* if the state machine is within a string Skip whitespace,           */
            /* leave state unchanged                                              */
            /* else Terminate non-strings once whitespace is found                */
            /*--------------------------------------------------------------------*/
            if (InString)
            {
                p++;
            }
            else
            {
                *p++ = '\0';
                InItem = 0;
            }
            break;
            /*----------------------------------------------------------------------*/
            /* begining or end of string - toggle the string indication flag        */
            /*----------------------------------------------------------------------*/
        case '"': 
            *p++ = '\0';
            if (!InString)
            {
                items[item++] = p;
                InItem = ~InItem;
            }
            else
            {
                InItem = 0;
            }
            InString = ~InString;
            break;
            /*----------------------------------------------------------------------*/
            /* Any othe character is taken into the item provided the state machine */
            /* is within a valid Item                                               */
            /*----------------------------------------------------------------------*/
        default:
            if (!InItem)
            {
                items[item++] = p;
                InItem = ~InItem;
            }
            p++;
        }
    }
    item--;
    /*--------------------------------------------------------------------------*/
    /* for all the items found - check if they correspond to any valid parameter*/
    /* names specified by the user through token map array,                     */
    /* Note: Contigous three items are grouped into triplets. Thsi oredered     */
    /* triplet is used to identify the valid parameter entry in the Token map   */
    /* array.                                                                   */
    /* First item in the triplet has to eb a parameter name.                    */
    /* Second item has to be a "=" symbol                                       */
    /* Third item has to be a integer value.                                    */
    /* Any violation of the above order in the triplets found would lead        */
    /* to error condition.                                                      */ 
    /*--------------------------------------------------------------------------*/
    for (i=0; i<item; i+= 3)
    {
        /*------------------------------------------------------------------------*/
        /* Get the map index into the token map array - corresponding to the      */
        /* first item - in a item-triplet                                         */
        /*------------------------------------------------------------------------*/
        if (0 <= (MapIdx = HLMC_CFG_param_parse(items[i])))
        {
            /*----------------------------------------------------------------------*/
            /* Assert if the second item is indeed "=" symbol                       */
            /*----------------------------------------------------------------------*/
            if (strcmp ((const char *)"=", (const char *)items[i+1]))
            {
                fprintf(stderr, "'=' expected after Parameter Name '%s', in Func <%s>, Line <%d>\n", items[i], __FUNCTION__, __LINE__);
                fprintf(fp_log, "'=' expected after Parameter Name '%s', in Func <%s>, Line <%d>\n", items[i], __FUNCTION__, __LINE__);
                return -1;
            }
            /*----------------------------------------------------------------------*/
            /* depending upon the type of content convert them and populate the     */
            /* corresponding entry in the token map array with this value           */
            /*----------------------------------------------------------------------*/
            if(token_map[MapIdx].type == 0)
            {
                strcpy((char *)token_map[MapIdx].place, (char *)items[i+2]);
            }
            else if (token_map[MapIdx].type == 4)
            {
                sscanf((const char *)items[i+2], "%d", (&IntContent));
                *((int*)(token_map[MapIdx].place)) = IntContent;
            }
            else if (token_map[MapIdx].type == 2)
            {
                sscanf((const char *)items[i+2], "%d", &IntContent);
                *((short*)(token_map[MapIdx].place)) = IntContent;
            }
            else
            {
                sscanf ((const char *)items[i+2], "%d", &IntContent);
                *((signed char*)(token_map[MapIdx].place)) = IntContent;
            }
        }
        else
        {
            fprintf(stderr, "Parameter Name '%s' not recognized, in Func <%s>, Line <%d>\n", items[i], __FUNCTION__, __LINE__);
            fprintf(fp_log, "Parameter Name '%s' not recognized, in Func <%s>, Line <%d>\n", items[i], __FUNCTION__, __LINE__);
        }
    }

    return 0;
}

/***************************************************************************************************
* 功  能：初始化配置文件内容
* 参  数：*
*        case_cfg_param           -OI       文件参数
*        coding_ctrl              -OI       编码控制参数
*        rate_ctrl                -OI       码控参数
*        dpb_ref_ctrl             -OI       dpb参数
* 返回值：无
* 备  注：
***************************************************************************************************/
HLM_VOID HLMC_CASE_InitCfgFile(HLMC_CASE_CFG_PARAM  *case_cfg_param,
                               HLMC_CODING_CTRL     *coding_ctrl,
                               HLMC_RATE_CTRL       *rate_ctrl,
                               HLMC_DPB_REF_CTRL    *dpb_ref_ctrl)
{

    case_cfg_param->total_frames                    = 10;
    case_cfg_param->rec_flag                        = 0;
    case_cfg_param->patch_x[0]                      = '0';
    case_cfg_param->patch_y[0]                      = '0';
    strcpy(case_cfg_param->patch_width,  "1920");
    strcpy(case_cfg_param->patch_height, "1080");

    coding_ctrl->width                              = 1920;
    coding_ctrl->height                             = 1080;
    coding_ctrl->uniform_patch_split                = 1;
    coding_ctrl->i_frame_enable_ibc                 = 1;
    coding_ctrl->p_frame_enable_ibc                 = 0;
    coding_ctrl->sub_ibc_enable_flag                = 1;
    coding_ctrl->mv_ref_cross_patch                 = 0;
    coding_ctrl->mv_search_width                    = 16;
    coding_ctrl->mv_search_height                   = 16;
    coding_ctrl->bitdepth                           = 8;
    coding_ctrl->img_format                         = 1;
    coding_ctrl->intra_8x8_enable_flag              = 0;
    coding_ctrl->intra_16x1_2x8_enable_flag         = 1;
    coding_ctrl->frame_rate_num                     = 30;
    coding_ctrl->frame_rate_denom                   = 1;
    coding_ctrl->profile                            = 64;
#if INTRA_CHROMA_MODE_SEPARATE
    coding_ctrl->intra_chroma_mode_enable_flag      = 1;
    coding_ctrl->intra_sub_chroma_mode_enable_flag  = 1;
#endif
#if FIX_CFG_ENC
    coding_ctrl->chroma_qp_offset                   = 0;
    coding_ctrl->segment_enable_flag                = 0;
    coding_ctrl->segment_width_in_log2              = 6;  // 16*4 = 64, 2^6 = 64
    coding_ctrl->segment_height_in_log2             = 5;  // 8*4  = 32, 2^5 = 32
    coding_ctrl->patch_extra_params_present_flag    = 0;
#endif

    rate_ctrl->rate_ctrl_mode                       = 0;
    rate_ctrl->rc_cbr_ctrl.bpp_i                    = 38;
    rate_ctrl->rc_cbr_ctrl.bpp_p                    = 38;
    rate_ctrl->rc_cbr_ctrl.init_qp                  = 27;
    rate_ctrl->rc_cbr_ctrl.rc_buffer_size_log2      = 0;
    rate_ctrl->rc_fixqp_ctrl.qp_i                   = 22;
    rate_ctrl->rc_fixqp_ctrl.qp_p                   = 22;

    dpb_ref_ctrl->intra_period                      = 1;
}

/***************************************************************************************************
* 功  能：读取测试用例配置文件内容
* 参  数：* 
*         fp_cfg               -I         测试用例配置文件指针
*         fp_log               -O         运行日志文件指针
* 返回值：0：成功；-1：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_ReadCfgFile(FILE *fp_cfg, FILE *fp_log)
{
    HLM_S32  ret     = 0;
    HLM_S08 *cfg_buf = HLM_NULL;
    HLM_SZT  cfg_len = 0;

    // 获取配置文件内容
    cfg_buf = HLMC_CFG_get_cfg_content(fp_cfg, fp_log);
    if (HLM_NULL == cfg_buf)
    {
        fprintf(stderr, "Unable to get config content, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "Unable to get config content, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        ret = -1;
        goto CFG_EXIT;
    }

    cfg_len = strlen((const char*)cfg_buf);
    ret = (HLM_S32)HLMC_CFG_parse_content(cfg_buf, cfg_len, fp_log);

CFG_EXIT:

    if (cfg_buf != HLM_NULL)
    {
        free(cfg_buf);
        cfg_buf = HLM_NULL;
    }

    return ret;
}

/***************************************************************************************************
* 功  能：创建算法模型句柄
* 参  数：* 
*         mem_tab              -I         内存指针，由外界统一管理
*         handle               -O         算法模型句柄
*         fp_log               -O         log文件指针
* 返回值：0：成功，其他：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_CreateHandle(HLM_MEM_TAB     mem_tab[HLM_MEM_TAB_NUM],
                               HLM_VOID      **handle,
                               FILE           *fp_log)
{
    HLM_S32     i          = 0;
    HLM_STATUS  sts        = HLM_STS_ERR;
    HLM_VOID   *lib_handle = HLM_NULL;

    ability.max_width  = HLM_IMG_WIDTH_MAX;
    ability.max_height = HLM_IMG_HEIGHT_MAX;

    // 获取所需资源数量
    sts = HLMC_LIB_GetMemSize(&ability, mem_tab, &coding_ctrl);
    if (sts != HLM_STS_OK)
    {
        fprintf(stderr, "[LIB] get mem size failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        fprintf(fp_log, "[LIB] get mem size failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        return -1;
    }

#ifdef _DEBUG
    // 打印内存大小
#if defined(_M_X64)
    for (i = 0; i < HLM_MEM_TAB_NUM; i++)
    {
        fprintf(stdout, "[LIB] memTab %d memory size = %lld\n", i, mem_tab[i].size);
        fprintf(fp_log, "[LIB] memTab %d memory size = %lld\n", i, mem_tab[i].size);
    }
#else
    for (i = 0; i < HLM_MEM_TAB_NUM; i++)
    {
        fprintf(stdout, "[LIB] memTab %d memory size = %d\n", i, mem_tab[i].size);
        fprintf(fp_log, "[LIB] memTab %d memory size = %d\n", i, mem_tab[i].size);
    }
#endif
#endif

    // 分配算法模型所需资源
    sts = HLM_MEM_AllocMemTab(mem_tab, HLM_MEM_TAB_NUM);
    if (sts != HLM_STS_OK)
    {
        fprintf(stderr, "[LIB] alloc lib mem failed in Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "[LIB] alloc lib mem failed in Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    // 创建算法模型实例
    sts = HLMC_LIB_Create(&ability, mem_tab, &lib_handle, &coding_ctrl);
    if (sts != HLM_STS_OK)
    {
        fprintf(stderr, "[LIB] create lib failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        fprintf(fp_log, "[LIB] create lib failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        return -1;
    }

    // 设置序列级编码控制参数
#if !FIX_CFG_ENC
    coding_ctrl.chroma_qp_offset       = 0;
    coding_ctrl.segment_enable_flag    = 0;
    coding_ctrl.segment_width_in_log2  = 6;  // 16*4 = 64, 2^6 = 64
    coding_ctrl.segment_height_in_log2 = 5;  // 8*4  = 32, 2^5 = 32
    if (coding_ctrl.segment_width_in_log2 < 4 || coding_ctrl.segment_height_in_log2 < 3)
    {
        fprintf(stderr, "[LIB] segment_width_in_log2 less than 4 or segment_height_in_log2 less than 3: in Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif
    sts = HLMC_LIB_SetCodingCtrl(lib_handle, &coding_ctrl);
    if (sts != HLM_STS_OK)
    {
        fprintf(stderr, "[LIB] Set Coding Ctrl failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        fprintf(fp_log, "[LIB] Set Coding Ctrl failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        return -1;
    }

    // 设置码率控制参数
    if (rate_ctrl.rate_ctrl_mode != HLMC_RC_CBR && rate_ctrl.rate_ctrl_mode != HLMC_RC_FIXQP)
    {
        fprintf(stderr, "[LIB] Illegal Rate Ctrl Mode %d in Func <%s>, Line <%d>.\n", rate_ctrl.rate_ctrl_mode, __FUNCTION__, __LINE__);
        fprintf(fp_log, "[LIB] Illegal Rate Ctrl Mode %d in Func <%s>, Line <%d>.\n", rate_ctrl.rate_ctrl_mode, __FUNCTION__, __LINE__);
        return -1;
    }
#if !FIX_3
    HLMC_LIB_InitRc(lib_handle, coding_ctrl.width, coding_ctrl.height, &rate_ctrl);
#else
    sts = HLMC_LIB_SetRateCtrl(lib_handle, coding_ctrl.width, coding_ctrl.height, &rate_ctrl);
    if (sts != HLM_STS_OK)
    {
        fprintf(stderr, "[LIB] Set Rate Ctrl failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        fprintf(fp_log, "[LIB] Set Rate Ctrl failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        return -1;
    }
#endif

    // 设置序列级编码控制参数
    sts = HLMC_LIB_SetDpbRefCtrl(lib_handle, &dpb_ref_ctrl);
    if (sts != HLM_STS_OK)
    {
        fprintf(stderr, "[LIB] Set DPB Ref Ctrl failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        fprintf(fp_log, "[LIB] Set DPB Ref Ctrl failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        return -1;
    }

    *handle = lib_handle;

    return 0;
}

/***************************************************************************************************
* 功  能：销毁算法库句柄
* 参  数：*
*         mem_tab              -I         内存指针，由外界统一管理
*         handle               -O         算法模型句柄
*         fp_log               -O         log文件指针
* 返回值：0：成功，其他：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_DestroyHandle(HLM_MEM_TAB     mem_tab[HLM_MEM_TAB_NUM],
                                HLM_VOID       *handle,
                                FILE           *fp_log)
{
    HLM_STATUS sts = HLM_STS_ERR;

    sts = HLM_MEM_FreeMemTab(mem_tab, HLM_MEM_TAB_NUM);
    if (HLM_STS_OK != sts)
    {
        fprintf(stderr, "[DEMO] free lib mem failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        fprintf(fp_log, "[DEMO] free lib mem failed: %d, in Func <%s>, Line <%d>.\n", sts, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

// 读入原始图像数据
HLM_S32 HLMC_CASE_read_picture(HLM_IMAGE *image,
                               HLM_S32   *ori_width,
                               HLM_S32   *ori_height,
                               HLM_U08    bitdepth,
                               FILE      *fp)
{
    HLM_U16 *pic_in_y   = (HLM_U16 *)image->data[0];
    HLM_U16 *pic_in_u   = (HLM_U16 *)image->data[1];
    HLM_U16 *pic_in_v   = (HLM_U16 *)image->data[2];
    HLM_U16 pixel_value = 0;
    HLM_S32 i           = 0;
    HLM_S32 j           = 0;
    HLM_S32 byte_size   = bitdepth == 8 ? 1 : 2;

    for (i = 0; i < ori_height[0]; i++)
    {
        for (j = 0; j < ori_width[0]; j++)
        {
            if (1 != fread(&pixel_value, byte_size, 1, fp))
            {
                return HLM_STS_ERR;
            }
            else
            {
                pic_in_y[j] = HLM_CLIP(pixel_value, 0, (1 << bitdepth) - 1);
            }
        }
        if (ori_width[0] < image->step[0])
        {
            for (j = 0; j < image->step[0] - ori_width[0]; j++)
            {
                pic_in_y[ori_width[0] + j] = *(pic_in_y + ori_width[0] - 1);
            }
        }
        pic_in_y += image->step[0];
    }
    if (ori_height[0] < image->height[0])
    {
        for (i = 0; i < image->height[0] - ori_height[0]; i++)
        {
            memcpy(pic_in_y, pic_in_y - image->step[0], image->step[0] * sizeof(HLM_U16));
            pic_in_y += image->step[0];
        }
    }

    for (i = 0; i < ori_height[1]; i++)
    {
        for (j = 0; j < ori_width[1]; j++)
        {
            if (1 != (HLM_U32)fread(&pixel_value, byte_size, 1, fp))
            {
                return HLM_STS_ERR;
            }
            else
            {
                pic_in_u[j] = HLM_CLIP(pixel_value, 0, (1 << bitdepth) - 1);
            }
        }
        if (ori_width[1] < image->step[1])
        {
            for (j = 0; j < image->step[1] - ori_width[1]; j++)
            {
                pic_in_u[ori_width[1] + j] = *(pic_in_u + ori_width[1] - 1);
            }
        }
        pic_in_u += image->step[1];
    }
    if (ori_height[1] < image->height[1])
    {
        for (i = 0; i < image->height[1] - ori_height[1]; i++)
        {
            memcpy(pic_in_u, pic_in_u - image->step[1], image->step[1] * sizeof(HLM_U16));
            pic_in_u += image->step[1];
        }
    }

    for (i = 0; i < ori_height[1]; i++)
    {
        for (j = 0; j < ori_width[1]; j++)
        {
            if (1 != (HLM_U32)fread(&pixel_value, byte_size, 1, fp))
            {
                return HLM_STS_ERR;
            }
            else
            {
                pic_in_v[j] = HLM_CLIP(pixel_value, 0, (1 << bitdepth) - 1);
            }
        }
        if (ori_width[1] < image->step[2])
        {
            for (j = 0; j < image->step[2] - ori_width[1]; j++)
            {
                pic_in_v[ori_width[1] + j] = *(pic_in_v + ori_width[1] - 1);
            }
        }
        pic_in_v += image->step[2];
    }
    if (ori_height[1] < image->height[1])
    {
        for (i = 0; i < image->height[1] - ori_height[1]; i++)
        {
            memcpy(pic_in_v, pic_in_v - image->step[2], image->step[2] * sizeof(HLM_U16));
            pic_in_v += image->step[2];
        }
    }

    return HLM_STS_OK;
}

// 计算mse
HLM_VOID HIMC_CASE_compute_mse(HLM_U16        *src_y,
                               HLM_U16        *src_u,
                               HLM_U16        *src_v,
                               HLM_U16        *rec_y,
                               HLM_U16        *rec_u,
                               HLM_U16        *rec_v,
                               HLM_S32         format,
                               HLM_S32         stride[2],
                               HLM_S32         real_width[2],
                               HLM_S32         real_height[2],
                               HLM_MSE_INFO   *mse_info)
{
    HLM_U16 *src_ptr[3] = { src_y, src_u, src_v };
    HLM_U16 *rec_ptr[3] = { rec_y, rec_u, rec_v };
    HLM_S32 num_pixels  = 0;
    HLM_U16 offset_w[2] = { 0 };
    HLM_U16 offset_h[2] = { 0 };
    HLM_S32 blk_w       = 0;
    HLM_S32 blk_h       = 0;
    HLM_F64 blk_mse     = 0;
    HLM_S32 yuv_idx     = 0;
    HLM_S32 cu_pel_x    = 0;
    HLM_S32 cu_pel_y    = 0;
    HLM_S32 x           = 0;
    HLM_S32 y           = 0;
    HLM_S32 p1          = 0;
    HLM_S32 p2          = 0;
    HLM_F64 err         = 0;
    HLM_S32 yuv_comp    = format == HLM_IMG_YUV_400 ? 1 : 3;

    offset_w[0] = 4;
    offset_h[0] = 3;
    switch (format)
    {
    case HLM_IMG_YUV_444:
    case HLM_IMG_RGB:
        offset_w[1] = 4;
        offset_h[1] = 3;
        break;
    case HLM_IMG_YUV_420:
        offset_w[1] = 3;
        offset_h[1] = 2;
        break;
    case HLM_IMG_YUV_422:
        offset_w[1] = 3;
        offset_h[1] = 3;
        break;
    }

    for (yuv_idx = 0; yuv_idx < yuv_comp; yuv_idx++)
    {
        mse_info->frame_mse[yuv_idx] = 0;
        mse_info->max_block_mse[yuv_idx] = 0;
        num_pixels = real_width[yuv_idx != 0] * real_height[yuv_idx != 0];
        for (cu_pel_y = 0; cu_pel_y < real_height[yuv_idx != 0]; cu_pel_y += (1 << offset_h[yuv_idx != 0]))
        {
            for (cu_pel_x = 0; cu_pel_x < real_width[yuv_idx != 0]; cu_pel_x += (1 << offset_w[yuv_idx != 0]))
            {
                blk_w = (cu_pel_x + (1 << offset_w[yuv_idx != 0]) <= real_width[yuv_idx != 0])
                      ? (1 << offset_w[yuv_idx != 0]) : (real_width[yuv_idx != 0] - cu_pel_x);
                blk_h = (cu_pel_y + (1 << offset_h[yuv_idx != 0]) <= real_height[yuv_idx != 0])
                      ? (1 << offset_h[yuv_idx != 0]) : (real_height[yuv_idx != 0] - cu_pel_y);
                blk_mse = 0;
                for (y = 0; y < blk_h; y++)
                {
                    for (x = 0; x < blk_w; x++)
                    {
                        p1 = src_ptr[yuv_idx][(cu_pel_y + y) * stride[yuv_idx != 0] + (cu_pel_x + x)];
                        p2 = rec_ptr[yuv_idx][(cu_pel_y + y) * stride[yuv_idx != 0] + (cu_pel_x + x)];
                        err = HLM_ABS(p1 - p2);
                        blk_mse += err * err;
                    }
                }
                mse_info->frame_mse[yuv_idx] += blk_mse;
                blk_mse /= (blk_w * blk_h);
                if (blk_mse > mse_info->max_block_mse[yuv_idx])
                {
                    mse_info->max_block_mse[yuv_idx] = blk_mse;
                    mse_info->bmse_cu_x[yuv_idx] = cu_pel_x >> offset_w[yuv_idx != 0];
                    mse_info->bmse_cu_y[yuv_idx] = cu_pel_y >> offset_h[yuv_idx != 0];
                }
            }
        }
        mse_info->frame_mse[yuv_idx] /= num_pixels;
    }
}

// 计算ssim
HLM_F64 HLMC_CASE_cal_ssim(HLM_U16 *org_buf,
                           HLM_U16 *ref_buf,
                           HLM_S32  src_stride,
                           HLM_S32  rec_stride,
                           HLM_U08  bitdepth,
                           HLM_S32  width,
                           HLM_S32  height)
{
    HLM_F64 K1                = 0.01f;
    HLM_F64 K2                = 0.03f;
    HLM_S32 win_width         = 8;
    HLM_S32 win_height        = 8;
    HLM_F64 max_pix_value_sqd = 0;
    HLM_F64 C1                = 0;
    HLM_F64 C2                = 0;
    HLM_F64 win_pixels        = (HLM_F64)(win_width * win_height);
    HLM_F64 win_pixels_bias   = win_pixels;
    HLM_F64 cu_ssim           = 0;
    HLM_F64 meanOrg           = 0;
    HLM_F64 meanEnc           = 0;
    HLM_F64 var_org           = 0;
    HLM_F64 var_enc           = 0;
    HLM_F64 cov_org_enc       = 0;
    HLM_S32 imean_org         = 0;
    HLM_S32 imean_enc         = 0;
    HLM_S32 ivar_org          = 0;
    HLM_S32 ivar_enc          = 0;
    HLM_S32 icov_org_enc      = 0;
    HLM_F64 cur_distortion    = 0.0;
    HLM_S32 i                 = 0;
    HLM_S32 j                 = 0;
    HLM_S32 n                 = 0;
    HLM_S32 m                 = 0;
    HLM_S32 win_cnt           = 0;
    HLM_S32 overlap_size      = 8;
    HLM_S32 src_idx           = 0;
    HLM_S32 rec_idx           = 0;
    HLM_U32 max_val           = (1 << bitdepth) - 1;

#if FIX_ENC
    if (width < win_width || height < win_height)
    {
        return 0;
    }
#endif

    max_pix_value_sqd = (HLM_F64)(max_val * max_val);
    C1 = K1 * K1 * max_pix_value_sqd;
    C2 = K2 * K2 * max_pix_value_sqd;

    for (j = 0; j <= height - win_height; j += overlap_size)
    {
        for (i = 0; i <= width - win_width; i += overlap_size)
        {
            imean_org    = 0;
            imean_enc    = 0;
            ivar_org     = 0;
            ivar_enc     = 0;
            icov_org_enc = 0;
            for (n = j; n < (j + win_height); n++)
            {
                for (m = i; m < (i + win_width); m++)
                {
                    src_idx       = n * src_stride + m;
                    rec_idx       = n * rec_stride + m;
                    imean_org    += org_buf[src_idx];
                    imean_enc    += ref_buf[rec_idx];
                    ivar_org     += org_buf[src_idx] * org_buf[src_idx];
                    ivar_enc     += ref_buf[rec_idx] * ref_buf[rec_idx];
                    icov_org_enc += org_buf[src_idx] * ref_buf[rec_idx];
                }
            }

            meanOrg         = (HLM_F64)imean_org / win_pixels;
            meanEnc         = (HLM_F64)imean_enc / win_pixels;
            var_org         = ((HLM_F64)ivar_org - ((HLM_F64)imean_org) * meanOrg) / win_pixels_bias;
            var_enc         = ((HLM_F64)ivar_enc - ((HLM_F64)imean_enc) * meanEnc) / win_pixels_bias;
            cov_org_enc     = ((HLM_F64)icov_org_enc - ((HLM_F64)imean_org) * meanEnc) / win_pixels_bias;
            cu_ssim         = (HLM_F64)((2.0 * meanOrg * meanEnc + C1) * (2.0 * cov_org_enc + C2));
            cu_ssim        /= (HLM_F64)(meanOrg * meanOrg + meanEnc * meanEnc + C1) * (var_org + var_enc + C2);
            cur_distortion += cu_ssim;
            win_cnt++;
        }
    }

    if (win_cnt == 0)
    {
        return HLM_MAXABS_F64;
    }

    cur_distortion /= (HLM_F64)win_cnt;

    if ((cur_distortion >= 1.0) && (cur_distortion < 1.01)) // avoid float accuracy problem at very low QP(e.g.2)
    {
        cur_distortion = 1.0;
    }

    return cur_distortion;
}

/***************************************************************************************************
* 功  能：获取编码算法模型的版本和时间
* 参  数：无
* 返回值：编码算法模型版本和时间
* 备  注：版本信息格式为：主版本号（6位）＋子版本号（5位）＋修正版本号（5位）
***************************************************************************************************/
HLM_U32 HLMC_CASE_GetVersion()
{
    HLM_U32 version      = 0;
    HLM_U32 date         = 0;
    HLM_U32 version_date = 0;

    version = (HLM_MAJOR_VERSION << 10) |
        (HLM_MINOR_VERSION << 5) |
        (HLM_REVISION_VERSION);
    version_date = ((version << 16) | date);

    return version_date;
}

#if FIX_LINUX
/***************************************************************************************************
* 功  能：统计时间
* 参  数：*

* 返回值：时钟
* 备  注：单位us
***************************************************************************************************/
HLM_S64 HLMC_TIME_get_usec()
{
#ifdef _MSC_VER
    LARGE_INTEGER m_nFreq;
    LARGE_INTEGER m_nTime;

    QueryPerformanceFrequency(&m_nFreq);
    QueryPerformanceCounter(&m_nTime);

    return m_nTime.QuadPart * 1000 * 1000 / m_nFreq.QuadPart;

#elif defined(__GNUC__)
    struct timeval tv_date;

    gettimeofday(&tv_date, HLM_NULL);
    return (HLM_S64)tv_date.tv_sec * 1000 * 1000 + (HLM_S64)tv_date.tv_usec;
#endif
}
#endif

/***************************************************************************************************
* 功  能：处理一个测试用例
* 参  数：*
*         handle               -I         算法模型句柄
*         fp_log               -O         log文件指针
* 返回值：0：成功，其他：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_ProcessSequence(HLM_VOID *handle,
                                  FILE     *fp_log)
{
    HLM_U32    i              = 0;
    HLM_STATUS sts            = HLM_STS_ERR;
    HLM_S32    fn             = 0;
    HLM_U32    code_width[2]  = { 0 };
    HLM_U32    code_height[2] = { 0 };
    HLM_U32    real_width[2]  = { 0 };
    HLM_U32    real_height[2] = { 0 };
    HLM_S32    frame_size     = 0;
    HLM_S32    stream_buf_len = 0;
    HLM_S32    in_size        = 0;
    HLM_S32    out_size       = 0;
    HLM_VOID  *in_buf         = HLM_NULL;
    HLM_VOID  *out_buf        = HLM_NULL;
    HLM_VOID  *src_y          = HLM_NULL;
    HLM_VOID  *src_u          = HLM_NULL;
    HLM_VOID  *src_v          = HLM_NULL;
    HLM_U08   *stream_buf     = HLM_NULL;
    HLM_U08   *out_stream_buf = HLM_NULL;
    HLM_U16   *recon_dbk_y    = HLM_NULL;
    HLM_U16   *recon_dbk_u    = HLM_NULL;
    HLM_U16   *recon_dbk_v    = HLM_NULL;
    FILE      *fp_input       = HLM_NULL;
    FILE      *fp_output      = HLM_NULL;
    FILE      *fp_rec         = HLM_NULL;
#if WRITE_PARAMETERS
    FILE      *fp_param       = HLM_NULL;
#endif
#if FIX_LINUX
    HLM_S64    start_time     = 0;
    HLM_S64    end_time       = 0;
#else
#if defined(_MSC_VER)
    LARGE_INTEGER  freq       = { 0 };
    LARGE_INTEGER  start_time = { 0 };
    LARGE_INTEGER  end_time   = { 0 };
#endif
#endif
    HLM_F64    ssim_y         = 0.0;
    HLM_F64    ssim_u         = 0.0;
    HLM_F64    ssim_v         = 0.0;
    HLM_F64    psnr_y         = 0;
    HLM_F64    psnr_u         = 0;
    HLM_F64    psnr_v         = 0;
    HLM_F64    refValueY      = 0;
    HLM_F64    refValueC      = 0;
    HLM_U64    bit_size       = 0;
    HLM_F64    once_time      = 0.0;
    HLM_F64    avg_time       = 0.0;
    HLM_F64    cur_ssim_y     = 0.0;
    HLM_F64    cur_ssim_u     = 0.0;
    HLM_F64    cur_ssim_v     = 0.0;
    HLM_F64    cur_psnr_y     = 0.0;
    HLM_F64    cur_psnr_u     = 0.0;
    HLM_F64    cur_psnr_v     = 0.0;
    HLM_S32    cnt_p          = 0;  // P帧个数
    HLM_U32    j              = 0;
    HLM_U08    bitdepth       = coding_ctrl.bitdepth;
    HLM_U08    byte_size      = bitdepth == 8 ? 1 : 2;
    HLM_VOID  *bak_src_y      = HLM_NULL;
    HLM_VOID  *bak_src_u      = HLM_NULL;
    HLM_VOID  *bak_src_v      = HLM_NULL;
    HLM_MSE_INFO  mse_info    = { 0 };
    HLM_F64    mse_y          = 0;
    HLM_F64    mse_u          = 0;
    HLM_F64    mse_v          = 0;
    HLM_F64    block_mse_y    = 0;
    HLM_F64    block_mse_u    = 0;
    HLM_F64    block_mse_v    = 0;

    code_width[0]  = HLM_SIZE_ALIGN_16(coding_ctrl.width);
    code_height[0] = HLM_SIZE_ALIGN_8(coding_ctrl.height);
    real_width[0]  = (coding_ctrl.width);
    real_height[0] = (coding_ctrl.height);
    switch (coding_ctrl.img_format)
    {
    case HLM_IMG_YUV_444:
    case HLM_IMG_RGB:
        code_width[1]  = code_width[0];
        code_height[1] = code_height[0];
        real_width[1]  = real_width[0];
        real_height[1] = real_height[0];
        break;
    case HLM_IMG_YUV_422:
        code_width[1]  = (code_width[0] >> 1);
        code_height[1] = code_height[0];
        real_width[1]  = (real_width[0] >> 1);
        real_height[1] = real_height[0];
        break;
    case HLM_IMG_YUV_420:
        code_width[1]  = (code_width[0] >> 1);
        code_height[1] = (code_height[0] >> 1);
        real_width[1]  = (real_width[0] >> 1);
        real_height[1] = (real_height[0] >> 1);
        break;
    case HLM_IMG_YUV_400:
        code_width[1]  = 0;
        code_height[1] = 0;
        real_width[1]  = 0;
        real_height[1] = 0;
        break;
    default:
        printf("wrong image format!");
        break;
    }
    frame_size = (code_width[0] * code_height[0] + code_width[1] * code_height[1] * 2);

#if FIX_LINUX == 0
#if defined(_MSC_VER)
    QueryPerformanceFrequency(&freq);
#endif
#endif
    fp_input = fopen(case_cfg_param.input_file, "rb");
    if (HLM_NULL == fp_input)
    {
        fprintf(stderr, "[PROC] Open input file failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "[PROC] Open input file failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        goto SEQUENCE_EXIT;
    }

    fp_output = fopen(case_cfg_param.output_file, "wb");
    if (HLM_NULL == fp_output)
    {
        fprintf(stderr, "[PROC] Open output file failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "[PROC] Open output file failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        goto SEQUENCE_EXIT;
    }

#if WRITE_PARAMETERS
    fp_param = fopen(case_cfg_param.param_file, "w");
    if (HLM_NULL == fp_param)
    {
        fprintf(stderr, "[PROC] Open param file failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "[PROC] Open param file failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        goto SEQUENCE_EXIT;
    }
#endif

    // 分配原始图像缓存
    src_y = malloc(frame_size * sizeof(HLM_U16));
    if (HLM_NULL == src_y)
    {
        fprintf(stderr, "[PROC] Malloc Source YUV buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "[PROC] Malloc Source YUV buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        goto SEQUENCE_EXIT;
    }
    src_u = (HLM_VOID *)((HLM_U16 *)src_y + code_width[0] * code_height[0]);
    src_v = (HLM_VOID *)((HLM_U16 *)src_u + code_width[1] * code_height[1]);

    // 分配原始图像缓存
    bak_src_y = malloc(frame_size * sizeof(HLM_U16));
    if (HLM_NULL == bak_src_y)
    {
        fprintf(stderr, "[PROC] Malloc Source YUV buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "[PROC] Malloc Source YUV buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        goto SEQUENCE_EXIT;
    }
    bak_src_u = (HLM_VOID *)((HLM_U16 *)bak_src_y + code_width[0] * code_height[0]);
    bak_src_v = (HLM_VOID *)((HLM_U16 *)bak_src_u + code_width[1] * code_height[1]);

    // 分配码流缓冲区大小，位宽按16比特算
#if FIX_ENC
    stream_buf_len = frame_size * sizeof(HLM_U16) + 4096;  // 加上缓冲区大小4096字节
#else
    stream_buf_len = frame_size * sizeof(HLM_U16);
#endif

    stream_buf = (HLM_U08 *)malloc(stream_buf_len);
    out_stream_buf = (HLM_U08 *)malloc(stream_buf_len);
    if (HLM_NULL == stream_buf)
    {
        fprintf(stderr, "[PROC] Malloc stream buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "[PROC] Malloc stream buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        goto SEQUENCE_EXIT;
    }
    if (HLM_NULL == out_stream_buf)
    {
        fprintf(stderr, "[PROC] Malloc stream buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "[PROC] Malloc stream buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        goto SEQUENCE_EXIT;
    }

    if (case_cfg_param.rec_flag)
    {
        fp_rec = fopen(case_cfg_param.rec_file, "wb");
        if (HLM_NULL == fp_rec)
        {
            fprintf(stderr, "[PROC] Open rec_dbk file failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
            fprintf(fp_log, "[PROC] Open rec_dbk file failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
            goto SEQUENCE_EXIT;
        }
        recon_dbk_y = (HLM_U16 *)malloc(code_width[0] * code_height[0] * sizeof(HLM_U16));
        recon_dbk_u = (HLM_U16 *)malloc(code_width[1] * code_height[1] * sizeof(HLM_U16));
        recon_dbk_v = (HLM_U16 *)malloc(code_width[1] * code_height[1] * sizeof(HLM_U16));

        if ((HLM_NULL == recon_dbk_y) || (HLM_NULL == recon_dbk_u) || (HLM_NULL == recon_dbk_v))
        {
            fprintf(stderr, "[PROC] Malloc recon_dbk buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
            fprintf(fp_log, "[PROC] Malloc recon_dbk buffer failed in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
            goto SEQUENCE_EXIT;
        }
    }

    // prc_in、prc_out为全局变量
    prc_in.image_in.format    = (HLM_IMAGE_FORMAT)coding_ctrl.img_format;
    prc_in.image_in.width[0]  = code_width[0];
    prc_in.image_in.height[0] = code_height[0];
    prc_in.image_in.width[1]  = code_width[1];
    prc_in.image_in.height[1] = code_height[1];
    prc_in.image_in.step[0]   = code_width[0];
    prc_in.image_in.step[1]   = code_width[1];
    prc_in.image_in.step[2]   = code_width[1];
    prc_in.image_in.bitdepth  = bitdepth;
    prc_in.image_in.data[0]   = src_y;
    prc_in.image_in.data[1]   = src_u;
    prc_in.image_in.data[2]   = src_v;
    prc_in.stream_buf         = stream_buf;
    prc_in.out_stream_buf     = out_stream_buf;
    prc_in.stream_buf_size    = stream_buf_len;
    prc_in.force_idr          = 0;
#if WRITE_PARAMETERS
    prc_in.fp_param           = fp_param;
#endif
    prc_out.recon_dbk_y = recon_dbk_y;
    prc_out.recon_dbk_u = recon_dbk_u;
    prc_out.recon_dbk_v = recon_dbk_v;

    // 设置算法库的输入输出
    in_buf    = (void *)&prc_in;
    in_size   = sizeof(HLMC_PROCESS_IN);
    out_buf   = (void *)&prc_out;
    out_size  = sizeof(HLMC_PROCESS_OUT);
    refValueY = (double)((1 << bitdepth) - 1) * ((1 << bitdepth) - 1);
    refValueC = (double)((1 << bitdepth) - 1) * ((1 << bitdepth) - 1);

    if (case_cfg_param.total_frames <= 0)
    {
        fprintf(stderr, "[PROC] Error: encode frame error, Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "[PROC] Error: encode frame error, Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
        goto SEQUENCE_EXIT;
    }

    fprintf(stdout, "%-6s%16s%16s%16s%16s%16s(%-5s,%-5s)%16s(%-5s,%-5s)%16s(%-5s,%-5s)%16s%16s%16s%16s%16s%16s%16s\n",
        "POC", "Bitrate(kbps)", "Ymse", "Umse", "Vmse",
        "Ybmse", "CUx", "CUy", "Ubmse", "CUx", "CUy", "Vbmse", "CUx", "CUy",
        "Ypsnr", "Upsnr", "Vpsnr", "Yssim", "Ussim", "Vssim", "EncT(s)");
    fprintf(stdout, "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    for (fn = 0; fn < case_cfg_param.total_frames; fn++)
    {
        if (HLM_STS_OK != HLMC_CASE_read_picture(&prc_in.image_in, real_width, real_height, bitdepth, fp_input))
        {
            fprintf(stderr, "[PROC] Error: no more input data to be read, Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
            fprintf(fp_log, "[PROC] Error: no more input data to be read, Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
            break;
        }

        // 备份原始输入
        memcpy(bak_src_y, src_y, frame_size * sizeof(HLM_U16));
        if (prc_in.image_in.format == HLM_IMG_RGB)
        {
            // rgb 转 yuv444
            HLM_COM_Rgb_To_YCbCr((HLM_U16 *)src_y, (HLM_U16 *)src_u, (HLM_U16 *)src_v, code_width[0], code_height[0], bitdepth);
        }

#if FIX_LINUX
        start_time = HLMC_TIME_get_usec();
#else
#if defined(_MSC_VER)
        QueryPerformanceCounter(&start_time);
#endif
#endif

        // 进行一帧处理
        sts = HLMC_LIB_EncodeFrame(handle, in_buf, in_size, out_buf, out_size);

#if FIX_LINUX
        end_time = HLMC_TIME_get_usec();
#else
#if defined(_MSC_VER)
        QueryPerformanceCounter(&end_time);
#endif
#endif

        if (sts != HLM_STS_OK)
        {
            fprintf(stderr, "[PROC] Encode frame %d failed %d, Func <%s>, Line <%d>\n", fn, sts, __FUNCTION__, __LINE__);
            fprintf(fp_log, "[PROC] Encode frame %d failed %d, Func <%s>, Line <%d>\n", fn, sts, __FUNCTION__, __LINE__);
            goto SEQUENCE_EXIT;
        }

        if (fwrite(out_stream_buf, prc_out.stream_len + prc_out.sei_len, 1, fp_output) != 1)
        {
            fprintf(stderr, "[PROC] write stream file failed, Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
            fprintf(fp_log, "[PROC] write stream file failed, Func <%s>, Line <%d>.\n", __FUNCTION__, __LINE__);
            sts = HLM_STS_ERR;
            goto SEQUENCE_EXIT;
        }

        bit_size += prc_out.stream_len;
#if FIX_LINUX
        once_time = (HLM_F64)(end_time - start_time);
        if (dpb_ref_ctrl.intra_period == 1)  // AI配置
        {
            avg_time += once_time;
        }
        else  // LDP配置，仅统计P帧的时间
        {
            if (fn % dpb_ref_ctrl.intra_period != 0)
            {
                cnt_p++;
                avg_time += once_time;
            }
        }
#else
#if defined(_MSC_VER)
        once_time = (end_time.QuadPart - start_time.QuadPart) * 1000.0 * 1000.0 / freq.QuadPart;
        if (dpb_ref_ctrl.intra_period == 1)  // AI配置
        {
            avg_time += once_time;
        }
        else  // LDP配置，仅统计P帧的时间
        {
            if (fn % dpb_ref_ctrl.intra_period != 0)
            {
                cnt_p++;
                avg_time += once_time;
            }
        }
#endif
#endif

        // yuv444 转 rgb
        if (prc_in.image_in.format == HLM_IMG_RGB)
        {
            HLM_COM_YCbCr_To_Rgb(prc_out.recon_dbk_y, prc_out.recon_dbk_u, prc_out.recon_dbk_v,
                code_width[0], code_height[0], bitdepth);
        }

        // 计算指标
        HIMC_CASE_compute_mse((HLM_U16 *)bak_src_y, (HLM_U16 *)bak_src_u, (HLM_U16 *)bak_src_v,
            prc_out.recon_dbk_y, prc_out.recon_dbk_u, prc_out.recon_dbk_v,
            prc_in.image_in.format, code_width, real_width, real_height, &mse_info);

        cur_psnr_y  = 10.0 * log10(refValueY / (double)mse_info.frame_mse[0]);
        cur_ssim_y  = HLMC_CASE_cal_ssim((HLM_U16 *)bak_src_y, prc_out.recon_dbk_y,
            code_width[0], code_width[0], bitdepth, real_width[0], real_height[0]);
        psnr_y     += cur_psnr_y;
        ssim_y     += cur_ssim_y;
        mse_y      += mse_info.frame_mse[0];
        block_mse_y = HLM_MAX(block_mse_y, mse_info.max_block_mse[0]);
        if ((coding_ctrl.img_format != HLM_IMG_YUV_400))
        {
            cur_psnr_u  = 10.0 * log10(refValueC / (double)mse_info.frame_mse[1]);
            cur_psnr_v  = 10.0 * log10(refValueC / (double)mse_info.frame_mse[2]);
            cur_ssim_u  = HLMC_CASE_cal_ssim((HLM_U16 *)bak_src_u, prc_out.recon_dbk_u,
                code_width[1], code_width[1], bitdepth, real_width[1], real_height[1]);
            cur_ssim_v  = HLMC_CASE_cal_ssim((HLM_U16 *)bak_src_v, prc_out.recon_dbk_v,
                code_width[1], code_width[1], bitdepth, real_width[1], real_height[1]);
            psnr_u     += cur_psnr_u;
            psnr_v     += cur_psnr_v;
            ssim_u     += cur_ssim_u;
            ssim_v     += cur_ssim_v;
            mse_u      += mse_info.frame_mse[1];
            mse_v      += mse_info.frame_mse[2];
            block_mse_u = HLM_MAX(block_mse_u, mse_info.max_block_mse[1]);
            block_mse_v = HLM_MAX(block_mse_v, mse_info.max_block_mse[2]);
        }

        fprintf(stdout, "%-6d%16.4f%16.4f%16.4f%16.4f%16.2f(%-5d,%-5d)%16.2f(%-5d,%-5d)%16.2f(%-5d,%-5d)%16.4f%16.4f%16.4f%16.4f%16.4f%16.4f%16.4f\n", fn,
            (double)((prc_out.stream_len << 3) * coding_ctrl.frame_rate_num) / 1024,
            mse_info.frame_mse[0], mse_info.frame_mse[1], mse_info.frame_mse[2],
            mse_info.max_block_mse[0], mse_info.bmse_cu_x[0], mse_info.bmse_cu_y[0],
            mse_info.max_block_mse[1], mse_info.bmse_cu_x[1], mse_info.bmse_cu_y[1],
            mse_info.max_block_mse[2], mse_info.bmse_cu_x[2], mse_info.bmse_cu_y[2],
            cur_psnr_y, cur_psnr_u, cur_psnr_v, cur_ssim_y, cur_ssim_u, cur_ssim_u, once_time / 1e6);

        if (case_cfg_param.rec_flag)
        {
            for (i = 0; i < real_height[0]; i++)
            {
                for (j = 0; j < real_width[0]; j++)
                {
                    fwrite(prc_out.recon_dbk_y + j, byte_size, 1, fp_rec);
                }
                prc_out.recon_dbk_y = prc_out.recon_dbk_y + code_width[0];
            }
            for (i = 0; i < real_height[1]; i++)
            {
                for (j = 0; j < real_width[1]; j++)
                {
                    fwrite(prc_out.recon_dbk_u + j, byte_size, 1, fp_rec);
                }
                prc_out.recon_dbk_u = prc_out.recon_dbk_u + code_width[1];
            }
            for (i = 0; i < real_height[1]; i++)
            {
                for (j = 0; j < real_width[1]; j++)
                {
                    fwrite(prc_out.recon_dbk_v + j, byte_size, 1, fp_rec);
                }
                prc_out.recon_dbk_v = prc_out.recon_dbk_v + code_width[1];
            }

            // 写重构开启，需要地址退回
            prc_out.recon_dbk_y -= real_height[0] * code_width[0];
            prc_out.recon_dbk_u -= real_height[1] * code_width[1];
            prc_out.recon_dbk_v -= real_height[1] * code_width[1];
        }
    }

SEQUENCE_EXIT:

    fprintf(stdout, "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    fprintf(stdout, "%-6s%16.4f%16.4f%16.4f%16.4f%16.2f             %16.2f             %16.2f             %16.4f%16.4f%16.4f%16.4f%16.4f%16.4f%16.4f\n", "avg",
        (double)((bit_size << 3) * coding_ctrl.frame_rate_num) / (1024 * fn),
        mse_y / fn, mse_u / fn, mse_v / fn, block_mse_y, block_mse_u, block_mse_v,
        psnr_y / fn, psnr_u / fn, psnr_v / fn, ssim_y / fn, ssim_u / fn, ssim_u / fn,
        avg_time / (1e6 * (dpb_ref_ctrl.intra_period == 1 ? fn : HLM_MAX(1, cnt_p))));

    if (HLM_NULL != fp_input)
    {
        fclose(fp_input);
        fp_input = HLM_NULL;
    }
    if (HLM_NULL != fp_output)
    {
        fclose(fp_output);
        fp_output = HLM_NULL;
    }
    if (HLM_NULL != fp_rec)
    {
        fclose(fp_rec);
        fp_rec = HLM_NULL;
    }
    if (src_y != HLM_NULL)
    {
        free(src_y);
        src_y = HLM_NULL;
    }
    if (bak_src_y != HLM_NULL)
    {
        free(bak_src_y);
        bak_src_y = HLM_NULL;
    }
    if (stream_buf != HLM_NULL)
    {
        free(stream_buf);
        stream_buf = HLM_NULL;
    }
    if (out_stream_buf != HLM_NULL)
    {
        free(out_stream_buf);
        out_stream_buf = HLM_NULL;
    }
    if (recon_dbk_y != HLM_NULL)
    {
        free(recon_dbk_y);
        recon_dbk_y = HLM_NULL;
    }
    if (recon_dbk_u != HLM_NULL)
    {
        free(recon_dbk_u);
        recon_dbk_u = HLM_NULL;
    }
    if (recon_dbk_v != HLM_NULL)
    {
        free(recon_dbk_v);
        recon_dbk_v = HLM_NULL;
    }

    return sts;
}

/***************************************************************************************************
* 功  能：处理一个测试用例
* 参  数：*
*         fp_cfg               -I         测试用例配置文件名
*         fp_log               -O         log文件指针
*         cmd                  -I         命令行参数
* 返回值：0：成功；-1：失败
* 备  注：
***************************************************************************************************/
HLM_S32 HLMC_CASE_Process(FILE           *fp_cfg,
                          FILE           *fp_log,
                          HLMC_DEMO_CMD  *cmd)
{
    HLM_S32      ret                      = 0;
    HLM_VOID    *handle                   = 0;
    HLM_MEM_TAB  mem_tab[HLM_MEM_TAB_NUM] = { 0 };

    HLMC_CASE_InitCfgFile(&case_cfg_param, &coding_ctrl, &rate_ctrl, &dpb_ref_ctrl);
    ret = HLMC_CASE_ReadCfgFile(fp_cfg, fp_log);
    if (ret != 0)
    {
        fprintf(stderr, "Unable to read case config file %s, in Func <%s>, Line <%d>\n", cmd->file_cfg, __FUNCTION__, __LINE__);
        fprintf(fp_log, "Unable to read case config file %s, in Func <%s>, Line <%d>\n", cmd->file_cfg, __FUNCTION__, __LINE__);
        goto CASE_EXIT;
    }

    HLMC_CMD_SetParameter(cmd, &case_cfg_param, &coding_ctrl, &rate_ctrl, &dpb_ref_ctrl);

    // 打印编码参数
    fprintf(stdout, "[ Input  ] %s\n", case_cfg_param.input_file);
    fprintf(stdout, "[ Output ] %s\n", case_cfg_param.output_file);
    fprintf(stdout, "[ Recon  ] %s\n", case_cfg_param.rec_file);
    fprintf(stdout, "[ Source ] Frames:%d  FPS:%d  Width:%d  Height:%d  Bit_Depth:%d\n",
        case_cfg_param.total_frames, coding_ctrl.frame_rate_num, coding_ctrl.width, coding_ctrl.height, coding_ctrl.bitdepth);
    fprintf(stdout, "[ Config ] Profile:%d  I_Period:%d  PatchWidth:%d  PatchHeight:%d  I_BPP:%d  P_BPP:%d\n",
        coding_ctrl.profile, dpb_ref_ctrl.intra_period,
        coding_ctrl.patch_info.patch_param[0].patch_width[0], coding_ctrl.patch_info.patch_param[0].patch_height[0],
        rate_ctrl.rc_cbr_ctrl.bpp_i, rate_ctrl.rc_cbr_ctrl.bpp_p);
#if FIX_CFG_ENC
    fprintf(stdout, "[ Tools  ] I_IBC:%d  P_IBC:%d  Sub_IBC:%d  Intra_8x8:%d  Intra_16x1_2x8:%d  LCsep:%d  Sub_LCsep:%d\n",
        coding_ctrl.i_frame_enable_ibc, coding_ctrl.p_frame_enable_ibc, coding_ctrl.sub_ibc_enable_flag,
        coding_ctrl.intra_8x8_enable_flag, coding_ctrl.intra_16x1_2x8_enable_flag,
        coding_ctrl.intra_chroma_mode_enable_flag, coding_ctrl.intra_sub_chroma_mode_enable_flag);
#else
    fprintf(stdout, "[ Tools  ] I_IBC:%d  P_IBC:%d  Sub_IBC:%d  Intra_8x8:%d  Intra_16x1_2x8:%d\n",
        coding_ctrl.i_frame_enable_ibc, coding_ctrl.p_frame_enable_ibc, coding_ctrl.sub_ibc_enable_flag,
        coding_ctrl.intra_8x8_enable_flag, coding_ctrl.intra_16x1_2x8_enable_flag);
#endif
    fprintf(stdout, "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    ret = HLMC_CASE_CreateHandle(mem_tab, &handle, fp_log);
    if (ret != 0)
    {
        fprintf(stderr, "Unable to Create handle, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "Unable to Create handle, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        goto CASE_EXIT;
    }

    ret = HLMC_CASE_ProcessSequence(handle, fp_log);
    if (ret != 0)
    {
        fprintf(stderr, "Unable to Process this Sequence, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        fprintf(fp_log, "Unable to Process this Sequence, in Func <%s>, Line <%d>\n", __FUNCTION__, __LINE__);
        goto CASE_EXIT;
    }

CASE_EXIT:

    ret = HLMC_CASE_DestroyHandle(mem_tab, handle, fp_log);

    return ret;
}
