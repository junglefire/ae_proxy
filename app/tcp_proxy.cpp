#include "tcp_proxy.h"

using namespace smart;

TcpProxy::TcpProxy(const char* ip, int port, EventLoop* evloop): Proxy(ip, port, evloop) {
	// constructor
	if (evloop == nullptr) {
		this->evloop_ = EventLoop::default_loop();
	} else {
		this->evloop_ = evloop;
	}
}

TcpProxy::~TcpProxy() {
}

int TcpProxy::start() {
	_info("tcp proxy server listen on `%s:%d`", this->ip.c_str(), this->port);
	// create tcp server instance, register inbound callback function
	this->server_ptr_ = std::make_shared<TcpServer>(this->evloop_);
	this->server_ptr_->set_message_callback([](TcpConnectionPtr client_conn_ptr, const char* data, ssize_t size) {
		_info("recv: %s", data);
		client_conn_ptr->write(data, size, nullptr);
	});
	// tcp server bind
	this->server_ptr_->listen(this->addr_);
	return 0;
}


