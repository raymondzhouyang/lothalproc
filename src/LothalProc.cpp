/*
 * LothalProc.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/11
 * Copyright © 2018 rokid. All rights reserved.
 */
#include <errno.h>
#include <sys/prctl.h>
#include "R2Base.hpp"
#include "R2Util.hpp"
#include "R2FrameQueue.hpp"
#include "R2Data.hpp"
#include "R2Lothal.hpp"
#include "DataProvider.hpp"
#include "LothalProc.hpp"

using namespace std;
using namespace r2base;

namespace lothal {

static void* doLothalProc(void* arg) {
	LothalProc* lothalproc = (LothalProc*) arg;
	prctl(PR_SET_NAME, "thread: process");
	lothalproc->process();
	return NULL;
}

LothalProc::LothalProc(R2Lothal* lothal) :
		lothal_(lothal), queue_("MicArray"),
		exit_(false), proc_thread_(0) {
	queue_.setIfFullWait(true);
	queue_.setMinNoticeCount(3);
	queue_.setQueueMonitor(true);
	R2Util::StartUpThread(proc_thread_, doLothalProc, this, false);
}

LothalProc::~LothalProc() {
	R2Data data;
	exit_ = true;
	queue_.addFrame(data);
    while (running_) {
        usleep(5000);
    }
	pthread_join(proc_thread_, NULL);
}

int LothalProc::run() {
	int data_size;
	int block_size;
	const unsigned char* data = NULL;
	int ret = 0;

    running_ = true;

	if (!provider_) {
		R2Error("Has no data provider");
		return -R2_ESYSTEM;
	}

	lothal_->startPipeline();
	block_size = lothal_->getDataBlockSize();
	while (!r2_exit() && !exit_) {
		if (!lothal_->isReady()) {
			usleep(2000);
			continue;
		}

		data_size = block_size;
		ret = provider_->readBlock(data, data_size);
		if (ret) {
			R2Error("Failed to read block, ret: %d", ret);
			#ifdef HAVE_VSP
				//vspfeed or hw vad maybe in this mode
				continue;
			#else
				break;
			#endif
		}

		if (data && (data_size == block_size)) {
			R2Data item((unsigned char*)data, data_size);
			queue_.addFrame(item);
			continue;
		}

		R2Error("size not equal: %d, %d", data_size, block_size);
	}
	lothal_->stopPipeline();

	r2_setExit(true);
    running_ = false;
	return R2_EOK;
}

void LothalProc::process() {
	R2Frame<R2Data>* frame;
	R2Data* data;

	while (!exit_ && lothal_) {
		frame = queue_.getFrame();
		if (!frame)
			continue;

		data = &frame->data;
		if (data->size)
			lothal_->process(data->data, data->size);
		queue_.deleteFrame(frame);
	}
}

void LothalProc::setProvider(DataProvider *provider) {
	provider_ = provider;
}

}

