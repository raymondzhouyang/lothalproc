/*
 * LothalProc.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/11
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifndef _LOTHALPROC_HPP_
#define _LOTHALPROC_HPP_

namespace lothal {
class LothalProc {
public:
	LothalProc(R2Lothal* lothal);
	~LothalProc();

public:
	void setProvider(DataProvider *provider);
	int run();
public:
	void process();

private:
	R2Lothal* lothal_;
	R2FrameQueue<R2Data> queue_;
	DataProvider* provider_ = NULL;
	volatile bool exit_ = false;
	volatile bool running_ = false;
	pthread_t proc_thread_;
};
}

#endif /* _LOTHALPROC_HPP_ */

