#ifndef SMART_UDP_AGENT_H__
#define SMART_UDP_AGENT_H__

#include <event_loop.h>
#include <udp_socket.h>
#include <logger.h>

NAMESPACE_BEGIN(smart)

#define OBSOLETED 1

class UdpAgent: public UdpSocket
{
public:
	UdpAgent(const char* cip, int cport, const char* sip, int sport, EventLoop* loop);
	~UdpAgent();
	UdpAgent(const UdpAgent&) = delete;
	UdpAgent& operator=(const UdpAgent&) = delete;
public:
	int send_to_server(const char* buf, unsigned size);
	int send_to_client(const char* buf, unsigned size);
private:
	SocketAddr ca_;
	SocketAddr sa_;
private: //obsoleted functions
#ifdef OBSOLETED
	int send_to_server2(const char* buf, unsigned size);
	int send_to_client2(const char* buf, unsigned size);
#endif
};

NAMESPACE_END(smart)

#endif
