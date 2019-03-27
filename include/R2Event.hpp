/*
 * R2Event.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/3/7
 * Copyright ? 2018 rokid. All rights reserved.
 */
/** \file R2Event.hpp
 * \brief 定义事件及声明事件处理接口
 */

#ifndef _R2EVENT_HPP_
#define _R2EVENT_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>	/* shared_ptr */

namespace lothal {

/**
 * \brief 语音激活事件，SDK ENGINE 模块在检测到语音激活时会触发该事件。
 *
 * 详细激活信息见 \link R2VoiceInfo \endlink。
 */
#define R2_EVENT_VOICE_COMING       "EVENT_VOICE_COMING"

/**
 * \brief 带指令语音激活事件，比如: "若琪你好"。事件信息
 * 见 \link R2VoiceInfo \endlink。当前 SDK 未产生该事件。
 */
#define R2_EVENT_VOICE_COMING_CMD 	"EVENT_VOICE_COMING_CMD"

/**
 * \brief 不带指令语音激活事件，比如: "若琪，你好"。事件信息
 * 见 \link R2VoiceInfo \endlink。当前 SDK 未产生该事件。
 */
#define R2_EVENT_VOICE_COMING_NOCMD "EVENT_VOICE_COMING_NOCMD"

/**
 * \brief 本地激活事件，基于等同于 \link R2_EVENT_VOICE_COMING \endlink。
 * 事件信息见 \link R2VoiceInfo \endlink。SDK ENGINE 模块在检测到语音激
 * 活时会触发该事件。
 */
#define R2_EVENT_VOICE_LOCAL_AWAKE	"EVENT_VOICE_LOCAL_AWAKE"

/**
 * \brief 本地语音开始事件，开始上传语音数据。SDK VAD 模块
 * 在已唤醒时会触发该事件。语音信息见 \link R2VoiceInfo \endlink。
 */
#define R2_EVENT_VOICE_LOCAL_START  "EVENT_VOICE_LOCAL_START"

/**
 * \brief 检测到语音休眠事件，比如检测到休眠词“没事了”。
 * 目前 SDK ENGINE 可触发该事件，在配置文件中去掉休眠
 * 词“meishile”可避免产生该事件。
 *
 * 事件信息见 \link R2VoiceInfo \endlink。
 */
#define R2_EVENT_VOICE_SLEEP		"EVENT_VOICE_SLEEP"

/**
 * \brief VAD 开始事件，事件信息见 \link R2VoiceInfo \endlink。SDK VAD 模块会
 * 产生该事件。
 */
#define R2_EVENT_VAD_BEGIN			"EVENT_VAD_BEGIN"

/**
 * \brief VAD 结束事件，事件信息见 \link R2VoiceInfo \endlink。SDK VAD 模块会
 * 产生该事件。
 */
#define R2_EVENT_VAD_END			"EVENT_VAD_END"

/**
 * \brief 无效事件占位符，系统中不应该出现该事件。
 */
#define	R2_EVENT_NONE 				"EVENT_NONE"

/**
 * \brief 系统退出事件。
 */
#define R2_EVENT_SYS_EXIT 			"EVENT_SYS_EXIT"

/**
 * \brief SDK 准备就绪事件。
 */
#define R2_EVENT_OUTSERVER_READY	"EVENT_OUTSERVER_READY"

/**
 * \brief 离线识别事件。SDK ENGINE 模块在开启本地 ASR 之后才
 * 会产生该事件。事件信息携带离线识别到的结果，
 * 见 \link R2OfflineAsrInfo \endlink。
 */
#define R2_EVENT_OFFLINE_ASR		"EVENT_OFFLINE_ASR"

/** \struct R2OfflineAsrInfo
 * \brief 离线识别结果信息。
 */
struct R2OfflineAsrInfo {
    std::string asr_result;
};

/**
 * \brief 开关主动拾音
 */
#define R2_EVENT_PICKUP				"EVENT_PICKUP"

/**
 * \brief 云端Start事件，开始识别语音数据。事件参数见
 * \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_VOICE_CLOUD_START	"EVENT_VOICE_CLOUD_START"

/**
 * \brief 服务端接受本次激活事件，开始解析语音流->asr文本。
 * 事件参数见 \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_VOICE_ACCEPT		"EVENT_VOICE_ACCEPT"

/**
 * \brief 服务端拒绝本次激活事件，结束本次激活拾音整个流程。
 * 事件参数见 \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_VOICE_REJECT		"EVENT_VOICE_REJECT"

/**
 * \brief 服务端本次激活已经被取消事件，结束本次激活拾音整个流程。
 * 事件参数见 \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_VOICE_CANCEL		"EVENT_VOICE_CANCEL"

/**
 * \brief 服务端错误，结束本次激活拾音整个流程。
 * 事件参数见 \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_VOICE_ERROR		"EVENT_VOICE_ERROR"

/**
 * \brief 服务端fake事件，绑定同一个账户的两个设备, 且挨得很近，
 * 同时唤醒了, 其中一台设备就会fake掉, 结束本次激活拾音整个流程。
 * 事件参数见 \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_VOICE_FAKE			"EVENT_VOICE_FAKE"

/**
 * \brief 服务端声纹识别fake事件。事件参数见 \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_VOICE_INVALID		"EVENT_VOICE_INVALID"

/**
 * \brief 服务端为其他功能增加的字段，用于区分是否为连续
 * 指令，不管是否连续指令都下发 \link R2_EVENT_VOICE_ACCEPT \endlink,　
 * 直到确认非连续指令时，再下发该事件。事件参数见
 * \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_VOICE_ONESHOT		"EVENT_VOICE_ONESHOT"

/**
 * \brief ASR事件中间临时结果，返回ASR的中间结果。只要
 * 还在吐字中就会抛出这个事件。事件参数见 \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_ASR_TMP_RESULT		"EVENT_ASR_TMP_RESULT"

/**
 * \brief ASR事件最终结果，返回ASR的最终结果。事件参数见
 * \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_ASR_END_RESULT		"EVENT_ASR_END_RESULT"

/**
 * \brief NLP 事件，返回NLP结果。事件参数见 \link R2SpeechInfo \endlink。
 */
#define R2_EVENT_VOICE_NLP			"EVENT_VOICE_NLP"

/**
 * \brief 上报语音事件时携带的声音信息
 */
struct R2SpeechInfo {
	int speech_id;
	int error;
	std::string asr;
	std::string nlp;
	std::string action;
	std::string extra;
};

/**
 * \brief SDK初始化完成后，通过 RPC 调用进行参数设置。
 * 产生该事件以通知相应模块重新配置参数。上报事件时
 * 携带内容为 \link R2ConfigOption \endlink。
 */
#define R2_EVENT_RECONFIG		"EVENT_RECONFIG"
/**
 * \brief RPC 配置参数。
 */
struct R2ConfigOption {
	std::string host;
	uint32_t port;
	std::string branch;
	std::string key;
	std::string device_type_id;
	std::string secret;
	std::string device_id;

