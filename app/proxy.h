#ifndef SMART_PROXY_H__
#define SMART_PROXY_H__

#include <socket_addr.h>
#include <event_loop.h>
#include <msgpack.h>
#include <logger.h>

NAMESPACE_BEGIN(smart)

#define UNPACKED_BUFFER_SIZE	2048

class Proxy
{
public:
	Proxy(const char* ip, int port, EventLoop* evloop): addr_(ip, port), ip(ip), port(port) {
	};
	virtual ~Proxy() {
	};
	Proxy(const Proxy&) = delete;
	Proxy& operator=(const Proxy&) = delete;
public:
	virtual int start() = 0;
protected:
	EventLoop* evloop_;
	SocketAddr addr_;
	std::string ip;
	int port;
};

NAMESPACE_END(smart)
#endif //SMART_PROXY_H__
