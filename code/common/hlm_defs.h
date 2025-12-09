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
#ifndef _HLM_DEFS_H_
#define _HLM_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

// 版本号
#define HLM_MAJOR_VERSION                (3)
#define HLM_MINOR_VERSION                (0)
#define HLM_REVISION_VERSION             (0)

#define FIX_BVP                          (1)            // 修复bvp计算bug
#define FIX_LINUX                        (1)            // 修复linux编解码不匹配以及编解码时间统计异常bug
#define FIX_ENC                          (1)            // 修复随机内容的编码buffer溢出bug

// HLM3.0改动的地方
#define FIX_1                            (1)            // 修复intra8x8下nbi预测模式存储bug
#define ENC_OPT                          (1)            // M9308: 基于cost的码控优化 + rc bug fix
#define Q_DEADZONE_OPT                   (1)            // M9308: 变换跳过模式量化死区调整
#define TWO_STAGE_IBC_SEARCH             (1)            // M9305: 两阶段搜索，先搜索4x4，再搜索子块
#define QP_RANGE_ADJUST                  (1)            // M9304: qp范围从[0,51]改为[0,47]
#define INTRA_CHROMA_MODE_SEPARATE       (1)            // M9306: 亮色度分离
#define QT_CLIP                          (1)            // 修复变换量化clip bug
#define FIX_LINE_BY_LINE                 (1)            // 修复逐行逐列色度粗搜bug
#define FIX_PATCH_ENC                    (1)            // 修复均匀划分patch时，最后一行/列的宽高计算bug
#define FIX_CFG_ENC                      (1)            // 修复配置文件中缺失部分高层语法bug
#define FIX_2                            (1)            // 修复写码流bug
#define FIX_3                            (1)            // 修复定QP的编码bug

#define HLM_INTRA_SEARCH_REC             (1)             // 帧内粗搜上一行使用重建像素
#define HLM_INTRA_SEARCH_REC_L           (1)             // 逐行粗搜上一行使用重建像素
#define HLM_INTRA_SEARCH_REC_R           (1)             // 逐列粗搜上一行使用重建像素
/***************************************************************************************************
* 多平台64位宏定义
***************************************************************************************************/
#if (defined (_WIN64)                    \
    || defined (_LINUX64)                \
    || defined (_MAC64)                  \
    || defined (_IOS64)                  \
    || defined (_ANDROID64)              \
    || defined (ARCHS_STANDARD_64_BIT))
#define _HLM_SYS64
#endif

/***************************************************************************************************
* 编解码端公共宏定义
***************************************************************************************************/
#if defined (_HLM_SYS64)
#define HLM_IMG_WIDTH_MAX                 (15360)                      // 编码算法模型支持的最小图像宽度
#define HLM_IMG_HEIGHT_MAX                (8640)                       // 编码算法模型支持的最小图像高度
#else
#define HLM_IMG_WIDTH_MAX                 (7680)                       // 编码算法模型支持的最小图像宽度
#define HLM_IMG_HEIGHT_MAX                (4320)                       // 编码算法模型支持的最小图像高度
#endif
#define HLM_IMG_WIDTH_MIN                 (16)                         // 编码算法模型支持的最小图像宽度
#define HLM_IMG_HEIGHT_MIN                (8)                         // 编码算法模型支持的最小图像高度
#define HLM_LOG2_WIDTH_SIZE               (4)                          // LCU尺寸：2^4 = 16
#define HLM_LOG2_HEIGHT_SIZE              (3)                          // LCU尺寸：2^3 = 8
#define HLM_WIDTH_SIZE                    (1 << HLM_LOG2_WIDTH_SIZE)   // LCU尺寸：16
#define HLM_HEIGHT_SIZE                   (1 << HLM_LOG2_HEIGHT_SIZE)  // LCU尺寸：8
#define HLM_CU_SIZE                       (1 << (HLM_LOG2_WIDTH_SIZE + HLM_LOG2_HEIGHT_SIZE ))
#define HLM_TU_4x4_NUMS                   (HLM_CU_SIZE >> 4)           // 4x4变换块的个数
#define HLM_MAX_PATCH_NUM                 (256)                        // 最大Patch个数
#define HLM_IBC_HOR_SEARCH_LOG            (7)                          // IBC水平搜索范围的log
#define HLM_IBC_BUFFER_WIDTH              ((1 << HLM_IBC_HOR_SEARCH_LOG) + 4)
#define HLM_IBC_SEARCH_AREA_WIDTH         (HLM_IBC_BUFFER_WIDTH + 16)  // 搜索区域缓存的宽度
#define HLM_IBC_SEARCH_AREA_HEIGHT        (8)                          // 搜索区域缓存的高度
#define HLM_BV_MERGE_NUM                  (4)                          // bv-merge类型数
#define HLM_INTRA_REF_PIXEL_NUM_16x8      (25)                         // 16x8的参考像素，一共16+8+1=25个
#define HLM_INTRA_REF_PIXEL_NUM_4x4       (25)                         // 8x8的参考像素，一共8+8+8+1=13个
#define START_CODE_FIX                    (1)                          // startcode增加3byte
#define PRED_MODE_NUM                     (4)                          // intra16x1_2x8技术的预测模式数
#define HLM_DELAY_CU_NUM                  (3)                          // 延迟3个CU

