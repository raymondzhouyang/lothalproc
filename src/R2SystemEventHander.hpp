/*
 * R2SystemEventHander.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/31
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifndef _R2SYSTEMEVENTHANDER_HPP_
#define _R2SYSTEMEVENTHANDER_HPP_

namespace lothal {

class R2SystemEventHander: public R2EventHandler {
public:
	R2SystemEventHander(R2Lothal &lothal, R2Client &client);
	~R2SystemEventHander();
	int onEvent(R2Event &event);

	static R2EventHandlerSp setupEventHandler(
		R2Lothal &lothal, R2Client &client);
	static void removeEventHandler(R2EventHandlerSp handler);

private:
	R2Lothal &lothal_;
	R2Client &client_;
	r2base::R2OutServer *voip_outserver_;
	bool isconnected_;
};


}

#endif /* _R2SYSTEMEVENTHANDER_HPP_ */

