/*
 * R2Flora.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/2/12
 * Copyright © 2018 rokid. All rights reserved.
 */

#ifdef HAVE_FLORA

#include <stdio.h>
#include <assert.h>
#include <functional>
#include <chrono>
#include "R2Log.hpp"
#include "R2Lothal.hpp"
#include "R2Client.hpp"
#include "R2Flora.hpp"

using namespace std;
using namespace std::chrono;
using namespace r2base;

namespace lothal {

#define FLORA_ID		"lothalproc"

R2Flora::R2Flora(R2Lothal &lothal, const std::string &uri):
		lothal_(lothal) {
	flora_uri_ = uri;
	flora_uri_.append("#");
	flora_uri_.append(FLORA_ID);
	initMsgHandlerMap();
	client_thread_ = thread([this] () {
		this->clientReconnRoutine();
	});
}

R2Flora::~R2Flora() {
}

int R2Flora::startVoice(R2VoiceInfo* vi) {
	lock_guard <mutex > locker(id_mutex_);
	static R2VoiceInfo emptyOptions;
	shared_ptr <flora::Client> client;
	shared_ptr <Caps> msg;

	if (isSpeaking())
	    cancelVoice();

	is_speaking_ = true;

	getClient(client);
	if (client.get()) {
		if (vi == nullptr)
			vi = &emptyOptions;

		msg = Caps::new_instance();
		msg->write(vi->voice_trigger.c_str());
		msg->write((int32_t) vi->trigger_start);
		msg->write((int32_t) vi->trigger_length);
		msg->write(vi->voice_energy);
		msg->write((int32_t) vi->trigger_confirm);
		msg->write(vi->extra);
		msg->write(++voice_id_);

		R2Log("voice_power %f, voice extra: %s",
			vi->voice_energy, vi->extra.c_str());
		if (client->post("rokid.turen.start_voice",
				msg, FLORA_MSGTYPE_INSTANT) == FLORA_CLI_ECONN)
			clearFlora();
	}
	return voice_id_;
}

void R2Flora::putVoice(const uint8_t * data, uint32_t length) {
	shared_ptr <flora::Client> client;
	shared_ptr <Caps> msg;

	getClient(client);
	if (client.get()) {
		msg = Caps::new_instance();
		msg->write(data, length);
		msg->write(voice_id_);
		if (client->post("rokid.turen.voice",
			msg, FLORA_MSGTYPE_INSTANT) == FLORA_CLI_ECONN)
			clearFlora();
	}
}

/* 系统组需要接收 end_voice 消息以维护拾音状态 */
void R2Flora::endVoice() {
	shared_ptr <flora::Client> client;
	shared_ptr <Caps> msg;

	is_speaking_ = false;

	getClient(client);
	if (client.get()) {
		msg = Caps::new_instance();
		msg->write(voice_id_);
		if (client->post("rokid.turen.end_voice",
		    		msg, FLORA_MSGTYPE_INSTANT) == FLORA_CLI_ECONN)
			clearFlora();
	}
}

void R2Flora::cancelVoice() {
	shared_ptr <flora::Client> client;
	shared_ptr <Caps> msg;

	is_speaking_ = false;

	getClient(client);
	if (client.get()) {
		msg = Caps::new_instance();
		msg->write(voice_id_);
		if (client->post("rokid.turen.cancel_voice",
				msg, FLORA_MSGTYPE_INSTANT) == FLORA_CLI_ECONN)
			clearFlora();
	}
}

bool R2Flora::isSpeaking() {
	return is_speaking_;
}

int R2Flora::voiceID() {
	return voice_id_;
}

void R2Flora::getClient(shared_ptr <flora::Client> &res) {
	unique_lock <mutex> locker(client_mutex_);
	connectFlora(res, locker);
}

void R2Flora::recv_post(const char *name, uint32_t msgtype,
			shared_ptr<Caps> &msg) {
	MsgHandlerMap::iterator it;
	string key = name;

	it = msg_handlers_.find(key);
	assert(it != msg_handlers_.end());
	it->second(msg);
}

void R2Flora::disconnected() {
	thread tmp([this] () {
		   this->clearFlora();
	});
	tmp.detach();
}

void R2Flora::connectFlora(shared_ptr <flora::Client> &res,
			   unique_lock <mutex> &locker) {
	if (flora_client_.get()) {
		res = flora_client_;
		return;
	}

	if (flora::Client::connect(flora_uri_.c_str(),
			this, 81920,
			flora_client_) != FLORA_CLI_SUCCESS) {
		R2Notice("connect flora service %s failed",
			flora_uri_.c_str());
	}

	res = flora_client_;
	locker.unlock();
	if (res.get()) {
		MsgHandlerMap::iterator it;
		for (it = msg_handlers_.begin();
			it != msg_handlers_.end(); ++it) {
			res->subscribe(it->first.c_str());
		}
	}
	locker.lock();
	res = flora_client_;
}

void R2Flora::clearFlora() {
	client_mutex_.lock();
	flora_client_.reset();
	/* notify reconnect thread */
	client_cond_.notify_one();
	client_mutex_.unlock();
}

void R2Flora::clientReconnRoutine() {
	unique_lock <mutex> locker(client_mutex_);
	shared_ptr <flora::Client> client;

	while (true) {
		connectFlora(client, locker);

		if (client.get())
			client_cond_.wait(locker);
		else
			client_cond_.wait_for(locker, milliseconds(10000));
	}
}

/* Message handler */

void R2Flora::initMsgHandlerMap() {
	msg_handlers_.insert(make_pair("rokid.speech.completed",
				      [this] (shared_ptr <Caps> &msg) {
				      this->handleCompleted(msg);}));
	msg_handlers_.insert(make_pair("rokid.turen.pickup",
				      [this] (shared_ptr <Caps> &msg) {
				      this->handlePickup(msg);}));
	msg_handlers_.insert(make_pair("rokid.turen.mute",
				      [this] (shared_ptr <Caps> &msg) {
				      this->handleMute(msg);}));
	msg_handlers_.insert(make_pair("rokid.turen.addVtWord",
				      [this] (shared_ptr <Caps> &msg) {
				      this->handleAddWord(msg);}));
	msg_handlers_.insert(make_pair("rokid.turen.removeVtWord",
				      [this] (shared_ptr <Caps> &msg) {
				      this->handleRemoveWord(msg);}));
}

static bool caps_read_int32(shared_ptr<Caps> &caps, int32_t & r) {
	double dv;

	if (caps->read(r) != CAPS_SUCCESS) {
		if (caps->read(dv) != CAPS_SUCCESS)
			return false;
		r = (int32_t) dv;
	}
	return true;
}

void R2Flora::handleCompleted(shared_ptr<Caps> &msg) {
	int32_t id;

	if (msg->read(id) != CAPS_SUCCESS) {
		R2Notice("Failed to handle pickup: invalid pickup flag");
		return;
	}

	R2Info("handleCompleted: id %d/%d", id, voice_id_);

	id_mutex_.lock();
	if (id != voice_id_) {
		id_mutex_.unlock();
		return;
	}

	endVoice();
	id_mutex_.unlock();
}

void R2Flora::handlePickup(shared_ptr<Caps> &msg) {
	int32_t pick;

	if (!caps_read_int32(msg, pick)) {
		R2Notice("Failed to handle pickup: invalid pickup flag");
		return;
	}

	R2Info("handlePickup: %d", pick);

	id_mutex_.lock();
	if (pick) {
		lothal_.forceTrigger();
	} else {
		endVoice();
	}

	id_mutex_.unlock();
}

void R2Flora::handleMute(shared_ptr<Caps> &msg) {
	int32_t mute;

	if (!caps_read_int32(msg, mute)) {
		R2Notice("Failed to handle mute: invalid mute flag");
		return;
	}

	R2Info("handleMute: %d", mute);

	id_mutex_.lock();
	if (mute) {
		lothal_.pausePipeline();
	} else {
		lothal_.resumePipeline();
	}

	id_mutex_.unlock();
}

void R2Flora::handleAddWord(shared_ptr<Caps> &msg) {
	string word;
	string pinyin;
	int32_t type;

	if (msg->read_string(word) != CAPS_SUCCESS) {
		R2Notice("Failed to add word: invalid word");
		return;
	}

	if (msg->read_string(pinyin) != CAPS_SUCCESS) {
		R2Notice("Failed to add word: invalid pinyin");
		return;
	}

	if (!caps_read_int32(msg, type)) {
		R2Notice("Failed to add word: invalid type");
		return;
	}

	lothal_.addWord(word, pinyin, type);
}

void R2Flora::handleRemoveWord(shared_ptr<Caps> &msg) {
	string word;

	if (msg->read_string(word) != CAPS_SUCCESS) {
		R2Notice("Failed to remove word: invalid msg.");
		return;
	}

	lothal_.removeWord(word);
}

}

#endif

