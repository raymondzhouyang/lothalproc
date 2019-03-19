/*
 * R2EventExecutor.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/13
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifndef _R2EVENTEXECUTOR_HPP_
#define _R2EVENTEXECUTOR_HPP_

namespace lothal {

class R2EventExecutor {
public:
	R2EventExecutor(R2Lothal &lothal, R2Client &client);
	~R2EventExecutor();
private:

#ifdef HAVE_FLORA
	R2EventHandlerSp flora_evt_handler_;
#else
	R2EventHandlerSp speech_evt_handler_;
#endif
	R2EventHandlerSp system_evt_handler_;
};

}

#endif /* _R2EVENTEXECUTOR_HPP_ */

