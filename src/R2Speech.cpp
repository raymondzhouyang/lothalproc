/*
 * R2Speech.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/12
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifndef HAVE_FLORA

#include <string>
#include <thread>
#include <sys/prctl.h>

#include "speech/speech.h"
#include "R2Log.hpp"
#include "R2Base.hpp"

#include "R2Lothal.hpp"

#include "R2Client.hpp"
#include "R2Speech.hpp"

using namespace std;
using namespace r2base;
using namespace rokid::speech;

namespace lothal {

static void ThreadMain(R2Speech* speechimpl) {
	prctl(PR_SET_NAME, "R2Speech: run");
	speechimpl->run();
}

R2Speech::R2Speech(R2Lothal &lothal, bool codec) :
		speech_thread_(ThreadMain, this),
		lothal_(lothal), codec_(codec) {

#if defined(__ANDROID__) || defined(ANDROID) || defined(__arm__) || \
	defined(__ARM__) || defined(__aarch64__) || defined(__ARM_ARCH)
#else
	prepare_opts_.host = "apigwws.open.rokid.com";
	prepare_opts_.port = 443;
	prepare_opts_.branch = "/api";
	prepare_opts_.key = "rokid_test_key";
	prepare_opts_.device_type_id = "rokid_test_device_type_id";
	prepare_opts_.device_id = "rokid.lothal.tester";
	prepare_opts_.secret = "rokid_test_secret";
#endif
}

R2Speech::~R2Speech() {
	exit_ = true;
	if (_speech)
		_speech->release();
	speech_thread_.join();
}

void R2Speech::restart(R2ConfigOption* options) {
	if (_speech) {
		R2Info("speech has been inited, don't need to reinit.");
		return;
	}

	if (options) {
		prepare_opts_.host = options->host;
		prepare_opts_.port = options->port;
		prepare_opts_.branch = options->branch;
		prepare_opts_.key = options->key;
		prepare_opts_.device_type_id = options->device_type_id;
		prepare_opts_.secret = options->secret;
		prepare_opts_.device_id = options->device_id;
	}
}

void R2Speech::run() {
	for (; !exit_;) {
		speechLooper();
	}
}

void R2Speech::speechLooper() {
	R2Log("Starting speech, waiting for options...");

	for (; !exit_;) {
		if (prepare_opts_.device_id.length() > 0 &&
			prepare_opts_.device_type_id.length() > 0 &&
			prepare_opts_.key.length() > 0 &&
			prepare_opts_.secret.length() > 0) {
			break;
		}
		usleep(1000 * 100);
	}

	if (!exit_) {
		shared_ptr<SpeechOptions> option =
			SpeechOptions::new_instance();
		shared_ptr<Speech> speech = Speech::new_instance();

		if (codec_) {
			option->set_codec(Codec::OPU);
			R2Log("⭕  Speech Codec::OPU");
		} else {
			option->set_codec(Codec::PCM);
			R2Log("⭕  Speech Codec::PCM");
		}

        //TODO add set_no_nlp option config
        //option->set_no_nlp(false);
		option->set_vad_mode(VadMode::CLOUD, 700);
		option->set_vad_begin(-1);
		option->set_log_server("0.0.0.0", 47033);

		speech->prepare(prepare_opts_);
		speech->config(option);
		_speech = speech;

		R2Log("Speech service started");
	}

	while (!exit_) {
		SpeechResult speech_res = { 0 };
		if (_speech && !_speech->poll(speech_res)) {
			break;
		}

		onSpeechResult(speech_res);
	}

	_speech = NULL;
	R2Log("Speech service stopped");
}

/* 云端语音识别结果映射为对应的语音事件 */
void R2Speech::onSpeechResult(SpeechResult &r) {
	R2SpeechInfo res = {0};

	res.speech_id = r.id;
	res.error = r.err;
	res.asr = r.asr;
	res.nlp = r.nlp;
	res.action = r.action;
	res.extra = r.extra;

	/* 将 @SpeechResultType 翻译成对应的事件 */
	switch (r.type) {
	case SPEECH_RES_START:
		lothal_.reportEvent(R2_EVENT_VOICE_CLOUD_START, &res);
		break;
	case SPEECH_RES_END:
		lothal_.reportEvent(R2_EVENT_VOICE_NLP, &res);
		break;
	case SPEECH_RES_CANCELLED:
		if (r.id < voice_id_) {
			R2Log("Skip cancel event. current id %d event id %d",
				voice_id_, r.id);
		} else {
			lothal_.reportEvent(R2_EVENT_VOICE_CANCEL, &res);
		}
		break;
	case SPEECH_RES_ERROR:
		lothal_.reportEvent(R2_EVENT_VOICE_ERROR, &res);
		break;
	case SPEECH_RES_INTER:
		static SpeechResult o = { 0 };
		if (o.id == r.id && o.asr == r.asr
			&& o.nlp == r.nlp && o.action == r.action
			&& o.extra == r.extra && o.err == r.err
			&& o.type == r.type) {
			/* 忽略相同的result */
			break;
		}

		o = r;
		if (r2_find(r.extra, "\"activation\":\"accept\"") > 0) {
			lothal_.reportEvent(R2_EVENT_VOICE_ACCEPT, &res);
			break;
		}

		if (r2_find(r.extra, "\"activation\":\"reject\"") > 0) {
			lothal_.reportEvent(R2_EVENT_VOICE_REJECT, &res);
			break;
		}

		if (r2_find(r.extra, "\"activation\":\"fake\"") > 0) {
			lothal_.reportEvent(R2_EVENT_VOICE_FAKE, &res);
			break;
		}

		if (r2_find(r.extra, "\"activation\":\"invalidate\"") > 0) {
			lothal_.reportEvent(R2_EVENT_VOICE_INVALID, &res);
			break;
		}

		if (r2_find(r.extra, "\"oneshot\":\"false\"") > 0) {
			lothal_.reportEvent(R2_EVENT_VOICE_ONESHOT, &res);
			break;
		}

		lothal_.reportEvent(R2_EVENT_ASR_TMP_RESULT, &res);
		break;
	case SPEECH_RES_ASR_FINISH:
		lothal_.reportEvent(R2_EVENT_ASR_END_RESULT, &res);
		break;
	default:
		R2Error("Unknown speech result");
		break;
	}
}

