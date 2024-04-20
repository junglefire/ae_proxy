#include <getopt.h>

#include <cstdlib>
#include <string>
#include <memory>

#include <cxxopts.hpp>
#include <msgpack.h>

#include <socket_addr.h>
#include <event_loop.h>
#include <udp_socket.h>
#include <logger.h>
#include <timer.h>

using namespace cxxopts;
using namespace smart;

#define IP_LEN		16
#define MESSAGE 	"hello"
#define MSG_LEN 	5

int main(int argc, char* argv[]) {
	_info("`test_udp_client` test...");
	
	ParseResult args;
	Options options("test_udp_client", "Libuv udp client test app");

	try {
		options.add_options()
			("proxy-ip", "Proxy Server IP", value<std::string>()->default_value("127.0.0.1"))
			("proxy-port", "Proxy Server Port", value<int>()->default_value("4444"))
			("real-ip", "Real Server IP", value<std::string>()->default_value("127.0.0.1"))
			("real-port", "Real Server Port", value<int>()->default_value("5555"))
			("h,help", "Print usage")
		;
		args = options.parse(argc, argv);
	} catch (cxxopts::OptionException &e){
		fprintf(stderr, "parse error: %s", e.what());
		exit(-1);
	}

	if (args.count("help")) {
		_info("%s", options.help().c_str());
		exit(0);
	}

	std::string real_ip = args["real-ip"].as<std::string>();
	int real_port = args["real-port"].as<int>();
	std::string proxy_ip = args["proxy-ip"].as<std::string>();
	int proxy_port = args["proxy-port"].as<int>();

	_info("send message to real server `%s:%d` through proxy server `%s:%d`", real_ip.c_str(), real_port, proxy_ip.c_str(), proxy_port);

	EventLoop* evloop = EventLoop::default_loop();
	SocketAddr proxy_addr(args["proxy-ip"].as<std::string>(), args["proxy-port"].as<int>());

	UdpSocket client(evloop);
	client.set_message_callback([](SocketAddr& from, const char* data, unsigned size) {
		_info("get `%d` bytes msg: %s", size, data);
	});

	client.try_read();

	msgpack_sbuffer* buffer = msgpack_sbuffer_new();
	msgpack_packer* pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);

	Timer timer(evloop, 1000, 1000, [&](Timer*) {
		msgpack_sbuffer_clear(buffer);
		msgpack_pack_str(pk, real_ip.length());
		msgpack_pack_str_body(pk, real_ip.c_str(), real_ip.length());
		msgpack_pack_int(pk, real_port);
		msgpack_pack_str(pk, MSG_LEN);
		msgpack_pack_str_body(pk, MESSAGE, MSG_LEN);
		client.send(proxy_addr, buffer->data, buffer->size);
	});
	timer.start();

	evloop->run();

	// cleaning
	msgpack_sbuffer_free(buffer);
	msgpack_packer_free(pk);
	return 0;
}
