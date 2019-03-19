/*
 * DataProviderAlsa.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/22
 * Copyright © 2018 rokid. All rights reserved.
 */

#ifdef HAVE_ALSA

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "R2Log.hpp"

#include "R2Types.hpp"
#include "DataProvider.hpp"
#include "DataProviderAlsa.hpp"

using namespace std;
using namespace r2base;

static snd_output_t *log;

DataProviderAlsa::DataProviderAlsa() {
	snd_pcm_info_t *info;
	int err;
	const char *pcm_name = "default";

	err = snd_output_stdio_attach(&log, stderr, 0);
	assert(err >= 0);

	err = snd_pcm_open(&handle, pcm_name, stream, open_mode);
	if (err < 0) {
		R2Error("audio open error: %s", snd_strerror(err));
		assert(0);
	}

	snd_pcm_info_alloca(&info);

	if ((err = snd_pcm_info(handle, info)) < 0) {
		R2Error("info error: %s", snd_strerror(err));
		assert(0);
	}

	err = set_params();
	assert(err == 0);
}

DataProviderAlsa::~DataProviderAlsa() {
	if (handle)
		snd_pcm_close(handle);
	if (audiobuf_)
		free(audiobuf_);
}

int DataProviderAlsa::readBlock(const unsigned char* &data, int& len) {
	int r;
	size_t nleft = len;
	size_t nread = 0;
	unsigned char *p = NULL;
	unsigned int chunk_bytes = chunk_size_ * bits_per_frame_ / 8;

	if ((!audiobuf_) || (audiobuf_size_ < len)) {
		audiobuf_size_ = len;
		audiobuf_ = (unsigned char *)realloc(audiobuf_,
					audiobuf_size_);
		if (audiobuf_ == NULL) {
			R2Error("not enough memory");
			return -R2_ENOMEMORY;
		}
	}

	p = audiobuf_;
	while (nleft > 0) {
		size_t c = (nleft <= chunk_bytes) ? nleft : chunk_bytes;
		size_t f = c * 8 / bits_per_frame_;

		r = snd_pcm_readi(handle, p, f);
		if (r <= 0)
			break;

		r *= bits_per_frame_ / 8;

		nread += r;
		p += r;
		nleft -= r;
	}

	data = audiobuf_;
	len = nread;

	return R2_EOK;
}

bool DataProviderAlsa::isOk() {
	return true;
}

const char* DataProviderAlsa::name() {
	return "ALSA";
}

int DataProviderAlsa::set_params(void) {
	int err;
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);

	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		R2Error("Broken configuration for this PCM: "
			"no configurations available");
		return -R2_ESYSTEM;
	}

	if (interleaved)
		err = snd_pcm_hw_params_set_access(handle, params,
				   SND_PCM_ACCESS_RW_INTERLEAVED);
	else
		err = snd_pcm_hw_params_set_access(handle, params,
				   SND_PCM_ACCESS_RW_NONINTERLEAVED);
	if (err < 0) {
		R2Error("Access type not available");
		return -R2_ESYSTEM;
	}

	err = snd_pcm_hw_params_set_format(handle, params, format_);
	if (err < 0) {
		R2Error("Sample format non available");
		return -R2_ESYSTEM;
	}

	err = snd_pcm_hw_params_set_channels(handle, params, channels_);
	if (err < 0) {
		R2Error("Channels count non available");
		return -R2_ESYSTEM;
	}

	err = snd_pcm_hw_params_set_rate_near(handle, params, &rate_, 0);
	assert(err >= 0);

	if ((buffer_time_ == 0) && (buffer_frames_ == 0)) {
		err = snd_pcm_hw_params_get_buffer_time_max(params,
					    &buffer_time_, 0);
		assert(err >= 0);
		if (buffer_time_ > 500000)
			buffer_time_ = 500000;
	}

	if ((period_time_ == 0) && (period_frames_ == 0)) {
		if (buffer_time_ > 0)
			period_time_ = buffer_time_ / 4;
		else
			period_frames_ = buffer_frames_ / 4;
	}

	if (period_time_ > 0)
		err = snd_pcm_hw_params_set_period_time_near(handle, params,
					     &period_time_, 0);
	else
		err = snd_pcm_hw_params_set_period_size_near(handle, params,
					     &period_frames_, 0);
	assert(err >= 0);

	if (buffer_time_ > 0) {
		err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
					     &buffer_time_, 0);
	} else {
		err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
					     &buffer_frames_);
	}
	assert(err >= 0);

	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		R2Error("Unable to install hw params:");
		snd_pcm_hw_params_dump(params, log);
		return -R2_ESYSTEM;
	}

	snd_pcm_hw_params_get_period_size(params, &chunk_size_, 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (chunk_size_ == buffer_size) {
		R2Error("Can't use period equal to buffer size (%lu == %lu)",
		      chunk_size_, buffer_size);
		return -R2_ESYSTEM;
	}

	snd_pcm_sw_params_current(handle, swparams);

	err = snd_pcm_sw_params_set_avail_min(handle, swparams, chunk_size_);
	assert(err >= 0);

	/* round up to closest transfer boundary */
	err = snd_pcm_sw_params_set_start_threshold(handle,
			swparams, buffer_size);
	assert(err >= 0);

	err = snd_pcm_sw_params_set_stop_threshold(handle,
			swparams, buffer_size);
	assert(err >= 0);

	if (snd_pcm_sw_params(handle, swparams) < 0) {
		R2Error("unable to install sw params:");
		snd_pcm_sw_params_dump(swparams, log);
		return -R2_ESYSTEM;
	}

	if (verbose)
		snd_pcm_dump(handle, log);

	bits_per_sample_ = snd_pcm_format_physical_width(format_);
	bits_per_frame_ = bits_per_sample_ * channels_;

	return R2_EOK;
}

#endif

