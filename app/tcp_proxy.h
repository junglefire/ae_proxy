#ifndef SMART_TCP_PROXY_H__
#define SMART_TCP_PROXY_H__

#include <tcp_server.h>
#include "tcp_agent.h"
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
	std::tuple<int, std::string, int, std::string> unpack(const char* buffer, int size);
	int connect_real_server(TcpConnectionPtr, const char*, int, std::string);
private:
	std::map<std::string, TcpAgent*> session_;
private:
	std::shared_ptr<TcpServer> server_ptr_;
	msgpack_sbuffer* msgbuffer;
	msgpack_packer* msgpacker;
};

NAMESPACE_END(smart)
#endif //SMART_TCP_PROXY_H__
