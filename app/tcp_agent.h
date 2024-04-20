#ifndef SMART_TCP_AGENT_H__
#define SMART_TCP_AGENT_H__

#include <event_loop.h>
#include <tcp_client.h>
#include <logger.h>

NAMESPACE_BEGIN(smart)

class TcpAgent: public TcpClient
{
public:
	TcpAgent(const char*, const char*, int, EventLoop* evloop);
	~TcpAgent();
	TcpAgent(const TcpAgent&) = delete;
	TcpAgent& operator=(const TcpAgent&) = delete;
public:
	const char* client_address();
	const char* server_address();
private:
	SocketAddr sa_;
	std::string client_address_;
	std::string server_address_;
};

NAMESPACE_END(smart)

#endif
