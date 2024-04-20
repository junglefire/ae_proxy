#include <getopt.h>

#include <cstdlib>
#include <chrono>
#include <thread>
#include <string>

#include <tcp_client.h>
#include <cxxopts.hpp>
#include <msgpack.h>
#include <logger.h>
#include <timer.h>

#define INTERVAL	1
#define MESSAGE 	"hello"
#define MSG_LEN 	5

#define _CONNECTING	1000
#define _CONNECTED	1001

using namespace std::chrono_literals;
using namespace cxxopts;
using namespace smart;



int main(int argc, char* argv[]) {
	_info("`test_tcp_client` test...");

	ParseResult args;
	Options options("test_tcp_client", "Libuv tcp client test app");

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

	int client_status = _CONNECTING;

	TcpClient client(evloop);
	client.set_connect_status_callback([&](ConnectStatus status) {
		if (status == ConnectStatus::ConnectSuccess) {
			client_status = _CONNECTED;
			_info("connect server ok");
		} else {
			_error("error : connect to server fail");
		}
	});
	
	client.set_message_callback([](const char* data, ssize_t size) {
		_info("recv %d bytes message: %s", size, data);
	});
	client.connect(proxy_addr);
	
	msgpack_sbuffer* buffer = msgpack_sbuffer_new();
	msgpack_packer* pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);


	// tcp proxy command
	// - 0x00: bind
	// - 0x01: wait for bind response
	// - 0x02: send data
	// - 0x03: sending data
	// - 0x04: unbind
	int command = 0x00;

	Timer timer(evloop, 1000, 1000, [&](Timer*) {
		if (client_status == _CONNECTING) {
			return;
		}
		msgpack_sbuffer_clear(buffer);
		if (command == 0x00) { // bind 
			msgpack_pack_int(pk, command);
			msgpack_pack_str(pk, real_ip.length());
			msgpack_pack_str_body(pk, real_ip.c_str(), real_ip.length());
			msgpack_pack_int(pk, real_port);
			msgpack_pack_str(pk, MSG_LEN);
			msgpack_pack_str_body(pk, MESSAGE, MSG_LEN);
			client.write(buffer->data, buffer->size);
			command = 0x01;
		}
	});
	timer.start();

	evloop->run();
	return 0;
}

// [Warning] this is an obsolete function
int main_obsolete_function(int argc, char* argv[]) {
	_info("`test_tcp_client` test...");

	ParseResult args;
	Options options("test_tcp_client", "Libuv tcp client test app");

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

	TcpClient client(evloop);
	client.set_connect_status_callback([&client](ConnectStatus status) {
		if (status == ConnectStatus::ConnectSuccess) {
			_info("connect server ok");
			char data[4096] = "hello world!";
			client.write(data, sizeof(data));
		} else {
			_error("error : connect to server fail");
		}
	});
	
	client.set_message_callback([&client](const char* data, ssize_t size) {
		client.write(data, (unsigned)size, nullptr);
		std::this_thread::sleep_for(1000ms);
	});
	client.connect(proxy_addr);
	evloop->run();
	return 0;
}

