/*
 * R2FloraEventHandler.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/2/12
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifdef HAVE_FLORA

#include "R2Base.hpp"
#include "R2Lothal.hpp"
#include "R2Client.hpp"
#include "R2Flora.hpp"
#include "R2FloraEventHandler.hpp"

using namespace std;
using namespace r2base;

namespace lothal {

R2FloraEventHandler::R2FloraEventHandler(R2Lothal &lothal, R2Flora &flora):
		lothal_(lothal), flora_(flora) {
}

R2FloraEventHandler::~R2FloraEventHandler() {

}

static R2EventIdMap getEventIdMap() {
	R2EventIdMap map;

	map[R2_EVENT_VOICE_COMING] = 10;
	map[R2_EVENT_VOICE_LOCAL_AWAKE] = 20;
	map[R2_EVENT_VOICE_LOCAL_START] = 30;
	map[R2_EVENT_VOICE_SLEEP] = 70;

	return map;
}

R2EventHandlerSp R2FloraEventHandler::setupEventHandler(
		R2Lothal &lothal, R2Flora &flora) {
	R2EventIdMap map;
	R2EventIdMapIter iter;
	R2EventHandlerSp handler;
	string name;

	map = getEventIdMap();

	handler = make_shared<R2FloraEventHandler>(lothal, flora);

	for (iter = map.begin(); iter != map.end(); iter++) {
		name = iter->first;
		lothal.setEventHandler(name, handler);
	}

	return handler;
}

void R2FloraEventHandler::removeEventHandler(R2EventHandlerSp handler) {
	string name;
	R2EventIdMap map;
	R2EventIdMapIter iter;
	std::shared_ptr<R2FloraEventHandler> d_ptr =
		std::dynamic_pointer_cast<R2FloraEventHandler>(handler);
	R2Lothal &lothal = d_ptr->lothal_;

	map = getEventIdMap();
	for (iter = map.begin(); iter != map.end(); iter++) {
		name = iter->first;
		lothal.removeEventHandler(name, handler);
	}
}

int R2FloraEventHandler::onEvent(R2Event &event) {
	string name = event.getName();
	shared_ptr<flora::Client> client;
	shared_ptr<Caps> msg;
	R2VoiceInfo* vi = (R2VoiceInfo *)event.getArgs();

	R2Info("%s <%d> sl: %f, energy: %f, threshold: %f, trigger: %s",
		name.c_str(), flora_.voiceID(), vi->sl, vi->voice_energy,
		vi->threshold_energy, vi->voice_trigger.c_str());

	if (name == R2_EVENT_VOICE_LOCAL_AWAKE) {
		flora_.getClient(client);
		if (client.get()) {
			msg = Caps::new_instance();
			msg->write(vi->sl);
			if (client->post("rokid.turen.local_awake", msg,
				      FLORA_MSGTYPE_INSTANT) == FLORA_CLI_ECONN)
				flora_.clearFlora();
		}
	} else if (name == R2_EVENT_VOICE_SLEEP) {
		flora_.endVoice();
		flora_.getClient(client);
		if (client.get()) {
			if (client->post("rokid.turen.sleep", msg,
				      FLORA_MSGTYPE_INSTANT) == FLORA_CLI_ECONN)
				flora_.clearFlora();
		}
	} else if (name == R2_EVENT_VOICE_COMING) {
		flora_.getClient(client);
		if (client.get()) {
			msg = Caps::new_instance();
			msg->write(vi->sl);
			if (client->post("rokid.turen.voice_coming", msg,
				      FLORA_MSGTYPE_INSTANT) == FLORA_CLI_ECONN)
				flora_.clearFlora();
		}
	} else if (name == R2_EVENT_VOICE_LOCAL_START) {
		flora_.startVoice(vi);
	} else {
		R2Error("Cannot handle event %s", name.c_str());
		return -R2_ENOTSUPPORT;
	}

	return R2_EOK;
}

}

#endif
