#ifndef SMART_UDP_SOCKET_HPP
#define SMART_UDP_SOCKET_HPP

#include "socket_addr.h"
#include "event_loop.h"
#include "log_writer.h"
#include "ae.h"

NAMESPACE_BEGIN(smart)

class UdpSocket
{
public:
	UdpSocket(EventLoop* loop);
	virtual ~UdpSocket();
	UdpSocket(const UdpSocket&) = delete;
	UdpSocket& operator=(const UdpSocket&) = delete;
public:
	int try_bind_and_read(SocketAddr& addr);
	int try_read();
	int send(SocketAddr& to, const char* buf, unsigned size);
	void close(DefaultCallback callback);
	void set_message_callback(OnUdpMessageCallback callback);
private:
	void on_close_completed();
	void on_message(const sockaddr* from, const char* data, unsigned size);
private:
	static void on_mesage_receive(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
protected:
	uv_udp_t* handle_;
private:
	SocketAddr::IPV ipv_;
	DefaultCallback on_close_;
	OnUdpMessageCallback on_message_callback_;
};

NAMESPACE_END(smart)

#endif
