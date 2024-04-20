#include <getopt.h>

#include <cstdlib>
#include <string>
#include <atomic>

#include <tcp_server.h>
#include <logger.h>

using namespace smart;

typedef struct _args {
	std::string ip;
	int port;
} args_t;

int parse_args(int, char**, args_t&);

int main(int argc, char* argv[]) {
	_info("`tcp_server` test...");

	// default argrments
	args_t args {
		"127.0.0.1",
		4444
	};

	int ret = parse_args(argc, argv, args);
	if (ret != 0) {
		return -1;
	}

	EventLoop*evloop = EventLoop::default_loop();
	SocketAddr addr("127.0.0.1", 4444);
	std::atomic<uint64_t> dataSize(0);

	TcpServer server(evloop);
	server.set_message_callback([&dataSize](TcpConnectionPtr ptr, const char* data, ssize_t size) {
		dataSize += size;
		_info("recv: %s", data);
		ptr->write(data, size, nullptr);
	});

	server.listen(addr);
	evloop->run();
	return 0;
}

int parse_args(int argc, char* argv[], args_t& args) {
	std::string usage = "[Usage]test_tcp_client -i|--ip <x.x.x.x> -p|--port <number>";
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
