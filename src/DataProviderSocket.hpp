/*
 * DataProviderSocket.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/29
 * Copyright © 2018 rokid. All rights reserved.
 */

#ifndef _DATAPROVIDERSOCKET_HPP_
#define _DATAPROVIDERSOCKET_HPP_

class DataProviderSocket: public DataProvider {
public:
	DataProviderSocket(int port, int block_size);
	~DataProviderSocket();

	int readBlock(const unsigned char* &data, int& len);

	bool isOk();
	const char* name();

private:
	r2base::R2InServer* inServer_ = NULL;
};

#endif /* _DATAPROVIDERSOCKET_HPP_ */

