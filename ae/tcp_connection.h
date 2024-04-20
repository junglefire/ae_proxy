#ifndef SMART_TCP_CONNECTION_H__
#define SMART_TCP_CONNECTION_H__

#include "list_buffer.h"
#include "socket_addr.h"
#include "event_loop.h"
#include "ae.h"
// #include "CycleBuffer.hpp"

NAMESPACE_BEGIN(smart)

// libuv tcp connection wrapper class
class TcpConnection: public std::enable_shared_from_this<TcpConnection> {
public :
	TcpConnection(EventLoop* loop, std::string& name, UVTcpPtr client, bool isConnected = true);
	virtual ~TcpConnection();
	TcpConnection(const TcpConnection&) = delete;
	TcpConnection& operator=(const TcpConnection&) = delete;
public:
	void on_socket_close();
	void close(std::function<void(std::string&)> callback);
	int write(const char* buf, ssize_t size, AfterWriteCallback callback);
	void write_in_loop(const char* buf, ssize_t size, AfterWriteCallback callback);
	void set_wrapper(std::shared_ptr<ConnectionWrapper> wrapper);
	std::shared_ptr<ConnectionWrapper> get_wrapper();
	void set_message_callback(OnMessageCallback callback);
	void set_connect_close_callback(OnCloseCallback callback);
	void set_connect_status(bool status);
	bool is_connected();
	const std::string& name();
	PacketBufferPtr get_packet_buffer();
private:
	void on_message(const char* buf, ssize_t size);
	void close_complete();
	char* resize_data(size_t size);
	static void on_mesage_receive(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
private:
	std::string name_;
	bool connected_;
	EventLoop* loop_;
	UVTcpPtr handle_;
	std::string data_;
	PacketBufferPtr buffer_;
	std::weak_ptr<ConnectionWrapper> wrapper_;
private:
	OnMessageCallback onMessageCallback_;
	OnCloseCallback onConnectCloseCallback_;
	CloseCompleteCallback closeCompleteCallback_;
};

class ConnectionWrapper: public std::enable_shared_from_this<ConnectionWrapper>
{
public:
	ConnectionWrapper(TcpConnectionPtr connection): connection_(connection) {
	}

	~ConnectionWrapper() {
		TcpConnectionPtr connection = connection_.lock();
		if (connection) {
			connection->on_socket_close();
		}
	}
private:
	std::weak_ptr<TcpConnection> connection_;
};

NAMESPACE_END(smart)
#endif
