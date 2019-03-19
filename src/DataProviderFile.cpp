/*
 * FileAudioProvider.cpp
 *
 *  Created on: 2018年6月7日
 *      Author: parallels
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "R2Log.hpp"

#include "R2Types.hpp"
#include "DataProvider.hpp"
#include "DataProviderFile.hpp"

using namespace std;
using namespace r2base;

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct riff_wave_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t wave_id;
};

struct chunk_header {
	uint32_t id;
	uint32_t sz;
};

struct chunk_fmt {
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
};

static int readn(int fd, void *vptr, size_t n) {
	size_t nleft;
	int nread;
	char *ptr;

	ptr = (char *) vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nread = (int) read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0; /* and call read() again */
			else
				return (-1);
		} else if (nread == 0)
			break; /* EOF */

		nleft -= nread;
		ptr += nread;
	}
	return (int) (n - nleft); /* return >= 0 */
}

DataProviderFile::DataProviderFile(std::string &file_path) :
		DataProvider() {
	size_t wave_pos;
	struct riff_wave_header riff_wave_header;
	struct chunk_header chunk_header;
	struct chunk_fmt chunk_fmt;
	int more_chunks = 1;

	if (file_path.length() <= 0) {
		R2Error("Invalid file path");
		exit(1);
	}

	fd_ = open(file_path.c_str(), O_RDONLY);
	if (fd_ < 0) {
		R2Error("Unable to open fd_ '%s', '%s'\n",
			file_path.c_str(), strerror(errno));
		exit(1);
	}

	wave_pos = file_path.rfind(".wav");
	if ((wave_pos != std::string::npos) &&
		((wave_pos + 4) == file_path.length())) {
		size_t size = read(fd_, &riff_wave_header,
				sizeof(riff_wave_header));
		assert(size == sizeof(riff_wave_header));
		if ((riff_wave_header.riff_id != ID_RIFF) ||
			(riff_wave_header.wave_id != ID_WAVE)) {
			R2Error("Error: '%s' is not a riff/wave fd_\n",
				file_path.c_str());
			close(fd_);
			exit(1);
		}

		do {
			size = read(fd_, &chunk_header,
					sizeof(chunk_header));
			assert(size == sizeof(chunk_header));

			switch (chunk_header.id) {
			case ID_FMT:
				size = read(fd_, &chunk_fmt,
					sizeof(chunk_fmt));
				assert(size == sizeof(chunk_fmt));

				/* If the format header is larger,
				 * skip the rest */
				if (chunk_header.sz > sizeof(chunk_fmt))
					lseek(fd_,
					chunk_header.sz - sizeof(chunk_fmt),
					SEEK_CUR);
				break;
			case ID_DATA:
				/* Stop looking for chunks */
				more_chunks = 0;
				break;
			default:
				/* Unknown chunk, skip bytes */
				lseek(fd_, chunk_header.sz, SEEK_CUR);
			}
		} while (more_chunks);
	}
	is_inited_ = true;
}

DataProviderFile::~DataProviderFile() {
	if (fd_ >= 0)
		close(fd_);
}

int DataProviderFile::readBlock(const unsigned char* &data, int& len) {
	int n = 0;

	if (buffer_ == NULL || buf_size_ < len) {
		buf_size_ = len;
		buffer_ = (unsigned char *) realloc(buffer_, len);
	}

	n = readn(fd_, buffer_, len);
	if (n <= 0) {
		R2Error("read error %s!!!", strerror(errno));
		return -R2_ESYSTEM;
	}

	data = buffer_;
	len = n;

	return R2_EOK;
}

bool DataProviderFile::isOk() {
	return is_inited_;
}

const char* DataProviderFile::name() {
	return "FILE";
}

