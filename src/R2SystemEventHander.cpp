/*
 * R2SystemEventHander.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/31
 * Copyright © 2018 rokid. All rights reserved.
 */
#include "R2Base.hpp"
#include "R2OutServer.hpp"
#include "R2Lothal.hpp"
#include "R2Client.hpp"
#include "R2SystemEventHander.hpp"
#include "R2ProcServer.hpp"

using namespace std;
using namespace r2base;

namespace lothal {

R2SystemEventHander::R2SystemEventHander(R2Lothal &lothal, R2Client &client):
		lothal_(lothal), client_(client) {
	voip_outserver_ = R2_NEW(R2ProcServer, R2_BASEPORT+17,
                            "voip(read)", lothal_);
	isconnected_ = false;
}

R2SystemEventHander::~R2SystemEventHander() {
	R2_DEL(voip_outserver_);
}

static R2EventIdMap getEventIdMap() {
	R2EventIdMap map;

	map[R2_EVENT_DATA_OUT] = 0;
	map[R2_EVENT_RECONFIG] = 0;
	map[R2_EVENT_SETSTACK] = 0;
	map[R2_EVENT_SETSKILL] = 0;
	map[R2_EVENT_OFFLINE_ASR] = 0;
	map[R2_EVENT_PICKUP] = 0;
	map[R2_EVENT_DATA_VOIP] = 0;

	map[R2_EVENT_NONE] = 0;
	map[R2_EVENT_SYS_EXIT] = 90;
	map[R2_EVENT_OUTSERVER_READY] = 100;

	return map;
}

R2EventHandlerSp R2SystemEventHander::setupEventHandler(
		R2Lothal &lothal, R2Client &client) {
	R2EventIdMap map;
	R2EventIdMapIter iter;
	R2EventHandlerSp handler;
	string name;

	map = getEventIdMap();

	handler = make_shared<R2SystemEventHander>(lothal, client);

	for (iter = map.begin(); iter != map.end(); iter++) {
		name = iter->first;
		lothal.setEventHandler(name, handler);
	}

	return handler;
}

void R2SystemEventHander::removeEventHandler(R2EventHandlerSp handler) {
	string name;
	R2EventIdMap map;
	R2EventIdMapIter iter;
	std::shared_ptr<R2SystemEventHander> d_ptr =
			std::dynamic_pointer_cast<R2SystemEventHander>(handler);
	R2Lothal &lothal = d_ptr->lothal_;

	map = getEventIdMap();
	for (iter = map.begin(); iter != map.end(); iter++) {
		name = iter->first;
		lothal.removeEventHandler(name, handler);
	}
}

int R2SystemEventHander::onEvent(R2Event &event) {
	std::string name = event.getName();

	if (name == R2_EVENT_RECONFIG) {
		R2ConfigOption *co = (R2ConfigOption *)event.getArgs();

		R2Log("<%s>: %s, %d, %s, %s, %s, %s, %s",
			name.c_str(), co->host.c_str(), co->port,
			co->branch.c_str(), co->key.c_str(), co->secret.c_str(),
			co->device_type_id.c_str(), co->device_id.c_str());

		client_.restart(co);
		return R2_EOK;
	}

	if (name == R2_EVENT_SETSKILL) {
		string *s = (string *)event.getArgs();
		R2Verbose("<%s>: %s", name.c_str(), (*s).c_str());
		client_.setSkill(*s);
		return R2_EOK;
	}

	if (name == R2_EVENT_SETSTACK) {
		string *s = (string *)event.getArgs();
		R2Log("<%s>: %s", name.c_str(), (*s).c_str());
		client_.setStack(*s);
		return R2_EOK;
	}

	if (name == R2_EVENT_PICKUP) {
		bool *s = (bool *)event.getArgs();
		R2Log("<%s>: %d", name.c_str(), (*s));
		client_.set_pickup(*s);
		return R2_EOK;
	}

	if (name == R2_EVENT_DATA_OUT) {
		R2PutDataInfo *datainfo = (R2PutDataInfo *)event.getArgs();

		if (client_.is_pickup() && !client_.isSpeaking()) {
		    R2Log("start voice from pickup");
		    client_.startVoice(NULL);
        }

		if (client_.isSpeaking()) {
			R2Verbose("put voice: %p, len: %d",
				datainfo->data, datainfo->len);
			client_.putVoice(datainfo->data, datainfo->len);
		}
		return R2_EOK;
	}

	if (name == R2_EVENT_OFFLINE_ASR) {
		R2OfflineAsrInfo *datainfo = (R2OfflineAsrInfo *)event.getArgs();
		// TODO handle offline asr event
		R2Log("[EVENT] offline asr:%s", datainfo->asr_result.c_str());
		return R2_EOK;
	}

	if (name == R2_EVENT_DATA_VOIP) {
        if (voip_outserver_ && voip_outserver_->isConnected()) {
        	if (!isconnected_) {
        		isconnected_ = true;
        	}
        	R2PutDataInfo *datainfo =
        			  (R2PutDataInfo *)event.getArgs();
        	voip_outserver_->onData(datainfo->data, datainfo->len);
        } else {
        	if (isconnected_) {
        		isconnected_ = false;
            	lothal_.setVoipStatus(isconnected_, "");
        	}
        }
        return R2_EOK;
	}

	return -R2_ENOTSUPPORT;
}

}

