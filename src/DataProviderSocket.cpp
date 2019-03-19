/*
 * DataProviderSocket.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/29
 * Copyright © 2018 rokid. All rights reserved.
 */
#include <stdbool.h>
#include <assert.h>
#include "R2InServer.hpp"

#include "R2Types.hpp"
#include "DataProvider.hpp"
#include "DataProviderSocket.hpp"

using namespace std;
using namespace r2base;

DataProviderSocket::DataProviderSocket(int port, int block_size) {
	inServer_ = R2_NEW(R2InServer, port, "MIC IN", block_size);
	assert(NULL != inServer_);
	inServer_->startServer();

	R2Notice("Recv data from socket port %d", port);
}

DataProviderSocket::~DataProviderSocket() {
	R2_DEL(inServer_);
}

int DataProviderSocket::readBlock(const unsigned char* &data, int& len) {
	const unsigned char* temp = NULL;

	if (!inServer_ || !inServer_->isConnected()) {
		data = NULL;
		len = 0;
		return R2_EOK;
	}

	temp = inServer_->readOneBlock();
	if (!temp)
		return -R2_ESYSTEM;

	data = temp;
	len = inServer_->DataBlockSize();

	if (!inServer_->firstData) {
		inServer_->firstData = true;
		/* 第一次收到数据需重置以对齐日志时间轴 */
		R2Log::setAudioTime(true, 0);
		R2Log("really reset audio timestamp");
	}

	return R2_EOK;
}

bool DataProviderSocket::isOk() {
	return inServer_ && inServer_->isConnected();
}

const char* DataProviderSocket::name() {
	return "SOCKET";
}