int R2Speech::startVoice(R2VoiceInfo* vi) {
	rokid::speech::VoiceOptions opts;

	if (!_speech) {
		R2Error("speech service not get ready yet.");
		return -1;
	}

	if (isSpeaking()) {
		cancelVoice();
	}

	if (vi) {
		opts.stack = stack_;
		opts.skill_options = skill_opts_;
		opts.voice_trigger = vi->voice_trigger;
		opts.trigger_start = vi->trigger_start;
		opts.trigger_length = vi->trigger_length;
		opts.trigger_confirm_by_cloud = vi->trigger_confirm;
		opts.voice_extra = vi->extra;

		if (ShowVoicePower)
			opts.voice_power = vi->voice_energy;

		R2Log("voice_power %f, voice extra: %s",
			opts.voice_power, opts.voice_extra.c_str());

		voice_id_ = _speech->start_voice(&opts);

		R2Log("startVoice() finished, get voice id: %d", voice_id_);
		return voice_id_;
	}

	voice_id_ = _speech->start_voice();
	R2Log("startVoice() finished, get voice id: %d", voice_id_);
	return voice_id_;
}

void R2Speech::putVoice(const uint8_t* data, uint32_t length) {
	if (!_speech) {
		R2Error("speech service not ready");
		return;
	}

	if (voice_id_)
		_speech->put_voice(voice_id_, data, length);

}

void R2Speech::endVoice() {
	if (!_speech) {
		R2Error("speech service not ready");
		return;
	}

	R2Log("R2Speech.endVoice(%d)", voice_id_);
	_speech->end_voice(voice_id_);
	voice_id_ = 0;
}

void R2Speech::cancelVoice() {
	if (!_speech) {
		R2Error("speech service not ready");
		return;
	}

	R2Log("R2Speech.cancelVoice(%d)", voice_id_);
	_speech->cancel(voice_id_);
	voice_id_ = 0;
}

bool R2Speech::isSpeaking() {
	return voice_id_ != 0;
}

void R2Speech::setValidFlag(bool flag) {
	isvalid_ = flag;
}

bool R2Speech::getValidFlag() {
	return isvalid_;
}

int R2Speech::voiceID() {
	return voice_id_;
}

void R2Speech::setStack(const string &stack) {
	stack_ = stack;
}

void R2Speech::setSkill(const string &skill) {
	skill_opts_ = skill;
}

}

#endif
