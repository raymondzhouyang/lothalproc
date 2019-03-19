/*
 * DataProvider.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/11
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifndef _DATAPROVIDER_HPP_
#define _DATAPROVIDER_HPP_

class DataProvider {
public:
	DataProvider() {};
	virtual ~DataProvider() {};

	virtual int readBlock(const unsigned char* &data, int& len) = 0;
	virtual bool isOk() = 0;
	virtual const char* name() = 0;
};

#endif /* _DATAPROVIDER_HPP_ */