/***************************************************************************************************
* 基本数据类型的定义
***************************************************************************************************/
typedef signed char         HLM_S08;
typedef unsigned char       HLM_U08;
typedef signed short        HLM_S16;
typedef unsigned short      HLM_U16;
typedef signed int          HLM_S32;
typedef unsigned int        HLM_U32;
typedef float               HLM_F32;
typedef double              HLM_F64;
typedef signed int          HLM_COEFF;

#if defined(_WIN32) || defined (_WIN64)
    typedef signed __int64      HLM_S64;
    typedef unsigned __int64    HLM_U64;
#else
    typedef signed long long    HLM_S64;
    typedef unsigned long long  HLM_U64;
#endif

#if defined (_HLM_SYS64)
    typedef HLM_U64      HLM_SZT;
#else
    typedef HLM_U32      HLM_SZT;
#endif

#ifndef HLM_VOID
#define HLM_VOID    void
#endif
#ifndef HLM_NULL
#define HLM_NULL    0
#endif

/***************************************************************************************************
* 基本常数定义
***************************************************************************************************/
#define HLM_MIN_U08     ( 0 )                           // 最小8位无符号整型值
#define HLM_MAX_U08     ( 0xFF )                        // 最大8位无符号整型值
#define HLM_MIN_U16     ( 0 )                           // 最小16位无符号整型值
#define HLM_MAX_U16     ( 0xFFFF )                      // 最大16位无符号整型值
#define HLM_MIN_U32     ( 0 )                           // 最小32位无符号整型值
#define HLM_MAX_U32     ( 0xFFFFFFFF )                  // 最大32位无符号整型值
#define HLM_MIN_S08     (-128 )                         // 最小8位有符号整型值
#define HLM_MAX_S08     ( 127 )                         // 最大8位有符号整型值
#define HLM_MIN_S16     (-32768 )                       // 最小16位有符号整型值
#define HLM_MAX_S16     ( 32767 )                       // 最大16位有符号整型值
#define HLM_MIN_S32     (-2147483647 - 1 )              // 最小32位有符号整型值
#define HLM_MAX_S32     ( 2147483647 )                  // 最大32位有符号整型值

#if defined(_WIN32) || defined (_WIN64)
#define HLM_MAX_S64  ( 9223372036854775807i64)          // 最小64位有符号整型值(windows平台)
#define HLM_MIN_S64  (-9223372036854775807i64 - 1)      // 最大64位有符号整型值(windows平台)
#else
#define HLM_MAX_S64  ( 9223372036854775807LL )          // 最小64位有符号整型值(非windows平台)
#define HLM_MIN_S64  (-9223372036854775807LL - 1 )      // 最大64位有符号整型值(非windows平台)
#endif

#define HLM_MINABS_F32  ( 1.175494351e-38f )            // 绝对值最小单精度值
#define HLM_MAXABS_F32  ( 3.402823466e+38f )            // 绝对值最大单精度值
#define HLM_EPS_F32     ( 1.192092890e-07f )            // 单精度浮点标准中最小差值
#define HLM_MINABS_F64  ( 2.2250738585072014e-308 )     // 绝对值最小双精度值
#define HLM_MAXABS_F64  ( 1.7976931348623158e+308 )     // 绝对值最大双精度值
#define HLM_EPS_F64     ( 2.2204460492503131e-016 )     // 双精度浮点标准中最小差值

