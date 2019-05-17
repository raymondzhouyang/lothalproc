/*
 * R2RPCServer.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/7
 * Copyright ? 2018 rokid. All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string>
#include <ctime>
#include <thread>
#include <unordered_map>

#include "R2Base.hpp"
#include "R2Util.hpp"
#include "R2Server.hpp"

#include "R2Lothal.hpp"
#include "R2Stream.hpp"
#include "R2RPCServer.hpp"

using namespace std;
using namespace r2base;

namespace lothal {

static void RPCThreadMain(R2RPCServer* server) {
	server->jobRun();
}

R2RPCServer::R2RPCServer(R2Lothal &lothal, int port) :
		R2Server(port, "RPCServer"), lothal_(lothal),
		rpc_thread_(RPCThreadMain, this),
		id_name_map_(), map_inited_(false) {
	this->startServer();
}

R2RPCServer::~R2RPCServer() {
	exit = true;
	if (connectHandle) {
		shutdown(connectHandle, 0);
	}
	closeConnect();
	rpc_thread_.join();
}

void R2RPCServer::jobRun() {
	R2Stream stream(20 * 1024);
	const int BUF_TOTAL_LEN = 10 * 1024;
	unsigned char* buffer = R2_NEW_AR1(unsigned char, BUF_TOTAL_LEN);
	int block_size = 0;
	int magic;
	int method_id;
	bool has_more;
	int ret;

	while (!exit) {
		if (!isConnected()) {
			usleep(1000 * 1000);
			continue;
		}

		if (!R2Util::readInt(connectHandle, block_size)) {
			closeConnect();
			continue;
		}

		if (block_size <= 0)
			continue;

		if (block_size > BUF_TOTAL_LEN) {
			R2Error("out of buffer size: %d %d",
				block_size, BUF_TOTAL_LEN);
			closeConnect();
			continue;
		}

		if (!R2Util::readBlock(connectHandle, buffer, block_size)) {
			closeConnect();
			continue;
		}

		stream.reset();
		stream.writeFrame(buffer, block_size);
		magic = stream.readInt();
		if (magic != R2_RPC_CALL_TAG) {
			R2Error("error: %d, %X", magic, magic);
			continue;
		}

		method_id = stream.readInt();
		R2Log("RPC Method: %s(%d)",
			getMethodName(method_id), method_id);

		ret = callMethod(method_id, stream, has_more);
		if (!has_more) {
			stream.reset();
			stream.writeInt(8);
			stream.writeInt(R2_RPC_CALL_TAG);
			stream.writeInt(ret);
		}

		R2Stream::sendDataBlock(connectHandle,
					stream.getData(), stream.getDataLen());
	}

	R2_DEL_AR1(buffer);
}

const char* R2RPCServer::getMethodName(int id) {
	if (!map_inited_) {
		map_inited_ = true;
		id_name_map_[R2_RPC_METHOD_RESTART] = "restart(...)";
		id_name_map_[R2_RPC_METHOD_RESTART_NOARGS] = "restart()";
		id_name_map_[R2_RPC_METHOD_ADD_WORD] = "addWord(content, pinyin, type)";
		id_name_map_[R2_RPC_METHOD_ADD_WORD2] = "addWord(content, info)";
		id_name_map_[R2_RPC_METHOD_PICKUP] = "pickup(bool)";
		id_name_map_[R2_RPC_METHOD_MIN_MODE] = "setMinMode(bool)";
		id_name_map_[R2_RPC_METHOD_IS_PICKUP] = "isPickup()";
		id_name_map_[R2_RPC_METHOD_OPEN_MIC] = "openMic(bool)";
		id_name_map_[R2_RPC_METHOD_IS_MIC_OPENED] = "isMicOpened()";
		id_name_map_[R2_RPC_METHOD_SET_STACK] = "setStack(string)";
		id_name_map_[R2_RPC_METHOD_SET_SKILL_OPTION] = "setSkillOption(string)";
		id_name_map_[R2_RPC_METHOD_DETECT_VOICE_TRIGGER] = "detectVoiceTrigger(bool)";
		id_name_map_[R2_RPC_METHOD_VOICE_ENERGY] = "float voiceEnergy()";
		id_name_map_[R2_RPC_METHOD_SET_OFFLINE_ASR_FLAG] = "setOfflineAsrFlag(bool)";
		id_name_map_[R2_RPC_METHOD_SET_VOIP_ANGLE] = "setVOIPAngle(int)";
	}
	const char* name = id_name_map_[id];
	return name ? name : "Unknown method id";
}

/* 处理RPC请求，必须考虑向后兼容，接口发生变化，需要
 * 兼容性问题.
 *
 * method_id: 输入参数，RPC方法ID
 *    stream: 输入参数，每个不同的方法，所携带的信息不同
 *            输出参数，取决于 has_more 值，如果 has_more 为True
 *            则也用作输出参数，用于携带返回的数据；否则，
 *            只向客户端返回结果错误码。
 *  has_more: 输出参数，表示stream中是否包含需要返回给客户
 *            端的数据.
 */
