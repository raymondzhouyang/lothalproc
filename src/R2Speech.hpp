/*
 * R2Speech.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/12
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifndef HAVE_FLORA

#ifndef _R2SPEECH_HPP_
#define _R2SPEECH_HPP_

#include <string>
#include <thread>

#include "speech/speech.h"

namespace lothal {

class R2Speech: public R2Client {
public:
	R2Speech(R2Lothal &lothal, bool codec);
	~R2Speech();

public:
	void run();

	void restart(R2ConfigOption* opions = NULL);
	int startVoice(R2VoiceInfo* vi = NULL);
	void putVoice(const uint8_t* data, uint32_t length);
	void endVoice();
	void cancelVoice();

	bool isSpeaking();
	int voiceID();

	void setValidFlag(bool flag);
	bool getValidFlag();

	void setStack(const std::string &stack);
	void setSkill(const std::string &skill);
	void handle_pickup(int pick);

private:
	void speechLooper();
	void onSpeechResult(rokid::speech::SpeechResult &r);

private:
	std::shared_ptr<rokid::speech::Speech> _speech;
	rokid::speech::PrepareOptions prepare_opts_;
	std::thread speech_thread_;
	R2Lothal &lothal_;

	volatile bool exit_ = false;
	bool codec_;

	bool isvalid_ = true;
	bool ShowVoicePower = false;
	int voice_id_ = 0;

	std::string stack_;
	std::string skill_opts_;
	std::mutex  mutex_;
};

}

#endif /* _R2SPEECH_HPP_ */

#endif
