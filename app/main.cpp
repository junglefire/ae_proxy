#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

#include <nlohmann/json.hpp>
#include <cxxopts.hpp>

#include <event_loop.h>
#include <logger.h>

#include "tcp_proxy.h"
#include "udp_proxy.h"
#include "proxy.h"

using json_t = nlohmann::json;
using namespace cxxopts;
using namespace smart;

#define TCP_PROXY	0
#define UDP_PROXY	1

typedef struct _args {
	std::string config_file;
	int mode;
	std::string mode_info;
} args_t;

UdpProxy* udp_proxy(const json_t&, EventLoop*);
TcpProxy* tcp_proxy(const json_t&, EventLoop*);
void parse_args(int, char**, args_t&);
int load_config(const char*, json_t&);

int main(int argc, char* argv[]) {
	// default argrments
	args_t args {
		"etc/proxy.json",
		TCP_PROXY,
		"TCP_PROXY"
	};

	parse_args(argc, argv, args);

	// smart proxy run...
	_info("smart_proxy run as `%s` mode...", args.mode_info.c_str());
	json_t jc;
	load_config(args.config_file.c_str(), jc);

	EventLoop* evloop = EventLoop::default_loop();
	Proxy* proxy = nullptr;
	if (args.mode == TCP_PROXY) {
		proxy = tcp_proxy(jc, evloop);
		proxy->start();
	} else {
		proxy = udp_proxy(jc, evloop);
		proxy->start();
	}
	evloop->run();
	delete proxy;
	return 0;
}

TcpProxy* tcp_proxy(const json_t& jc, EventLoop* evloop) {
	TcpProxy* proxy = new TcpProxy(
		jc["tcp.proxy"]["self.ip"].get<std::string>().c_str(), 
		jc["tcp.proxy"]["self.port"].get<int>(),
		evloop
	);
	return proxy;
}

UdpProxy* udp_proxy(const json_t& jc, EventLoop* evloop) {
	UdpProxy* proxy = new UdpProxy(
		jc["udp.proxy"]["self.ip"].get<std::string>().c_str(), 
		jc["udp.proxy"]["self.port"].get<int>(),
		evloop
	);
	return proxy;
}

void parse_args(int argc, char* argv[], args_t& args) {
	ParseResult result;
	Options options("smart_proxy", "Yet another smart proxy");

	try {
		options.add_options()
			("c,config", "Config File Name", value<std::string>()->default_value("etc/proxy.json"))
			("m,mode", "Proxy Mode: tcp|udp", value<std::string>()->default_value("tcp"))
			("h,help", "Print usage")
		;
		result = options.parse(argc, argv);
	} catch (cxxopts::OptionException &e){
		fprintf(stderr, "parse error: %s", e.what());
		exit(-1);
	}

	if (result.count("help")) {
		_info("%s", options.help().c_str());
		exit(0);
	}

	args.config_file = result["config"].as<std::string>();
	if (result["mode"].as<std::string>() == "udp") {
		args.mode = UDP_PROXY;
		args.mode_info = "UDP_PROXY";
	}
}

int load_config(const char* filename, json_t& jc) {
	std::ifstream i(filename);
	i >> jc;
	return 0;
}