int R2RPCServer::callMethod(int method_id, R2Stream& stream, bool& has_more) {
	has_more = 0;

	switch (method_id) {
	case R2_RPC_METHOD_RESTART: {
		R2ConfigOption co;
		/* 用来重新设置外部speech参数，该参数不由SDK
		 * 维护，通过事件通知外部speech模块进行处理。
		 */
		co.host = stream.readString();
		co.port = stream.readInt();
		co.branch = stream.readString();
		co.key = stream.readString();
		co.secret = stream.readString();
		co.device_type_id = stream.readString();
		co.device_id = stream.readString();

		R2Log("RPC <%s>: %s, %d, %s, %s, %s, %s, %s",
			getMethodName(method_id), co.host.c_str(),
			co.port, co.branch.c_str(),
			co.key.c_str(), co.secret.c_str(),
			co.device_type_id.c_str(),
			co.device_id.c_str());

		lothal_.reportEvent(R2_EVENT_RECONFIG, &co);
		break;
	}
	case R2_RPC_METHOD_RESTART_NOARGS:
		R2Notice("Legacy RPC <%s>, do nothing, has no meaning",
			getMethodName(method_id));
		break;
	case R2_RPC_METHOD_ADD_WORD: {
		string content = stream.readString();
		string pinyin = stream.readString();
		int type = stream.readInt();

		R2Log("RPC <%s>: %s, %s, %d", getMethodName(method_id),
			content.c_str(), pinyin.c_str(), type);
		return lothal_.addWord(content, pinyin, type);
	}
	case R2_RPC_METHOD_ADD_WORD2: {
		string content = stream.readString();
		string info = stream.readString();

		R2Log("RPC <%s>: %s, %s, %d", getMethodName(method_id),
			content.c_str(), info.c_str(), -1);
		return lothal_.addWord(content, info, -1);
	}
	case R2_RPC_METHOD_ADD_WORD_MARGIN: {
		string content = stream.readString();
		string pinyin = stream.readString();
		int type = stream.readInt();
		float margin = stream.readFloat();
		R2Info("Call lothal.addWord(%s, %s, %d, %f)",
			   content.c_str(), pinyin.c_str(), type, margin);
		return lothal_.addWord(content, pinyin, type, margin);
	}
	case R2_RPC_METHOD_REMOVE_WORD: {
		string content = stream.readString();

		R2Log("RPC <%s>: %s",
			getMethodName(method_id), content.c_str());
		return lothal_.removeWord(content);
	}
	case R2_RPC_METHOD_GET_WORDS: {
		string wordsinfo;
		vector<string> words;
		lothal_.getAllWords(wordsinfo);
		words = r2_strsplit(wordsinfo, '\n');
		has_more = true;

		stream.reset();
		stream.writeInt(0);
		stream.writeInt(R2_RPC_CALL_TAG);
		stream.writeInt((int)words.size());
		for (int i = 0; i < (int) words.size(); i++)
			stream.writeString(words[i]);

		stream.renewHeadLen();

		R2Log("RPC <%s>: %d",
			getMethodName(method_id), (int)words.size());
		break;
	}
	case R2_RPC_METHOD_GET_WORD_INFO: {
		string content = stream.readString();
		string info;
		lothal_.getWordInfo(content, info);

		has_more = true;
		stream.reset();
		stream.writeInt(0);
		stream.writeInt(R2_RPC_CALL_TAG);
		stream.writeString(info);
		stream.renewHeadLen();
		R2Log("RPC <%s>: %s, %s",
			getMethodName(method_id),
			content.c_str(), info.c_str());
		break;
	}
	case R2_RPC_METHOD_PICKUP: {
		bool yes = stream.readInt();

		R2Log("RPC <%s>: %d", getMethodName(method_id), yes);
		return lothal_.setPickUpStatus(yes);
	}
	case R2_RPC_METHOD_IS_MIC_OPENED:
		return lothal_.isReady();
	case R2_RPC_METHOD_OPEN_MIC: {
		bool yes = stream.readInt();

		R2Log("RPC <%s>: %d", getMethodName(method_id), yes);
		if (yes)
			return lothal_.resumePipeline();
		return lothal_.pausePipeline();
	}
	case R2_RPC_METHOD_MIN_MODE:
		R2Notice("Legacy RPC <%s>, won't maintain anymore",
			getMethodName(method_id));
		break;

	case R2_RPC_METHOD_SET_STACK: {
		string stack = stream.readString();

		R2Info("RPC <%s>: %s", getMethodName(method_id), stack.c_str());
		lothal_.reportEvent(R2_EVENT_SETSTACK, &stack);
		break;
	}
	case R2_RPC_METHOD_SET_SKILL_OPTION: {
		string skill = stream.readString();

		R2Verbose("RPC <%s>: %s",
			getMethodName(method_id), skill.c_str());
		lothal_.reportEvent(R2_EVENT_SETSKILL, &skill);
		break;
	}
#if 0
	case R2_RPC_METHOD_IS_PICKUP:
		R2Log("RPC <%s>: %d", getMethodName(method_id));
		return lothal_.getPickUpStatus();
	case R2_RPC_METHOD_VOICE_ENERGY: {
		float f = lothal_.getVoiceEnergy();
		int *pi = (int *)&f;

		R2Log("RPC <%s>: %f, %d", getMethodName(method_id), f, *pi);
		return *pi;
	}
	case R2_RPC_METHOD_SET_OFFLINE_ASR_FLAG: {
		bool yes = stream.readInt();

		R2Log("RPC <%s>: %d", getMethodName(method_id), yes);
		return lothal_.setOfflineAsrFlag(yes);
	}
	case R2_RPC_METHOD_SET_VOIP_ANGLE: {
		int angle = stream.readInt();

		R2Log("RPC <%s>: %d", getMethodName(method_id), angle);
		return lothal_.setVoipAngle(angle);
	}
#endif
	case R2_RPC_METHOD_DETECT_VOICE_TRIGGER: {
		int yesno = stream.readInt();

		R2Log("RPC <%s>: %d", getMethodName(method_id), yesno);
		return lothal_.setVoipStatus(yesno);
	}
	default:
		R2Error("Unknown method id: %d", method_id);
		return -R2_ENOTSUPPORT;
	}

	return R2_EOK;
}

}

