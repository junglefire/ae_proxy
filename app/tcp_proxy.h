#ifndef SMART_TCP_PROXY_H__
#define SMART_TCP_PROXY_H__

#include <tcp_server.h>
#include "proxy.h"

NAMESPACE_BEGIN(smart)

class TcpProxy: public Proxy
{
public:
	TcpProxy(const char* ip, int port, EventLoop* evloop = nullptr);
	~TcpProxy();
	TcpProxy(const TcpProxy&) = delete;
	TcpProxy& operator=(const TcpProxy&) = delete;
public:
	int start();
private:
	std::shared_ptr<TcpServer> server_ptr_;
};

NAMESPACE_END(smart)
#endif //SMART_TCP_PROXY_H__