#if defined (_HLM_SYS64)
#define HLM_MAX_MEM_SIZE   HLM_MAX_S64    // 最大内存
#else
#define HLM_MAX_MEM_SIZE   (4294966272U)  // 2^32 - 2^10
#endif

/***************************************************************************************************
* 基本函数定义
***************************************************************************************************/
#define HLM_ABS(x)                (((HLM_S32)(x) < 0) ? (-(HLM_S32)(x)) : (HLM_S32)(x))
#define HLM_SABS(x)               (((HLM_S16)(x) < 0) ? (-(HLM_S16)(x)) : (HLM_S16)(x))
#define HLM_LABS(x)               (((HLM_S64)(x) < 0) ? (-(HLM_S64)(x)) : (HLM_S64)(x))
#define HLM_FABS(x)               (((HLM_F32)(x) < 0) ? (-(HLM_F32)(x)) : (HLM_F32)(x))
#define HLM_DABS(x)               (((HLM_F64)(x) < 0) ? (-(HLM_F64)(x)) : (HLM_F64)(x))
#define HLM_MAX(a, b)             (((a) < (b)) ? (b) : (a))
#define HLM_MIN(a, b)             (((a) > (b)) ? (b) : (a))
#define HLM_CLIP(v, minv, maxv)   HLM_MIN((maxv), HLM_MAX((v), (minv)))
#define HLM_ROUND(x)              (((x) < 0.0) ? (HLM_S32)((x) - 0.5) : (HLM_S32)((x) + 0.5))

//参数校验返回状态码
#define PRT_LOG(...) printf(__VA_ARGS__)
#define HLM_CHECK_ERROR(flag, sts)    \
{                                     \
    if (flag)                         \
    {                                 \
        PRT_LOG("HLM_CHECK_ERROR: Function<%s>, Line<%d>, sts<%d>\n", __FUNCTION__, __LINE__, sts); \
        return (sts);                 \
    }                                 \
}

//对齐运算
#define HLM_SIZE_ALIGN(size, align)  (((size) + ((align) - 1)) & (~((align) - 1)))
#define HLM_SIZE_ALIGN_4(size)       HLM_SIZE_ALIGN(size, 4)
#define HLM_SIZE_ALIGN_8(size)       HLM_SIZE_ALIGN(size, 8)
#define HLM_SIZE_ALIGN_16(size)      HLM_SIZE_ALIGN(size, 16)
#define HLM_SIZE_ALIGN_32(size)      HLM_SIZE_ALIGN(size, 32)
#define HLM_SIZE_ALIGN_64(size)      HLM_SIZE_ALIGN(size, 64)
#define HLM_SIZE_ALIGN_128(size)     HLM_SIZE_ALIGN(size, 128)

/***************************************************************************************************
* 内存管理器HLM_MEM_TAB结构体定义
***************************************************************************************************/
//内存对齐属性
typedef enum _HLM_MEM_ALIGNMENT
{
    HLM_MEM_ALIGN_NULL     = 0,   // 不需要对齐，不能在HLM_SIZE_ALIGN宏中使用
    HLM_MEM_ALIGN_1BYTE    = 1,
    HLM_MEM_ALIGN_4BYTE    = 4,
    HLM_MEM_ALIGN_8BYTE    = 8,
    HLM_MEM_ALIGN_16BYTE   = 16,
    HLM_MEM_ALIGN_32BYTE   = 32,
    HLM_MEM_ALIGN_64BYTE   = 64,
    HLM_MEM_ALIGN_128BYTE  = 128,
    HLM_MEM_ALIGN_256BYTE  = 256
}HLM_MEM_ALIGNMENT; 

//内存属性（从算法应用角度的复用性对内存进行分类）
typedef enum _HLM_MEM_ATTRS
{
    HLM_MEM_SCRATCH,                 //可复用内存，能在多路切换时有条件复用  
    HLM_MEM_PERSIST                  //不可复用内存
}HLM_MEM_ATTRS;

//内存分配空间（从内存分配的物理位置来分类）
typedef enum _HLM_MEM_SPACE
{
    HLM_MEM_INTERNAL_RAM,            //片内随机读写存储区
    HLM_MEM_INTERNAL_ROM,            //片内只读存储区
    HLM_MEM_EXTERNAL_DDR             //片外DDR存储区
}HLM_MEM_SPACE;

