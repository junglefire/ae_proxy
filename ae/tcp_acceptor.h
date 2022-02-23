#ifndef SMART_TCP_ACCEPTER_H__
#define SMART_TCP_ACCEPTER_H__

#include "ae.h"
#include "event_loop.h"
#include "socket_addr.h"

NAMESPACE_BEGIN(smart)

class TcpAcceptor {
public:
	TcpAcceptor(EventLoop* loop, bool tcpNoDelay);
	virtual ~TcpAcceptor();
	TcpAcceptor(const TcpAcceptor&) = delete;
	TcpAcceptor& operator=(const TcpAcceptor&) = delete;
public:
	int bind(SocketAddr& addr);
	int listen();
	bool is_listen();
	void close(DefaultCallback callback);
	bool is_tcp_no_delay();
	void set_new_connection_callback(NewConnectionCallback callback);
	EventLoop* loop();
private:
	bool listened_;
	bool tcpNoDelay_;
	EventLoop* loop_;
	NewConnectionCallback callback_;
	DefaultCallback onCloseCompletCallback_;
	uv_tcp_t server_;
	void on_new_connect(UVTcpPtr client);
	void on_close_complete();
};

NAMESPACE_END(smart)
#endif
