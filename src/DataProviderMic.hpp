/*
 * DataProviderMic.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/15
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifdef HAVE_MIC_ARRAY

#ifndef _DATAPROVIDERMIC_HPP_
#define _DATAPROVIDERMIC_HPP_

#if defined(__ANDROID__) || defined(ANDROID) ||	\
	defined(__APPLE__) || defined(HAVE_VSP)
#include "r2hw/mic_array.h"
#else
#if defined (__arm__) || defined(__aarch64__)
#include <hardware/mic_array.h>
#endif
#endif

class DataProviderMic: public DataProvider {
public:
	DataProviderMic(int frame_size);
	~DataProviderMic();

	int readBlock(const unsigned char* &data, int& len);
	bool isOk();
	const char* name();

private:
	int openDevice(const hw_module_t* module,
			struct mic_array_device_t** device);
	int openDevice();

private:
	struct mic_array_module_t* module = NULL;
	struct mic_array_device_t* device = NULL;
	int mic_buf_size_;
	unsigned char* buffer_ = NULL;
	bool is_ok_ = false;
};

#endif /* _DATAPROVIDERMIC_HPP_ */

#endif

