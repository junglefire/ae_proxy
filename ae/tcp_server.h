#ifndef SMART_TCP_SERVER_H__
#define SMART_TCP_SERVER_H__

#include "tcp_connection.h"
#include "tcp_acceptor.h"
#include "timer_wheel.h"
#include "ae.h"

NAMESPACE_BEGIN(smart)

// no thread safe.
class TcpServer {
public:
	static void set_buffer_mode(GlobalConfig::BufferMode mode);
public:
	TcpServer(EventLoop* loop, bool tcpNoDelay = true);
	~TcpServer();
	TcpServer(const TcpServer&) = delete;
	TcpServer& operator=(const TcpServer&) = delete;
public:
	int listen(SocketAddr& addr);
	void close(DefaultCallback callback);
	TcpConnectionPtr get_connection(const std::string& name);
	void close_connection(const std::string& name);
public:
	void set_new_connect_callback(OnConnectionStatusCallback callback);
	void set_connect_close_callback(OnConnectionStatusCallback callback);
	void set_message_callback(OnMessageCallback callback);
public:
	void write(TcpConnectionPtr connection,const char* buf,unsigned int size, AfterWriteCallback callback = nullptr);
	void write(std::string& name,const char* buf,unsigned int size, AfterWriteCallback callback =nullptr);
	void write_in_loop(TcpConnectionPtr connection,const char* buf,unsigned int size,AfterWriteCallback callback);
	void write_in_loop(std::string& name,const char* buf,unsigned int size,AfterWriteCallback callback);
public:
	void set_timeout(unsigned int);
private:
	void on_accept(EventLoop* loop, UVTcpPtr client);
	void add_connection(std::string& name, TcpConnectionPtr connection);
	void remove_connection(std::string& name);
	void on_message(TcpConnectionPtr connection, const char* buf, ssize_t size);
protected:
	EventLoop* loop_;
private:
	bool tcpNoDelay_;
	SocketAddr::IPV ipv_;
	std::shared_ptr<TcpAcceptor> accetper_;
	std::map<std::string ,TcpConnectionPtr> connnections_;
	OnMessageCallback onMessageCallback_;
	OnConnectionStatusCallback onNewConnectCallback_;
	OnConnectionStatusCallback onConnectCloseCallback_;
	TimerWheel<ConnectionWrapper> timerWheel_;
};

NAMESPACE_END(smart)
#endif