//内存分配结构体 
typedef struct _HLM_MEM_TAB
{
    HLM_SZT             size;        //ASIC中可以BYTE不对齐，C Model中为BYTE单位的存储区大小
    HLM_MEM_ALIGNMENT   alignment;   //内存对齐属性, 一般RAM/ROM不需要考虑对齐问题，DDR中要求16BYTE对齐
    HLM_MEM_SPACE       space;       //内存分配空间 
    HLM_MEM_ATTRS       attrs;       //内存属性 
    HLM_VOID           *base;        //分配出的内存指针 
}HLM_MEM_TAB;

// 缓存表的个数
typedef enum _HLM_MEM_TAB_TYPE
{
    HLM_MEM_TAB_DDR_PERSIST = 0,     // 不可复用内存
    HLM_MEM_TAB_DDR_SCRATCH,         // DDR缓存中可复用内存，能在多路切换时有条件复用
    HLM_MEM_TAB_RAM_SCRATCH,         // 模拟ASIC中RAM缓存
    HLM_MEM_TAB_NUM                  // 缓存表总个数
} HLM_MEM_TAB_TYPE;

/***************************************************************************************************
* 图像结构体定义
***************************************************************************************************/
// 分量枚举
typedef enum _HLM_COMPONENT
{
    HLM_LUMA_Y = 0,                  // luma
    HLM_CHROMA_U = 1,                // chroma U
    HLM_CHROMA_V = 2,                // chroma V
    HLM_MAX_COMP_NUM = 3
} HLM_COMPONENT;

//图像格式（新的类型添加在最后面，保持前向兼容，每种格式按段长度100划分）
typedef enum _HLM_IMAGE_FORMAT
{
    HLM_IMG_RGB,
    HLM_IMG_YUV_444,
    HLM_IMG_YUV_422,
    HLM_IMG_YUV_420,
    HLM_IMG_YUV_400,
}HLM_IMAGE_FORMAT;

// 帧类型
typedef enum _HLM_FRAME_TYPE
{
    HLM_FRAME_TYPE_NONE = -1,
    HLM_FRAME_TYPE_P,
    HLM_FRAME_TYPE_B,
    HLM_FRAME_TYPE_I,
} HLM_FRAME_TYPE;

// 宏块类型
typedef enum _HLM_CU_TYPE
{
    HLM_I_4x4,
    HLM_I_16x8,
    HLM_I_LINE,
    HLM_I_ROW,
    HLM_IBC_4x4,
    HLM_P_8x8,
    HLM_P_16x8,
    HLM_P_SKIP,
}HLM_CU_TYPE;

// 处理阶段
typedef enum _HLM_PROC
{
    PROC_PRE,  // 粗筛预决策
    PROC_RDO,  // RDO决策
    PROC_BS,   // 读写码流
} HLM_PROC;

// SEI负载类型
typedef enum _SEI_PAYLOAD_TYPE
{
    USER_DATA_MD5 = 16,
    USER_DATA_RESERVED,
    SEI_PAYLOAD_TYPE_NUM,
}SEI_PAYLOAD_TYPE;

// Nalu类型
typedef enum _HLM_NAL_UNIT_TYPE
{
    HLM_NONIDR_NUT   = 1,            // 非IDR图像
    HLM_IDR_NUT      = 2,            // IDR图像
    HLM_SEI_NUT      = 3,            // SEI
    HLM_SPS_NUT      = 4,            // 序列头
    HLM_PPS_NUT      = 5,            // 图像头
    HLM_EOCVS_NUT    = 6,            // end_of_cvs_rbsp
    HLM_EOS_NUT      = 7,            // end_of_stream_rbsp
} HLM_NAL_UNIT_TYPE;

// 图像格式表示结构体
typedef struct _HLM_IMAGE
{
    HLM_IMAGE_FORMAT    format;      // 图像格式，按照数据类型HLM_IMAGE_FORMAT赋值
    HLM_S32             width[2];    // 图像宽度
    HLM_S32             height[2];   // 图像高度
    HLM_S32             bitdepth;    // 比特深度
    HLM_S32             step[4];     // 行间距
    HLM_VOID           *data[4];     // 数据存储地址
}HLM_IMAGE;

