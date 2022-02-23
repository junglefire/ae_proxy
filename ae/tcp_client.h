#ifndef   SMART_TCP_CLIENT_HPP
#define   SMART_TCP_CLIENT_HPP

#include "tcp_connection.h"
#include "socket_addr.h"
#include "ae.h"

NAMESPACE_BEGIN(smart)

// libuv tcp client wrapper class
class TcpClient {
public:
	TcpClient(EventLoop* loop, bool tcpNoDelay = true);
	virtual ~TcpClient();
	TcpClient(const TcpClient&) = delete;
	TcpClient& operator=(const TcpClient&) = delete;
public:
	bool is_tcp_no_delay();
	void set_tcp_no_delay(bool isNoDelay);
	void connect(SocketAddr& addr);
	void close(TcpClientCloseCallback callback);
	void set_connect_status_callback(ConnectStatusCallback callback);
	void set_message_callback(NewMessageCallback callback);
public:
	int write(const char* buf, unsigned int size, AfterWriteCallback callback = nullptr);
	void write_in_loop(const char* buf, unsigned int size, AfterWriteCallback callback);
public:
	EventLoop* loop();
	PacketBufferPtr get_current_buf();
protected:
	EventLoop* loop_;
protected:
	void on_message(TcpConnectionPtr connection, const char* buf, ssize_t size);
	void on_connect_close(std::string& name);
	void on_connect(bool success);
	void after_connect_fail();
private:
	void run_connect_callback(ConnectStatus success);
	void on_close(std::string& name);
	void create_tcp_socket();
private:
	TcpConnectionPtr connection_;
	uv_connect_t* connect_;
	SocketAddr::IPV ipv;
	UVTcpPtr socket_;
	bool tcpNoDelay_;
private:
	ConnectStatusCallback connectCallback_;
	NewMessageCallback onMessageCallback_;
};

NAMESPACE_END(smart)
#endif
