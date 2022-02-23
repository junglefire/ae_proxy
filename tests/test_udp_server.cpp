#include <getopt.h>

#include <cstdlib>
#include <string>
#include <memory>

#include <socket_addr.h>
#include <event_loop.h>
#include <udp_socket.h>
#include <logger.h>

using namespace smart;

typedef struct _args {
	std::string ip;
	int port;
} args_t;

int parse_args(int, char**, args_t&);

int main(int argc, char* argv[]) {
	_info("`udp_server` test...");

	// default argrments
	args_t args {
		"127.0.0.1",
		5555
	};

	int ret = parse_args(argc, argv, args);
	if (ret != 0) {
		return -1;
	}

	EventLoop* evloop = EventLoop::default_loop();
	SocketAddr addr(args.ip, args.port);

	UdpSocket server(evloop);
	server.set_message_callback([&server](SocketAddr& from, const char* data, unsigned size) {
		_info("get `%d` bytes msg: %s", size, data);
		server.send(from, data, size);
	});

	server.try_bind_and_read(addr);
	evloop->run();
	return 0;
}

int parse_args(int argc, char* argv[], args_t& args) {
	std::string usage = "[Usage]test_udp_client -i|--ip <x.x.x.x> -p|--port <number>";
	const char* optstr = "i:p:h";

	/*
	 * no_argument - 0
	 * required_argument - 1
	 * optional_argument - 2
	 */
	struct option opts[] = {
		{"ip", 1, NULL, 'i'},
		{"port", 1, NULL, 'p'},
		{"help", 0, NULL, 'h'},
		{0, 0, 0, 0},
	};
	
	int opt;
	while((opt = getopt_long(argc, argv, optstr, opts, NULL)) != -1){
		switch(opt) {
		case 'i':
			args.ip = optarg;
			break;
		case 'p':
			args.port = std::atoi(optarg);
			break;
		case 'h':
			fprintf(stderr, "%s\n", usage.c_str());
			return -1;
		case '?':
			if(strchr(optstr, optopt) == NULL){
				fprintf(stderr, "unknown option '-%c'\n", optopt);
			}else{
				fprintf(stderr, "option requires an argument '-%c'\n", optopt);
			}
			return -1;
		}
	}

	return 0;
}