// 运动矢量
typedef struct _HLM_MV
{
    HLM_S16 mvx;
    HLM_S16 mvy;
} HLM_MV;

/***************************************************************************************************
* 状态码
***************************************************************************************************/
//状态码数据类型
typedef int HLM_STATUS;    //组件接口函数返回值都定义为该类型

//函数返回状态类型
//算法库可以在库头文件中自定义状态类型，自定义状态类型值 < -1000。
typedef enum _HLM_STATUS_CODE
{
    //cpu指令集支持错误码
    HLM_STS_ERR_CPUID           = -29,    //cpu不支持优化代码中的指令集

    //内部模块返回的基本错误类型
    HLM_STS_ERR_STEP            = -28,    //数据step不正确（除HLM_IMAGE结构体之外）
    HLM_STS_ERR_DATA_SIZE       = -27,    //数据大小不正确（一维数据len，二维数据的HLM_SIZE）
    HLM_STS_ERR_BAD_ARG         = -26,    //参数范围不正确

    //算法库加密相关错误码定义
    HLM_STS_ERR_EXPIRE          = -25,    //算法库使用期限错误
    HLM_STS_ERR_ENCRYPT         = -24,    //加密错误

    //以下为组件接口函数使用的错误类型
    HLM_STS_ERR_CALL_BACK       = -23,    //回调函数出错
    HLM_STS_ERR_OVER_MAX_MEM    = -22,    //超过限定最大内存
    HLM_STS_ERR_NULL_PTR        = -21,    //函数参数指针为空（共用）

    //检查HLM_KEY_PARAM、HLM_KEY_PARAM_LIST成员变量的错误类型
    HLM_STS_ERR_PARAM_NUM       = -20,    //param_num参数不正确
    HLM_STS_ERR_PARAM_VALUE     = -19,    //value参数不正确或者超出范围
    HLM_STS_ERR_PARAM_INDEX     = -18,    //index参数不正确

    //检查cfg_type, cfg_size, prc_type, in_size, out_size, func_type是否正确
    HLM_STS_ERR_FUNC_SIZE       = -17,    //子处理时输入、输出参数大小不正确
    HLM_STS_ERR_FUNC_TYPE       = -16,    //子处理类型不正确
    HLM_STS_ERR_PRC_SIZE        = -15,    //处理时输入、输出参数大小不正确
    HLM_STS_ERR_PRC_TYPE        = -14,    //处理类型不正确
    HLM_STS_ERR_CFG_SIZE        = -13,    //设置、获取参数输入、输出结构体大小不正确
    HLM_STS_ERR_CFG_TYPE        = -12,    //设置、获取参数类型不正确

    //检查HLM_IMAGE成员变量的错误类型
    HLM_STS_ERR_IMG_DATA_NULL   = -11,    //图像数据存储地址为空（某个分量）
    HLM_STS_ERR_IMG_STEP        = -10,    //图像宽高与step参数不匹配
    HLM_STS_ERR_IMG_SIZE        = -9,     //图像宽高不正确或者超出范围
    HLM_STS_ERR_IMG_FORMAT      = -8,     //图像格式不正确或者不支持

    //检查HLM_MEM_TAB成员变量的错误类型
    HLM_STS_ERR_MEM_ADDR_ALIGN  = -7,     //内存地址不满足对齐要求
    HLM_STS_ERR_MEM_SIZE_ALIGN  = -6,     //内存空间大小不满足对齐要求
    HLM_STS_ERR_MEM_LACK        = -5,     //内存空间大小不够
    HLM_STS_ERR_MEM_ALIGN       = -4,     //内存对齐不满足要求
    HLM_STS_ERR_MEM_NULL        = -3,     //内存地址为空

    //检查ability成员变量的错误类型
    HLM_STS_ERR_ABILITY_ARG     = -2,     //ABILITY存在无效参数

    //通用类型
    HLM_STS_ERR                 = -1,     //不确定类型错误（接口函数共用）
    HLM_STS_OK                  =  0,     //处理正确（接口函数共用）
    HLM_STS_WARNING             =  1      //警告
}HLM_STATUS_CODE;

#ifdef __cplusplus
}
#endif 

#endif //_HLM_DEFS_H_
