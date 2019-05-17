/*
 * R2Stream.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/18
 * Copyright © 2018 rokid. All rights reserved.
 */

/** \file R2Stream.hpp
 * \brief 封装 R2Stream 类用于读写事件数据流。
 */

#ifndef _R2STREAM_HPP_
#define _R2STREAM_HPP_

#include <string>

namespace lothal {

/** \class R2Stream
 * \brief 数据流操作类
 */
class R2Stream {
public:
	R2Stream(int totalSize);
	~R2Stream();

public:
	void writeChar(char c);
	void writeShort(short s);
	void writeInt(int i);
	void writeFloat(float i);
	void writeString(const std::string& s);

	char readChar();
	short readShort();
	int readInt();
	float readFloat();
	std::string readString();

	void reset();

	int getReadIdx();
	int getWriteIdx();
	int getDataLen();

	void setWriteIdx(int i);
	void setReadIdx(int i);
	const unsigned char* getBuffer();
	const unsigned char* getData();
	void renewHeadLen();
	void writeFrame(const unsigned char* cs, int len);

public:
	static bool readBlock(int socketHd, void* buffer0, int BlockTotalSize);
	static bool sendDataBlock(int connectHD, const unsigned char* chars, int size);

private:
	unsigned char* buffer_;
	int write_pos_;
	int read_pos_;
	const int total_buffer_size_;
};
}

#endif /* _R2STREAM_HPP_ */

