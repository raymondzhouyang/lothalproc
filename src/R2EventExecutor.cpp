/*
 * R2EventExecutor.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/31
 * Copyright © 2018 rokid. All rights reserved.
 */
#include "R2OutServer.hpp"
#include "R2Lothal.hpp"
#include "R2Client.hpp"
#ifdef HAVE_FLORA
#include "R2Flora.hpp"
#include "R2FloraEventHandler.hpp"
#else
#include "R2Speech.hpp"
#include "R2SpeechEventHandler.hpp"
#endif
#include "R2SystemEventHander.hpp"
#include "R2EventExecutor.hpp"

using namespace std;
using namespace r2base;

namespace lothal {

R2EventExecutor::R2EventExecutor(R2Lothal &lothal, R2Client &client) {
	system_evt_handler_ =
		R2SystemEventHander::setupEventHandler(lothal, client);

#ifdef HAVE_FLORA
	flora_evt_handler_ =
		R2FloraEventHandler::setupEventHandler(lothal,
			dynamic_cast<R2Flora &>(client));
#else
	speech_evt_handler_ =
		R2SpeechEventHandler::setupEventHandler(lothal,
			dynamic_cast<R2Speech &>(client));
#endif
}

R2EventExecutor::~R2EventExecutor() {

#ifdef HAVE_FLORA
	R2FloraEventHandler::removeEventHandler(flora_evt_handler_);
#else
	R2SpeechEventHandler::removeEventHandler(speech_evt_handler_);
#endif
	R2SystemEventHander::removeEventHandler(system_evt_handler_);
}

}

