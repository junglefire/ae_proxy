#include "tcp_agent.h"

using namespace smart;

TcpAgent::TcpAgent(const char* client_address, const char* sip, int sport, EventLoop* evloop):
	TcpClient(evloop),
	sa_(sip, sport),
	client_address_(client_address) {
	// constructor
	this->server_address_ = std::string(sip) + ":" + std::to_string(sport);
}

TcpAgent::~TcpAgent() {
}

const char* TcpAgent::client_address() {
	return this->client_address_.c_str();
}

const char* TcpAgent::server_address() {
	return this->server_address_.c_str();
}


