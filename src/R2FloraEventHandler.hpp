/*
 * R2FloraEventHandler.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/2/12
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifdef HAVE_FLORA

#ifndef _R2FLORAEVENTHANDLER_HPP_
#define _R2FLORAEVENTHANDLER_HPP_

namespace lothal {

class R2FloraEventHandler: public R2EventHandler {
public:
	R2FloraEventHandler(R2Lothal &lothal, R2Flora &flora);
	~R2FloraEventHandler();

	int onEvent(R2Event &event);

	static R2EventHandlerSp setupEventHandler(
			R2Lothal &lothal,
			R2Flora &flora);
	static void removeEventHandler(R2EventHandlerSp handler);

private:
	R2Lothal &lothal_;
	R2Flora &flora_;
};

}

#endif /* _R2FLORAEVENTHANDLER_HPP_ */

#endif