	/* milliseconds */
	uint32_t reconn_interval;
	uint32_t ping_interval;
	uint32_t no_resp_timeout;
};

/**
 * \brief 产生该事件以通知进行应用栈设置，参数为 string。
 */
#define R2_EVENT_SETSTACK		"EVENT_SETSTACK"

/**
 * \brief 产生该事件以通知进行技能设置，参数为 string。
 */
#define R2_EVENT_SETSKILL		"EVENT_SETSKILL"

/**
 * \brief 该事件用于通知上传数据，携带的数据信息为 \link R2PutDataInfo \endlink
 */
#define R2_EVENT_DATA_OUT		"EVENT_DATA_OUT"

/**
 * \brief 该事件用于通知上传经AEC后数据，携带的数据信息为 \link R2PutDataInfo \endlink。
 */
#define R2_EVENT_DATA_AEC		"EVENT_DATA_AEC"

/**
 * \brief 该事件用于通知上传经BF后数据，携带的数据信息为 \link R2PutDataInfo \endlink。
 */
#define R2_EVENT_DATA_BF		"EVENT_DATA_BF"


/**
 * \brief 该事件用于通知上传VOIP数据，携带的数据信息为 \link R2PutDataInfo \endlink。
 */
#define R2_EVENT_DATA_VOIP		"EVENT_DATA_VOIP"

/**
 * \brief 事件\link R2_EVENT_DATA_OUT \endlink ，\link R2_EVENT_DATA_AEC \endlink
 * \link R2_EVENT_DATA_BF \endlink 及 \link R2_EVENT_DATA_VOIP \endlink
 * 信息，用于传递具体数据及长度。
 */
struct R2PutDataInfo {
	unsigned char *data;
	int len;
};

/**
 * \brief 事件对象
 */
class R2Event {
public:
	R2Event(const std::string &name, void *p);
	virtual ~R2Event();
	std::string getName();
	void *getArgs();

private:
	std::string name_;
	void *args_ = NULL;
};

/**
 * \brief 事件处理对象
 */
class R2EventHandler {
public:
	R2EventHandler() {};
	virtual ~R2EventHandler() {};
public:
	virtual int onEvent(R2Event &event) = 0;
};

typedef std::shared_ptr<R2EventHandler> R2EventHandlerSp;
typedef std::vector<R2EventHandlerSp> HandlerVec;
typedef std::unordered_map<std::string, std::shared_ptr<HandlerVec>> EventHandlerMap;
typedef std::unordered_map<std::string, int> R2EventIdMap;
typedef std::unordered_map<std::string, int>::iterator R2EventIdMapIter;

}

#endif /* _R2EVENT_HPP_ */

