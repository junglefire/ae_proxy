#include <getopt.h>

#include <cstdlib>
#include <string>

#include <tcp_client.h>
#include <logger.h>

using namespace smart;

typedef struct _args {
	std::string ip;
	int port;
} args_t;

int parse_args(int, char**, args_t&);

int main(int argc, char* argv[]) {
	_info("`tcp_client` test...");

	// default argrments
	args_t args {
		"127.0.0.1",
		4444
	};

	int ret = parse_args(argc, argv, args);
	if (ret != 0) {
		return -1;
	}

	// 这里有错，`EventLoop::DefaultLoop()`返回单体
	std::shared_ptr<EventLoop> evloop(EventLoop::default_loop());
	SocketAddr addr("127.0.0.1", 4444);

	TcpClient client(evloop.get());
	client.set_connect_status_callback([&client](ConnectStatus status) {
		if (status == ConnectStatus::ConnectSuccess) {
			_info("connect server ok");
			char data[4096] = "hello world!";
			client.write(data, sizeof(data));
		} else {
			_error("error : connect to server fail");
		}
	});
	
	client.set_message_callback([&client](const char* data,ssize_t size) {
		client.write(data,(unsigned)size,nullptr);
	});
	client.connect(addr);

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
