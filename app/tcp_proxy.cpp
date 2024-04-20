#include "tcp_proxy.h"

using namespace smart;

TcpProxy::TcpProxy(const char* ip, int port, EventLoop* evloop): Proxy(ip, port, evloop) {
	// constructor
	if (evloop == nullptr) {
		this->evloop_ = EventLoop::default_loop();
	} else {
		this->evloop_ = evloop;
	}
	this->msgbuffer = msgpack_sbuffer_new();
	this->msgpacker = msgpack_packer_new(this->msgbuffer, msgpack_sbuffer_write);
}

TcpProxy::~TcpProxy() {
	msgpack_sbuffer_free(this->msgbuffer);
	msgpack_packer_free(this->msgpacker);
}

int TcpProxy::start() {
	_info("tcp proxy server listen on `%s:%d`", this->ip.c_str(), this->port);
	// create tcp server instance, register inbound callback function
	this->server_ptr_ = std::make_shared<TcpServer>(this->evloop_);
	this->server_ptr_->set_message_callback([this](TcpConnectionPtr client_conn_ptr, const char* data, ssize_t size) {
		auto [command, sip, sport, message] = this->unpack(data, size);
		_debug("get message `%d`: `%s` --> `%s:%d`", command, client_conn_ptr->name().c_str(), sip.c_str(), sport);
		// client_conn_ptr->write(message.c_str(), message.size(), nullptr);
		// tcp proxy command
		// - 0x00: bind
		// - 0x01: wait for bind response
		// - 0x02: send data
		// - 0x03: sending data
		// - 0x04: unbind
		if (command == 0x00) {
			this->connect_real_server(client_conn_ptr, sip.c_str(), sport, message);
		}
		// client_conn_ptr->close(nullptr);
	});
	// tcp server bind
	this->server_ptr_->listen(this->addr_);
	return 0;
}

int TcpProxy::connect_real_server(TcpConnectionPtr client_conn_ptr, const char* ip, int port, std::string message) {
	TcpAgent* agent = new TcpAgent(client_conn_ptr->name().c_str(), ip, port, this->evloop_);
	SocketAddr real_server_addr(ip, port);

	_info("connect real server: `%s:%d`", ip, port);
	agent->set_connect_status_callback([&, agent, client_conn_ptr, message](ConnectStatus status) {
		if (status == ConnectStatus::ConnectSuccess) {
			_info("create link: `%s->%s`", agent->client_address(), agent->server_address());
			this->session_[client_conn_ptr->name()] = agent;
			client_conn_ptr->write(message.c_str(), message.size(), nullptr);
		} else {
			_error("error : connect to real server fail");
			client_conn_ptr->close(nullptr);
		}
	});
	agent->connect(real_server_addr);
	return 0;
}

std::tuple<int, std::string, int, std::string> TcpProxy::unpack(const char* buffer, int size) {
	size_t offset = 0;
	char unpacked_buffer[UNPACKED_BUFFER_SIZE];
	msgpack_unpacked result;
	msgpack_unpacked_init(&result);

	int command;
	std::string ip;
	int port;
	std::string message;

	int i = 0;
	msgpack_unpack_return ret = msgpack_unpack_next(&result, buffer, size, &offset);
	while (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = result.data;
		switch (i) {
		case 0: // command
			command = obj.via.i64;
			break;
		case 1: // real server ip
			ip.assign(obj.via.str.ptr, obj.via.str.size);
			break;
		case 2: // real server port
			port = obj.via.i64;
			break;
		case 3: // message
			message.assign(obj.via.str.ptr, obj.via.str.size);
			break;
		}	
		i++;
		ret = msgpack_unpack_next(&result, buffer, size, &offset);
	}
	// debug("IP: `%s:%d`, Message: %s", ip.c_str(), port, message.c_str());
	msgpack_unpacked_destroy(&result);
	return std::make_tuple(command, ip, port, message);
}



