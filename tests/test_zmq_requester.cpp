#include <getopt.h>

#include <cstdlib>
#include <string>

#include <cxxopts.hpp>
#include <logger.h>
#include <zmq.h>

using namespace cxxopts;

int main(int argc, char* argv[]) {
	_info("`zmq requester` test...");

	ParseResult args;
	Options options("test_zmq_requester", "zmq requester");

	try {
		options.add_options()
			("a,address", "zmq responder address", value<std::string>()->default_value("tcp://127.0.0.1:4444"))
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

	std::string address = args["address"].as<std::string>();

	void *context = zmq_ctx_new();
	void *requester = zmq_socket(context, ZMQ_REQ);
	zmq_connect(requester, address.c_str());

	int request_nbr;
	for (request_nbr = 0; request_nbr != 10; request_nbr++) {
		char buffer [10];
		memset(buffer, 0, sizeof(buffer));
		_info("send message: %d", request_nbr);
		zmq_send(requester, "ping", 4, 0);
		zmq_recv(requester, buffer, 10, 0);
		_info("recv message: %s", buffer);
	}
	zmq_close(requester);
	zmq_ctx_destroy(context);
	return 0;
}
