/*
 * R2SpeechEventHandler.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/31
 * Copyright © 2018 rokid. All rights reserved.
 */

#ifndef HAVE_FLORA

#ifndef _R2SPEECHEVENTHANDLER_HPP_
#define _R2SPEECHEVENTHANDLER_HPP_

#include <string>

namespace lothal {

typedef struct {
	uint32_t version;
	/* 0: awake; 1: sleep */
	uint32_t cmd;
	uint32_t sl;
} AwakeEvent;

class R2SpeechEventHandler: public R2EventHandler {
public:
	R2SpeechEventHandler(R2Lothal &lothal, R2Speech &speech);
	~R2SpeechEventHandler();

	int onEvent(R2Event &event);

	static R2EventHandlerSp setupEventHandler(
			R2Lothal &lothal, R2Speech &speech);
	static void removeEventHandler(R2EventHandlerSp handler);

private:
	int onVoiceEvent(R2Event &event);
	int onSpeechEvent(R2Event &event);

	int genId();
	int getCurrentId();

	void sendEvent(const std::string &name,
			int seq, int speech_id, R2SpeechInfo *si);
	void sendEvent(const std::string &name,
			int seq, int speech_id, R2VoiceInfo *vi);

private:
	R2Lothal &lothal_;
	R2Speech &speech_;
	r2base::R2OutServer *awake_server_ = NULL;
	r2base::R2OutServer *event_server_ = NULL;
	int seq_ = 0;
};

}

#endif /* _R2SPEECHEVENTHANDLER_HPP_ */

#endif
