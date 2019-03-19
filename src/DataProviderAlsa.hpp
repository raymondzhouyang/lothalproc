/*
 * DataProviderAlsa.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/22
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifdef HAVE_ALSA

#ifndef _DATAPROVIDERALSA_HPP_
#define _DATAPROVIDERALSA_HPP_

#include <alsa/asoundlib.h>

class DataProviderAlsa: public DataProvider {
public:
	DataProviderAlsa();
	~DataProviderAlsa();

	int readBlock(const unsigned char* &data, int& len);
	bool isOk();
	const char* name();

private:
	int set_params(void);

private:
	snd_pcm_t *handle;
	/* SND_PCM_STREAM_PLAYBACK */
	snd_pcm_format_t format_ = SND_PCM_FORMAT_S32_LE;
	snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
	unsigned int channels_ = 8;
	unsigned int rate_ = 48000;
	unsigned int bits_per_sample_;
	unsigned int bits_per_frame_;

	unsigned period_time_ = 0;
	snd_pcm_uframes_t period_frames_ = 0;
	unsigned buffer_time_ = 0;
	snd_pcm_uframes_t buffer_frames_ = 0;

	unsigned char *audiobuf_ = NULL;
	snd_pcm_uframes_t chunk_size_ = 0;
	unsigned int audiobuf_size_ = 0;

	int open_mode = 0;
	int interleaved = 1;
	int verbose = 0;
};

#endif /* _DATAPROVIDERALSA_HPP_ */

#endif

