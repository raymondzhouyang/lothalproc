/*
 * DataProviderFile.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/11
 * Copyright © 2018 rokid. All rights reserved.
 */

#ifndef _DATAPROVIDERFILE_HPP_
#define _DATAPROVIDERFILE_HPP_

#include <string>

class DataProviderFile: public DataProvider {
public:
	DataProviderFile(std::string &file_path);
	~DataProviderFile();

	int readBlock(const unsigned char* &data, int& len);
	bool isOk();
	const char* name();

private:
	int fd_ = 0;
	unsigned char *buffer_ = 0;
	int  buf_size_ = 0;
	bool is_inited_ = false;
};

#endif /* _DATAPROVIDERFILE_HPP_ */

