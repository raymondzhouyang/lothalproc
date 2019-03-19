/*
 * DataProviderMic.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/15
 * Copyright © 2018 rokid. All rights reserved.
 */

#ifdef HAVE_MIC_ARRAY

#include <stdbool.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "R2Base.hpp"
#include "R2Log.hpp"

#include "R2Types.hpp"
#include "DataProvider.hpp"
#include "DataProviderMic.hpp"

using namespace std;
using namespace r2base;

DataProviderMic::DataProviderMic(int frame_size) :
		mic_buf_size_(frame_size) {
	openDevice();
}

DataProviderMic::~DataProviderMic() {
	R2Log("Stop stream");
	device->stop_stream(device);
	R2_DEL_AR1(buffer_);
}

int DataProviderMic::readBlock(const unsigned char* &data, int& len) {
	int ret;

#ifdef HAVE_VSP
	if (buffer_ == NULL) {
		buffer_ = R2_NEW_AR1(unsigned char,
				device->get_stream_buff_size(device));
	}

	ret = device->read_stream(device, (char*)buffer_, &mic_buf_size_);
#else
	if (buffer_ == NULL) {
		buffer_ = R2_NEW_AR1(unsigned char, mic_buf_size_);
	}
	ret = device->read_stream(device, (char*)buffer_, mic_buf_size_);
#endif
	if (ret != 0) {
		R2Error("Read stream failed: %d (Need adb root !)", ret);
		data = NULL;
		len = 0;
		return ret;
	}

	data = buffer_;
	len = mic_buf_size_;
	return R2_EOK;
}

const char* DataProviderMic::name() {
	return "MIC";
}

bool DataProviderMic::isOk() {
	return is_ok_;
}

int DataProviderMic::openDevice(const hw_module_t* module,
		struct mic_array_device_t** device) {
	return module->methods->open(module, MIC_ARRAY_HARDWARE_MODULE_ID,
					(struct hw_device_t **)device);
}

int DataProviderMic::openDevice() {
	int retry = 10;
	int ret;

	ret = hw_get_module(MIC_ARRAY_HARDWARE_MODULE_ID,
				(const struct hw_module_t **)&module);
	if (ret) {
		R2Error("Mic Array init (not found mic_array.xxx.so) : %d",
			ret);
		return ret;
	}

	R2Log("Open device ...");
	ret = openDevice(&module->common, &device);
	if (ret) {
		R2Error("Failed to open MIC device: %d", ret);
		return ret;
	}

	R2Log("Start stream...");
	do {
		ret = device->start_stream(device);
		if (ret == 0) {
			R2Notice("Start MIC stream -- OK");
			is_ok_ = true;
			break;
		}

		R2Notice("Failed to start MIC stream: %d."
			" Device may not ready, try later.", ret);
		sleep(1);
	} while (--retry);

	R2Log("channels=%d, sample_rate=%d, bit=%d, frame_cnt=%d, "
		"stream_buff_size:%d, frameSize:%d",
		device->channels, device->sample_rate, device->bit,
		device->frame_cnt, device->get_stream_buff_size(device),
		mic_buf_size_);
	return 0;
}

#endif

