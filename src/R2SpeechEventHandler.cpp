/*
 * R2SpeechEventHandler.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/31
 * Copyright © 2018 rokid. All rights reserved.
 */

#ifndef HAVE_FLORA

#include "speech/speech.h"

#include "R2OutServer.hpp"
#include "R2Stream.hpp"
#include "R2Lothal.hpp"

#include "R2Client.hpp"
#include "R2Speech.hpp"
#include "R2SpeechEventHandler.hpp"

using namespace std;
using namespace r2base;

namespace lothal {

R2SpeechEventHandler::R2SpeechEventHandler(R2Lothal &lothal, R2Speech &speech):
		lothal_(lothal), speech_(speech) {
	event_server_ = R2_NEW(R2OutServer, R2_BASEPORT + 200, "EventServer");
	event_server_->startServer();

	awake_server_ = R2_NEW(R2OutServer, R2_BASEPORT + 201, "AwakeServer");
	awake_server_->startServer();
}

R2SpeechEventHandler::~R2SpeechEventHandler() {
	R2_DEL(awake_server_);
	R2_DEL(event_server_);
}

static R2EventIdMap getEventIdMap() {
	R2EventIdMap map;

	/* speech event */
	map[R2_EVENT_VOICE_CLOUD_START] = 31;
	map[R2_EVENT_VOICE_ACCEPT] = 40;
	map[R2_EVENT_VOICE_REJECT] = 41;
	map[R2_EVENT_VOICE_CANCEL] = 42;
	map[R2_EVENT_VOICE_ERROR] = 43;
	map[R2_EVENT_VOICE_FAKE] = 44;
	map[R2_EVENT_VOICE_INVALID] = 45;
	map[R2_EVENT_VOICE_ONESHOT] = 46;
	map[R2_EVENT_ASR_TMP_RESULT] = 50;
	map[R2_EVENT_ASR_END_RESULT] = 51;
	map[R2_EVENT_VOICE_NLP] = 60;

	/* voice event */
	map[R2_EVENT_VOICE_COMING] = 10;
	map[R2_EVENT_VOICE_COMING_CMD] = 11;
	map[R2_EVENT_VOICE_COMING_NOCMD] = 12;
	map[R2_EVENT_VOICE_LOCAL_AWAKE] = 20;
	map[R2_EVENT_VOICE_LOCAL_START] = 30;
	map[R2_EVENT_OFFLINE_ASR] = 55;
	map[R2_EVENT_VOICE_SLEEP] = 70;
	map[R2_EVENT_VAD_BEGIN] = 80;
	map[R2_EVENT_VAD_END] = 81;

	return map;
}

R2EventHandlerSp R2SpeechEventHandler::setupEventHandler(
		R2Lothal &lothal, R2Speech &speech) {
	R2EventIdMap map;
	R2EventIdMapIter iter;
	R2EventHandlerSp handler;
	string name;

	map = getEventIdMap();

	handler = make_shared<R2SpeechEventHandler>(lothal, speech);

	for (iter = map.begin(); iter != map.end(); iter++) {
		name = iter->first;
		lothal.setEventHandler(name, handler);
	}

	return handler;
}

void R2SpeechEventHandler::removeEventHandler(R2EventHandlerSp handler) {
	string name;
	R2EventIdMap map;
	R2EventIdMapIter iter;
	std::shared_ptr<R2SpeechEventHandler> d_ptr =
			std::dynamic_pointer_cast<R2SpeechEventHandler>(handler);
	R2Lothal &lothal = d_ptr->lothal_;

	map = getEventIdMap();
	for (iter = map.begin(); iter != map.end(); iter++) {
		name = iter->first;
		lothal.removeEventHandler(name, handler);
	}
}

int R2SpeechEventHandler::onEvent(R2Event &event) {
	std::string name = event.getName();

	if ((name == R2_EVENT_VOICE_COMING) ||
		(name == R2_EVENT_VOICE_COMING_CMD) ||
		(name == R2_EVENT_VOICE_COMING_NOCMD) ||
		(name == R2_EVENT_VOICE_LOCAL_AWAKE) ||
		(name == R2_EVENT_VOICE_LOCAL_START) ||
		(name == R2_EVENT_VOICE_SLEEP) ||
		(name == R2_EVENT_VAD_BEGIN) ||
		(name == R2_EVENT_VAD_END)) {
		return onVoiceEvent(event);
	}

	return onSpeechEvent(event);
}

int R2SpeechEventHandler::onSpeechEvent(R2Event &event) {
	AwakeEvent ae;
	std::string name = event.getName();
	R2SpeechInfo *si = (R2SpeechInfo *)event.getArgs();

	if ((si->speech_id > 0) && (si->speech_id < speech_.voiceID())) {
		R2Info("Skip speech event %s: %d, %d",
			name.c_str(), si->speech_id, speech_.voiceID());
		return R2_EOK;
	}

	R2Info("speech valid: %d, %s",
		speech_.getValidFlag(), name.c_str());

	if (name == R2_EVENT_VOICE_NLP) {
		speech_.set_pickup(false);
		speech_.endVoice();

		/* 被标志为'无效'时，应避免向系统组上报
		 * 该NLP事件。
		 */
		if (!speech_.getValidFlag())
			return R2_EOK;

		ae.version = 1;
		ae.cmd = 1;
		ae.sl = 0.0;

		if (awake_server_->isConnected()) {
			awake_server_->onData((unsigned char *)&ae,
				sizeof(ae));
		}
	} else if (name == R2_EVENT_ASR_END_RESULT) {
		if (si->asr.length() > 0) {
			/* 关闭拾音 */
            speech_.set_pickup(false);
			speech_.endVoice();
        }
	} else if ((name == R2_EVENT_VOICE_ONESHOT) ||
		(name == R2_EVENT_ASR_TMP_RESULT) ||
		(name == R2_EVENT_VOICE_ACCEPT)) {
		/* 暂无其他特殊处理，可以直接传给系统端 */
	} else if ((name == R2_EVENT_VOICE_REJECT) ||
		(name == R2_EVENT_VOICE_ERROR) ||
		(name == R2_EVENT_VOICE_FAKE) ||
		(name == R2_EVENT_VOICE_INVALID)) {
		speech_.set_pickup(false);
		/* 云端返回无效、拒绝或其他错误时，应该忽略
		 * 后面的NLP事件。
		 */
		speech_.setValidFlag(false);
	} else if (name == R2_EVENT_VOICE_CANCEL) {
		/* 当撤销当前语音后，也应该忽略后面的NLP事件，同时
		 * 系统组不关心该事件，不需要通知系统组。
		 */
		speech_.set_pickup(false);
		speech_.setValidFlag(false);
		return R2_EOK;
	} else if (name == R2_EVENT_VOICE_CLOUD_START) {
		return R2_EOK;
	} else {
		R2Error("Unknown speech event %s", name.c_str());
		return -R2_ENOTSUPPORT;
	}

	sendEvent(name, getCurrentId(), si->speech_id, si);

	return R2_EOK;
}

int R2SpeechEventHandler::onVoiceEvent(R2Event &event) {
	AwakeEvent ae;
	int seq = getCurrentId();
	std::string name = event.getName();
	R2VoiceInfo *vi = (R2VoiceInfo *)event.getArgs();
	int speech_id = -1;

	R2Info("speech valid: %d, %s",
		speech_.getValidFlag(), name.c_str());

	if (name == R2_EVENT_VOICE_LOCAL_AWAKE) {
		ae.version = 1;
		ae.cmd = 0;
		ae.sl = vi->sl;
		if (awake_server_->isConnected()) {
			awake_server_->onData((unsigned char *)&ae,
				sizeof(ae));
		}
	} else if (name == R2_EVENT_VOICE_SLEEP) {
		speech_.setValidFlag(false);
		seq = genId();
		speech_.endVoice();

		ae.version = 1;
		ae.cmd = 1;
		ae.sl = 0.0;
		if (awake_server_->isConnected()) {
			awake_server_->onData((unsigned char *)&ae,
				sizeof(ae));
		}
	} else if ((name == R2_EVENT_VOICE_COMING) ||
		(name == R2_EVENT_VOICE_COMING_CMD) ||
		(name == R2_EVENT_VOICE_COMING_NOCMD)) {
		seq = genId();
		speech_.setValidFlag(true);
	} else if ((name == R2_EVENT_VAD_BEGIN) ||
		(name == R2_EVENT_VAD_END)) {
		speech_id = speech_.voiceID();
	} else if (name == R2_EVENT_VOICE_LOCAL_START) {
		speech_id = speech_.startVoice(vi);
	} else {
		R2Error("Unknown voice event %s", name.c_str());
		return -R2_ENOTSUPPORT;
	}

	sendEvent(name, seq, speech_id, vi);

	return R2_EOK;
}

void R2SpeechEventHandler::sendEvent(const string &name,
		int seq, int speech_id, R2SpeechInfo *si) {
	int eventid = getEventIdMap()[name];
	int size = sizeof(int) * 5 + sizeof(char) +
		sizeof(float) * 3 + sizeof(short) * 5 +
		si->asr.length() + si->nlp.length() +
		si->action.length() + si->extra.length() + 5;
	R2Stream stream(size);

	stream.reset();
	stream.writeInt(0);
	stream.writeInt(0x76543210);
	stream.writeInt(seq);		/* Lothal ID */
	stream.writeInt(speech_id);	/* speechID */
	stream.writeChar(eventid);
	stream.writeFloat(0.f);
	stream.writeFloat(0.f);
	stream.writeFloat(0.f);
	stream.writeString(si->asr);
	stream.writeString(si->nlp);
	stream.writeString(si->action);
	stream.writeString(si->extra);
	stream.writeString("");
	stream.writeInt(si->error);
	stream.renewHeadLen();

	R2Notice("SendEvent <%d> %s(%d) (%dB), nlp: %s, asr: %s, "
		"action: %s, extra: %s, error: %d",
		si->speech_id, name.c_str(), eventid, size,
		si->nlp.c_str(), si->asr.c_str(), si->action.c_str(),
		si->extra.c_str(), si->error);

	if (event_server_->isConnected()) {
		event_server_->onData(stream.getBuffer(), stream.getWriteIdx());
	}
}

void R2SpeechEventHandler::sendEvent(const string &name,
		int seq, int speech_id, R2VoiceInfo *vi) {
	int eventid = getEventIdMap()[name];
	int size = sizeof(int) * 5 + sizeof(char) + sizeof(float) * 3 +
		sizeof(short) * 5 + vi->voice_trigger.length() + 5;
	R2Stream stream(size);

	stream.reset();
	stream.writeInt(0);
	stream.writeInt(0x76543210);
	stream.writeInt(seq);		/* Lothal ID */
	stream.writeInt(speech_id); /* speechID */
	stream.writeChar(eventid);
	stream.writeFloat(vi->sl);
	stream.writeFloat(vi->voice_energy);
	stream.writeFloat(vi->threshold_energy);
	stream.writeString("");
	stream.writeString("");
	stream.writeString("");
	stream.writeString("");
	stream.writeString(vi->voice_trigger);
	stream.writeInt(0);
	stream.renewHeadLen();

	R2Notice("SendEvent <%d> %s(%d) sl: %f, energy: %f, threshold: %f, "
		"trigger: %s, start: %d, len: %d, confirm: %d",
		speech_id, name.c_str(), eventid,
		vi->sl, vi->voice_energy, vi->threshold_energy,
		vi->voice_trigger.c_str(), vi->trigger_start,
		vi->trigger_length, vi->trigger_confirm);

	if (event_server_->isConnected()) {
		event_server_->onData(stream.getBuffer(), stream.getWriteIdx());
	}
}

int R2SpeechEventHandler::genId() {
	seq_++;
	seq_ &= 0x7Fffffff;
	return seq_;
}

int R2SpeechEventHandler::getCurrentId() {
	return seq_;
}

}

#endif

