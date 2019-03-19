/*
 * R2ProcServer.cpp
 * Lothal
 *
 * Created by jiqiang.fu on 2019/02/27
 * Copyright © 2019 rokid. All rights reserved.
 */

#include "R2OutServer.hpp"
#include "R2Lothal.hpp"
#include "R2ProcServer.hpp"
using namespace r2base;
namespace lothal {
R2ProcServer::R2ProcServer(int port, std::string name,
		                   R2Lothal &lothal):
		R2OutServer(port, name), lothal_(lothal) {
	    this->startServer();
}

R2ProcServer::~R2ProcServer(){
	exit = true;
}

void R2ProcServer::haveNewConnetion() {
	lothal_.setVoipStatus(true);
    return;
}

}

