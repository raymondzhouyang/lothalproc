/*
 * main.cpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/10
 * Copyright © 2018 rokid. All rights reserved.
 */
#include <unistd.h>
#include <getopt.h>
#include "R2Base.hpp"
#include "R2Util.hpp"
#include "R2InServer.hpp"
#include "R2FrameQueue.hpp"
#include "R2Data.hpp"

#include "R2Lothal.hpp"
#include "DataProvider.hpp"
#include "DataProviderFile.hpp"
#include "DataProviderMic.hpp"
#include "DataProviderAlsa.hpp"
#include "DataProviderSocket.hpp"
#include "R2Client.hpp"
#ifdef HAVE_FLORA
#include "R2Flora.hpp"
#else
#include "R2Speech.hpp"
#endif
#include "R2EventExecutor.hpp"
#include "R2Stream.hpp"
#include "R2RPCServer.hpp"
#include "LothalProc.hpp"

using namespace std;
using namespace lothal;

#define PROVIDER_PORT		(R2Lothal::getBasePort())
#define LOG_SERVER_PORT		(R2Lothal::getBasePort() + 100)
#define LOTHAL_CONFIG		"/system/workdir_asr_cn/lothal.ini"
#define FLORA_URI		"unix:/var/run/flora.sock"

#ifndef COMMIT
#define COMMIT "unknow"
#endif

static struct option long_opts[] = {
	{"help", no_argument, 0, 'h'},
	{"version", no_argument, 0, 'v'},
	{"config", required_argument, 0, 'c'},
	{"provider", required_argument, 0, 'p'},
	{"type", required_argument, 0, 'T'},
	{"base-port", required_argument, 0, 4},
	{"provider-port", required_argument, 0, 2},
	{"log-server", required_argument, 0, 1},
	{"record-mode", no_argument, 0, 5},
#ifdef HAVE_FLORA
	{"flora-uri", required_argument, 0, 3}
#endif
};

void show_promt() {
	printf("Use -h or --help to get help.\n");
	exit(-1);
}

void show_help() {
	printf("This is an audio processor using the SDK Lothal. Usage:\n");
	printf("\n  lothalproc [options]\n");
	printf("\nSelection of options:\n\n");
	printf("  -h, --help           Show this help message.\n");
	printf("  -v, --version        Show the version information.\n");
	printf("  -c, --config=PATH    Set the Lothal config file path. Default is\n"
		"                       %s.\n", LOTHAL_CONFIG);
	printf("  -T, --type=TYPE      The type of provider, can set to be \"mic\", \"socket\" or \"file\".\n"
		"                       Default is \"mic\".\n");
	printf("  -p, --provider=FILE  The .wav file path when use file as provider. Use this when\n"
		"                       \"--type=file\" is set.\n");
	printf("  --base-port=PORT     Set the base port for socket. This would be useful if there're more than\n"
		"                       one process running on the same system. This keeps them use different ports.\n"
		"                       Default is %d.\n", PROVIDER_PORT);
	printf("  --provider-port=PORT Set the socket provider port. Use this port when \"--type=socket\".\n"
		"                       is set. Default is %d.\n", PROVIDER_PORT);
	printf("  --log-server=PORT    Set log server port. Default is %d.\n", LOG_SERVER_PORT);
	printf("  --record-mode        Enable record mode. Keep reading audio data when is mute.\n");
#ifdef HAVE_FLORA
	printf("  --flora-uri=URI      Set set flora uri. Default is %s.\n", FLORA_URI);
#endif
	printf("\n");
	exit(-1);
}

void show_version() {
	printf("%s\n", R2Lothal::version());

	exit(-1);
}

int main(int argc, char* argv[]) {
	int opt_index = 0;
	int c;
	string config = LOTHAL_CONFIG;
	string file_path;
	string type = "mic";
	R2Lothal* lothal = NULL;
	LothalProc* lothalproc = NULL;
	DataProvider* provider = NULL;
	int base_port = PROVIDER_PORT;
	int provider_port = PROVIDER_PORT;
	R2Client *client = NULL;
	R2EventExecutor *executor = NULL;
	r2base::R2Server* rpc_server = NULL;
	int log_port = LOG_SERVER_PORT;
	bool record = false;
#ifdef HAVE_FLORA
	string flora_uri = FLORA_URI;
#endif

    R2Log("BUILD: " __DATE__ ", commit:%s", COMMIT);

	while (1) {
		c = getopt_long(argc, argv, "c:T:p:hv", long_opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case 1:
			log_port = atoi(optarg);
			break;
		case 'c':
			config = optarg;
			break;
		case 'p':
			file_path = optarg;
			break;
		case 'T':
			type = optarg;
			break;
		case 2:
			provider_port = atoi(optarg);
			break;
#ifdef HAVE_FLORA
		case 3:
			flora_uri = optarg;
			break;
#endif
		case 4:
			base_port = atoi(optarg);
			break;
		case 5:
			record = true;
			break;
		case 'v':
			show_version();
		case 'h':
		case '?':
		default:
			show_help();
		}
	}

	R2Lothal::setBasePort(base_port);
	R2Log::setLogServer(log_port);

	if (config.length() <= 0) {
		R2Error("config path is invalid.");
		show_promt();
	}

	lothal = R2_NEW(R2Lothal, config);
	if (!lothal) {
		R2Error("Failed to create R2Lothal.");
		goto _finish;
	}

	if (type == "file") {
		if (file_path.length() <= 0) {
			R2Error("Not set provider.");
			show_promt();
		}

		provider = R2_NEW(DataProviderFile, file_path);
	} else if (type == "mic") {
#ifdef HAVE_MIC_ARRAY
		provider = R2_NEW(DataProviderMic, lothal->getDataBlockSize());
#else
#ifdef HAVE_ALSA
		provider = R2_NEW(DataProviderAlsa);
#endif
#endif
	} else if (type == "socket") {
		provider = R2_NEW(DataProviderSocket, provider_port,
				lothal->getDataBlockSize());
	} else {
		R2Error("Invalid provider type: %s", type.c_str());
		goto _finish;
	}

	if (!provider) {
		R2Error("Failed to create DataProvider.");
		goto _finish;
	}

	lothalproc = R2_NEW(LothalProc, lothal);
	if (!lothalproc) {
		R2Error("Failed to create LothalProc.");
		goto _finish;
	}
	lothalproc->setRecordMode(record);
	lothalproc->setProvider(provider);

#ifdef HAVE_FLORA
	client = R2_NEW(R2Flora, *lothal, flora_uri);
#else
	client = R2_NEW(R2Speech, *lothal, lothal->isModuleEnabled("OPUS"));
#endif
	if (!client) {
		R2Error("Failed to create client.");
		goto _finish;
	}

	executor = R2_NEW(R2EventExecutor, *lothal, *client);
	if (!executor) {
		R2Error("Failed to create R2EventExecutor.");
		goto _finish;
	}

	rpc_server = R2_NEW(R2RPCServer, *lothal, lothal->getBasePort() + 300);
	if (NULL == rpc_server) {
		R2Error("Failed to start RPC.");
		goto _finish;
	}

	lothalproc->run();

_finish:
	R2_DEL(rpc_server);
	R2_DEL(executor);
	R2_DEL(lothalproc);
	R2_DEL(client);
	R2_DEL(provider);
	R2_DEL(lothal);

	r2_memory_leak_print();
	return 0;
}

