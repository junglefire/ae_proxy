#ifndef SMART_UDP_PROXY_H__
#define SMART_UDP_PROXY_H__

#include <functional>

#include <event_loop.h>
#include <udp_socket.h>
#include <msgpack.h>
#include <logger.h>

#include "udp_agent.h"
#include "session.h"
#include "proxy.h"

NAMESPACE_BEGIN(smart)

class UdpProxy: public Proxy
{
public:
	UdpProxy(const char* ip, int port, EventLoop* evloop = nullptr);
	~UdpProxy();
	UdpProxy(const UdpProxy&) = delete;
	UdpProxy& operator=(const UdpProxy&) = delete;
public:
	int start();
private:
	std::tuple<std::string, int, std::string> unpack(const char* buffer, int size);
	int forward(SocketAddr& from, const char* data, unsigned size);
	UdpAgent* get_agent(const char* cip, int cport, const char* sip, int sport);
private:
	std::map<std::string, UdpAgent*> session_;
private:
	std::shared_ptr<UdpSocket> server_ptr_;
};

NAMESPACE_END(smart)

#endif
