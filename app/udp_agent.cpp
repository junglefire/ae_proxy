#include "udp_agent.h"

using namespace smart;

UdpAgent::UdpAgent(const char* cip, int cport, const char* sip, int sport, EventLoop* evloop):
	UdpSocket(evloop),
	ca_(cip, cport),
	sa_(sip, sport) {
	// constructor
}

UdpAgent::~UdpAgent() {
}

int UdpAgent::send_to_server(const char* buf, unsigned size) {
	_info("send to real server `%d` bytes: %s", size, buf);
	return this->send(this->sa_, buf, size);
}

int UdpAgent::send_to_client(const char* buf, unsigned size) {
	_info("send to client `%d` bytes: %s", size, buf);
	return this->send(this->ca_, buf, size);
}

#ifdef OBSOLETED
int UdpAgent::send_to_server2(const char* buf, unsigned size) {
	_info("send to real server `%d` bytes: %s", size, buf);
	uv_udp_send_t* sendHandle = new uv_udp_send_t();
	const uv_buf_t uvbuf = uv_buf_init(const_cast<char*>(buf), size);
	return ::uv_udp_send(sendHandle, handle_, &uvbuf, 1, this->sa_.addr(), [](uv_udp_send_t* handle, int status) {
		if (status) {
			_error("udp send failed, err: %s", EventLoop::get_error_message(status));
		}
		delete handle;
	});
}

int UdpAgent::send_to_client2(const char* buf, unsigned size) {
	_info("send to client `%d` bytes: %s", size, buf);
	uv_udp_send_t* sendHandle = new uv_udp_send_t();
	const uv_buf_t uvbuf = uv_buf_init(const_cast<char*>(buf), size);
	return ::uv_udp_send(sendHandle, handle_, &uvbuf, 1, this->ca_.addr(), [](uv_udp_send_t* handle, int status) {
		if (status) {
			_error("udp send failed, err: %s", EventLoop::get_error_message(status));
		}
		delete handle;
	});
}
#endif //OBSOLETED