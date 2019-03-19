/*
 * R2Flora.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/2/12
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifdef HAVE_FLORA

#ifndef _R2FLORA_HPP_
#define _R2FLORA_HPP_

#include <map>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "flora-cli.h"

namespace lothal {

typedef std::map <std::string,
		std::function <void (std::shared_ptr<Caps> &)>> MsgHandlerMap;

class R2Flora: public R2Client,
	public flora::ClientCallback {
public:
	R2Flora(R2Lothal &lothal, const std::string &uri);
	~R2Flora();

public:
	int startVoice(R2VoiceInfo* vi = NULL);
	void putVoice(const uint8_t *data, uint32_t length);
	void endVoice();
	void cancelVoice();

	bool isSpeaking();
	int voiceID();

	void getClient(std::shared_ptr<flora::Client> &res);
	void clearFlora();

	/* implementation of flora::ClientCallback */
	void recv_post(const char *name, uint32_t msgtype,
		       std::shared_ptr<Caps> &msg);
	void disconnected();

private:
	void initMsgHandlerMap();
	void clientReconnRoutine();

	void connectFlora(std::shared_ptr<flora::Client> &res,
			  std::unique_lock<std::mutex> &locker);

	/* msg handlers */
	void handleCompleted(std::shared_ptr<Caps> &msg);
	void handlePickup(std::shared_ptr<Caps> &msg);
	void handleMute(std::shared_ptr<Caps> &msg);
	void handleAddWord(std::shared_ptr<Caps> &msg);
	void handleRemoveWord(std::shared_ptr<Caps> &msg);

private:
	R2Lothal &lothal_;

	int32_t voice_id_ = 0;
	std::mutex id_mutex_;

	std::shared_ptr<flora::Client> flora_client_;
	std::mutex client_mutex_;
	std::condition_variable client_cond_;
	std::thread client_thread_;

	std::string flora_uri_;
	MsgHandlerMap msg_handlers_;
	volatile bool is_speaking_ = false;
};

}

#endif /* _R2FLORA_HPP_ */
#endif

