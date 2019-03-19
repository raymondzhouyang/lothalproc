/*
 * R2ProcServer.hpp
 * Lothal
 *
 * Created by jiqiang.fu on 2019/02/27
 * Copyright © 2019 rokid. All rights reserved.
 */

#ifndef R2PROCSERVER_HPP
#define R2PROCSERVER_HPP

namespace lothal {

class R2ProcServer: public R2OutServer {
public:
	R2ProcServer(int port, std::string name, R2Lothal &lothal);
	~R2ProcServer();
	void haveNewConnetion();

private:
	R2Lothal &lothal_;
};
}
#endif
