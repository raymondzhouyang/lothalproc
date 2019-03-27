/*
 * R2Types.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/3/7
 * Copyright ? 2018 rokid. All rights reserved.
 */

/** \file R2Types.hpp
 * \brief 定义错误码及公共数据结构
 */
#ifndef _R2TYPES_HPP_
#define _R2TYPES_HPP_

#include <string>

/**
 * \brief 该结结构包含了语音相关的信息，一些语音事件通过
 * 该结构体传递语音信息。
 *
 * 目前使用该结构体的事件包括：
 * \li \link R2_EVENT_VAD_BEGIN \endlink
 * \li \link R2_EVENT_VAD_END \endlink
 * \li \link R2_EVENT_VOICE_LOCAL_START \endlink
 * \li \link R2_EVENT_VOICE_SLEEP \endlink
 * \li \link R2_EVENT_VOICE_COMING \endlink
 * \li \link R2_EVENT_VOICE_LOCAL_AWAKE \endlink
 */
struct R2VoiceInfo {
	float sl;		/**< 寻向角度（弧度） */
	float voice_energy;	/**< 声音能量 */
	float threshold_energy;	/**< 阈值 */

	std::string voice_trigger;	/**< 激活词内容 */
	uint32_t trigger_start;
	uint32_t trigger_length;
	bool trigger_confirm;
	std::string extra;
};

/**
 * \brief Lothal 错误返回值。
 */
enum R2ErrorCode {
	R2_EOK = 0,		/**< 请求已被接受，或操作已执行成功。*/
	R2_EEXIST,		/**< 操作已经执行过，或者所请求的对象已经存在。*/
	R2_EIGNORE,		/**< 本次操作未完成，因某些策略配置原因，而结束本次操作。*/
	R2_ENOMEMORY,	/**< 内存不足，内存分配失败。*/
	R2_ESYSTEM,		/**< 出现系统错误，比如系统调用失败或第三方库调用返回失败等。*/
	R2_EWRONGARGS,	/**< 输入参数有误。*/
	R2_EOVERFLOW,	/**< 出现数组越界，比如返回数据超出缓存空间或其他缓存溢出情况。*/
	R2_ENOTSUPPORT,	/**< 该功能不支持，比如相关模块未使能，或者输入参数有误（但不是无效参数）等。*/
	R2_ENOTREADY,	/**< 模块尚未准备就绪，比如仍在初始化，或者pipeline被暂停等。*/
	R2_EMAXCODE,	/**< 最大错误码值，不用作返回值。*/
};

/**
 * MIC 阵列类型
 */
enum R2MicArrayShape {
	R2_SHAPE_LINEAR = 10,	/**< 线麦 */
	R2_SHAPE_PLANE = 11,	/**< 环麦 */
};

/**
 * 音频输入 bit 格式
 */
enum R2BitFormat {
	R2_BIT_FORMAT_SHORT,
	R2_BIT_FORMAT_INT24,
	R2_BIT_FORMAT_INT32,
	R2_BIT_FORMAT_FLOAT32,
	R2_BIT_FORMAT_NUM,
};

/**
 * VOIP通话数据来源
 */

#define R2_VOPI_SOURCE_AEC "VOIP_SOURCE_AEC"
#define R2_VOPI_SOURCE_BF "VOIP_SOURCE_BF"


#if defined(__arm__) || defined(__aarch64__) || defined(__ANDROID__)
#define R2_BASEPORT	30000
#else
#define R2_BASEPORT	40000
#endif

/** \brief 测试两个浮点数是否相等。 */
#define R2_FEQUAL(f1, f2)		(fabs((f1) - (f2)) < 0.001)

#endif /* _R2TYPES_HPP_ */

