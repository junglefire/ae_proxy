#include "udp_proxy.h"

using namespace smart;

UdpProxy::UdpProxy(const char* ip, int port, EventLoop* evloop): Proxy(ip, port, evloop) {
	// constructor
	if (evloop == nullptr) {
		this->evloop_ = EventLoop::default_loop();
	} else {
		this->evloop_ = evloop;
	}
}

UdpProxy::~UdpProxy() {
	for (auto [k, v]: this->session_) {
		delete v;
	}
}

int UdpProxy::start() {
	_info("udp proxy server listen on `%s:%d`", this->ip.c_str(), this->port);
	
	this->server_ptr_ = std::make_shared<UdpSocket>(this->evloop_);
	this->server_ptr_->set_message_callback([this](SocketAddr& from, const char* data, unsigned size) {
		_info("get `%d` bytes from client `%s` msg: %s", size, from.to_string().c_str(), data);
		this->forward(from, data, size);
	});

	this->server_ptr_->try_bind_and_read(this->addr_);
	return 0;
}

int UdpProxy::forward(SocketAddr& from, const char* data, unsigned size) {
	auto [sip, sport, message] = this->unpack(data, size);
	UdpAgent* agent = this->get_agent(from.ip().c_str(), from.port(), sip.c_str(), sport);
	agent->send_to_server(message.c_str(), message.size());
	return 0;
}

UdpAgent* UdpProxy::get_agent(const char* cip, int cport, const char* sip, int sport) {
	UdpAgent* agent = nullptr;
	std::string key = cip;
	key += ":"+std::to_string(cport)+"@"+sip+":"+std::to_string(sport);
	// c++20
	// std::string key = std::format("{}:{}@{}:{}", cip, std::to_string(cport), sip, std::to_string(sport));
	if (this->session_.find(key) == this->session_.end()) {
		_info("create udp agent, session key `%s`", key.c_str());
		agent = new UdpAgent(cip, cport, sip, sport, this->evloop_); 
		agent->set_message_callback([agent](SocketAddr& from, const char* data, unsigned size) {
			_info("get `%d` bytes from real server `%s` msg: %s", size, from.to_string().c_str(), data);
			agent->send_to_client(data, size);
		});
		agent->try_read();
		this->session_[key] = agent;
	} else {
		agent = this->session_[key];
	}
	return agent;
}

std::tuple<std::string, int, std::string> UdpProxy::unpack(const char* buffer, int size) {
	size_t offset = 0;
	char unpacked_buffer[UNPACKED_BUFFER_SIZE];
	
	msgpack_unpacked result;
	msgpack_unpacked_init(&result);

	std::string ip;
	int port;
	std::string message;

	int i = 0;
	msgpack_unpack_return ret = msgpack_unpack_next(&result, buffer, size, &offset);
	while (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = result.data;
		switch (i) {
		case 0: // real server ip
			ip.assign(obj.via.str.ptr, obj.via.str.size);
			break;
		case 1: // real server port
			port = obj.via.i64;
			break;
		case 2: // message
			message.assign(obj.via.str.ptr, obj.via.str.size);
			break;
		}	
		i++;
		ret = msgpack_unpack_next(&result, buffer, size, &offset);
	}
	// debug("IP: `%s:%d`, Message: %s", ip.c_str(), port, message.c_str());
	msgpack_unpacked_destroy(&result);
	return std::make_tuple(ip, port, message);
}

