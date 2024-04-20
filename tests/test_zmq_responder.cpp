#include <getopt.h>

#include <cstdlib>
#include <string>

#include <cxxopts.hpp>
#include <logger.h>
#include <zmq.h>

using namespace cxxopts;

int main(int argc, char* argv[]) {
	ParseResult args;
	Options options("test_zmq_responder", "zmq responder");

	try {
		options.add_options()
			("a,address", "zmq responder address", value<std::string>()->default_value("tcp://*:4444"))
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

	_info("`zmq responder` test...");

	void* context = zmq_ctx_new();
	void* responder = zmq_socket(context, ZMQ_REP);
	int rc = zmq_bind(responder, address.c_str());
	assert (rc == 0);

	while (1) {
		char buffer [10];
		memset(buffer, 0, sizeof(buffer));
		zmq_recv(responder, buffer, 10, 0);
		_info("recv message: %s", buffer);
		sleep(1);
		zmq_send(responder, "pong", 4, 0);
	}
	zmq_close(responder);
    zmq_ctx_destroy(context);
	return 0;
}
