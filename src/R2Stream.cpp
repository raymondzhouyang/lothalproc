/*
 * R2Stream.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/18
 * Copyright © 2018 rokid. All rights reserved.
 */

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "R2Base.hpp"
#include "R2Stream.hpp"

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

using namespace std;
using namespace r2base;

namespace lothal {

R2Stream::R2Stream(int totalSize) :
		buffer_(NULL), write_pos_(0), read_pos_(0),
		total_buffer_size_(totalSize) {
	buffer_ = R2_NEW_AR1(unsigned char, total_buffer_size_);
}

R2Stream::~R2Stream() {
	R2_DEL_AR1(buffer_);
}

void R2Stream::writeFrame(const unsigned char* cs, int len) {
	if (!cs || len <= 0) {
		R2Error("Input is invalid: %p, %d", cs, len);
		return;
	}

	if (write_pos_ + len >= total_buffer_size_) {
		R2Error("Buffer is not enough: %d, %d, %d",
			write_pos_, len, total_buffer_size_);
		return;
	}

	memcpy(buffer_ + write_pos_, cs, len);
	write_pos_ += len;
}

void R2Stream::writeChar(char i) {
	if (write_pos_ + 1 >= total_buffer_size_) {
		R2Error("Buffer is not enough: %d, 1, %d",
			write_pos_, total_buffer_size_);
		return;
	}

	buffer_[write_pos_++] = (unsigned char)i;
}

char R2Stream::readChar() {
	if (read_pos_ + 1 > write_pos_) {
		R2Error("Read the end of buffer_: %d, 1, %d",
			read_pos_, write_pos_);
		return 0;
	}

	char d1 = buffer_[read_pos_++];
	return d1;
}

void R2Stream::writeShort(short i) {
	unsigned char* p;

	if (write_pos_ + 2 >= total_buffer_size_) {
		R2Error("Buffer is not enough: %d, 2, %d",
			write_pos_, total_buffer_size_);
		return;
	}

	p = (unsigned char*)&i;
	buffer_[write_pos_++] = p[0];
	buffer_[write_pos_++] = p[1];
}

short R2Stream::readShort() {
	short value = 0;
	unsigned char* p;

	if (read_pos_ + 2 > write_pos_) {
		R2Error("Read the end of buffer_: %d, 2, %d",
			read_pos_, write_pos_);
		return 0;
	}

	p = (unsigned char*) &value;
	p[0] = buffer_[read_pos_++];
	p[1] = buffer_[read_pos_++];
	return value;
}

void R2Stream::writeInt(int i) {
	unsigned char* p;

	if (write_pos_ + 4 >= total_buffer_size_) {
		R2Error("Buffer is not enough: %d, 4, %d",
			write_pos_, total_buffer_size_);
		return;
	}

	p = (unsigned char*) &i;
	buffer_[write_pos_++] = p[0];
	buffer_[write_pos_++] = p[1];
	buffer_[write_pos_++] = p[2];
	buffer_[write_pos_++] = p[3];
}

int R2Stream::readInt() {
	int value = 0;
	unsigned char* p;

	if (read_pos_ + 4 > write_pos_) {
		R2Error("Read the end of buffer_: %d, 4, %d",
			read_pos_, write_pos_);
		return 0;
	}

	p = (unsigned char*) &value;
	p[0] = buffer_[read_pos_++];
	p[1] = buffer_[read_pos_++];
	p[2] = buffer_[read_pos_++];
	p[3] = buffer_[read_pos_++];
	return value;
}

void R2Stream::writeFloat(float i) {
	unsigned char* p;

	if (write_pos_ + 4 >= total_buffer_size_) {
		R2Error("Buffer is not enough: %d, 4, %d",
			write_pos_, total_buffer_size_);
		return;
	}

	p = (unsigned char*) &i;
	buffer_[write_pos_++] = p[0];
	buffer_[write_pos_++] = p[1];
	buffer_[write_pos_++] = p[2];
	buffer_[write_pos_++] = p[3];
}

float R2Stream::readFloat() {
	float value = 0;
	unsigned char* p;

	if (read_pos_ + 4 > write_pos_) {
		R2Error("Read the end of buffer_: %d, 4, %d",
			read_pos_, write_pos_);
		return 0;
	}

	p = (unsigned char*) &value;
	p[0] = buffer_[read_pos_++];
	p[1] = buffer_[read_pos_++];
	p[2] = buffer_[read_pos_++];
	p[3] = buffer_[read_pos_++];
	return value;
}

void R2Stream::writeString(const string& s) {
	const short LEN = (short) (s.length() + 1);

	if (write_pos_ + 2 + LEN >= total_buffer_size_) {
		R2Error("Buffer is not enough: %d, %d, %d",
			write_pos_, LEN, total_buffer_size_);
		return;
	}

	if (LEN == 1) {
		writeShort(0);
		return;
	}

	writeShort(LEN);
	memcpy(buffer_ + write_pos_, s.c_str(), LEN);
	write_pos_ += LEN;
}

string R2Stream::readString() {
	const int LEN = readShort();

	if (LEN <= 0)
		return "";

	const char* cs = (const char*) (buffer_ + read_pos_);
	read_pos_ += LEN;
	return cs;
}

void R2Stream::reset() {
	read_pos_ = 0;
	write_pos_ = 0;
}

int R2Stream::getReadIdx() {
	return read_pos_;
}

int R2Stream::getWriteIdx() {
	return write_pos_;
}

int R2Stream::getDataLen() {
	return write_pos_;
}

void R2Stream::setWriteIdx(int i) {
	write_pos_ = i;
}

void R2Stream::setReadIdx(int i) {
	read_pos_ = i;
}

const unsigned char* R2Stream::getBuffer() {
	return buffer_;
}

const unsigned char* R2Stream::getData() {
	return buffer_;
}

void R2Stream::renewHeadLen() {
	int index = getWriteIdx();
	setWriteIdx(0);
	writeInt(index - 4);
	setWriteIdx(index);
}

bool R2Stream::readBlock(int sock, void* buffer0, int BlockTotalSize) {
	int readedSize = 0;
	char* buffer = (char*) buffer0;

	if (sock <= 0) {
		R2Error("Invalid socket: %d", sock);
		return false;
	}

	while (buffer0 && readedSize < BlockTotalSize) {
		long dSize = recv(sock, buffer + readedSize,
				BlockTotalSize - readedSize, 0);
		if (dSize <= 0) {
			R2Error("Socket connect is breakdown");
			return false;
		}

		readedSize += dSize;
	}

	return true;

}

bool R2Stream::sendDataBlock(int sock,
		const unsigned char* chars, int size) {
	if (sock <= 0) {
		R2Error("Invalid socket: %d", sock);
		return false;
	}

	while (size > 0) {
		long dsize = send(sock, chars, size, MSG_NOSIGNAL);
		if (dsize <= 0) {
			R2Error("Socket connect is breakdown");
			return false;
		}

		size -= dsize;
		chars += dsize;
	}
	return true;
}

}
