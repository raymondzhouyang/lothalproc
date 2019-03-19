/*
 * R2Client.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/2/12
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifndef _R2CLIENT_HPP_
#define _R2CLIENT_HPP_

#include <string>
#include <atomic>
#include <memory>

namespace lothal {

class R2Client {
public:
	R2Client() : pickup_(false) {};
	virtual ~R2Client() {};

public:
	virtual void restart(R2ConfigOption* opions = NULL) {};
	virtual int startVoice(R2VoiceInfo* vi = NULL) {
		return R2_EOK;
	};

	virtual void putVoice(const uint8_t* data, uint32_t length) {};
	virtual void endVoice() {};
	virtual void cancelVoice() {};

	virtual bool isSpeaking() {
		return true;
	};
	virtual int voiceID() {return 0;};

	virtual void setValidFlag(bool flag) {};
	virtual bool getValidFlag() {
		return true;
	};

	virtual void setStack(const std::string &stack) {};
	virtual void setSkill(const std::string &skill) {};
	virtual void set_pickup(bool enable) { pickup_ = enable; }
	virtual bool is_pickup() { return pickup_; }
private:
    std::atomic<bool> pickup_;
};

}

#endif /* _R2CLIENT_HPP_ */

