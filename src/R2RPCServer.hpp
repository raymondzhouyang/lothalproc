/*
 * R2RPCServer.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/7
 * Copyright © 2018 rokid. All rights reserved.
 */
#ifndef _R2RPCSERVER_HPP_
#define _R2RPCSERVER_HPP_

#include <thread>
#include <unordered_map>

namespace lothal {

#define R2_RPC_CALL_TAG     0x12345678

typedef enum {
	R2_RPC_METHOD_RESTART = 2,
	R2_RPC_METHOD_RESTART_NOARGS,
	R2_RPC_METHOD_ADD_WORD,
	R2_RPC_METHOD_ADD_WORD2,
	R2_RPC_METHOD_REMOVE_WORD,
	R2_RPC_METHOD_GET_WORDS,
	R2_RPC_METHOD_GET_WORD_INFO,
	R2_RPC_METHOD_PICKUP,
	R2_RPC_METHOD_IS_PICKUP,
	R2_RPC_METHOD_OPEN_MIC,
	R2_RPC_METHOD_IS_MIC_OPENED,
	R2_RPC_METHOD_SET_STACK,
	R2_RPC_METHOD_SET_SKILL_OPTION,
	R2_RPC_METHOD_VOICE_ENERGY,
	R2_RPC_METHOD_MIN_MODE,
	R2_RPC_METHOD_DETECT_VOICE_TRIGGER,
	R2_RPC_METHOD_SET_OFFLINE_ASR_FLAG,
	R2_RPC_METHOD_SET_VOIP_ANGLE,
	R2_RPC_METHOD_ADD_WORD_MARGIN,
} R2RpcMethodID;

/* TODO: 当前的R2Server实现不允许多个RPCClient连入。新
 * Client接入时，会关闭原socket。先保留当前实现，后续若
 * 有必要考虑多个Client，则必须要优化。另外，连接长时间
 * 无数据时，未实现主动踢掉Client。
 */
class R2RPCServer: public r2base::R2Server {
public:
	R2RPCServer(R2Lothal &lothal, int port);
	~R2RPCServer();
	void jobRun();

private:
	const char* getMethodName(int id);
	int callMethod(int method_id, R2Stream& stream, bool& has_more);

private:
	R2Lothal &lothal_;
	std::thread rpc_thread_;
	std::unordered_map<int, const char*> id_name_map_;
	bool map_inited_;
};
}

#endif /* _R2RPCSERVER_HPP_ */

