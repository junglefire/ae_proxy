#include "tcp_connection.h"
#include "async_task.h"
#include "log_writer.h"
// #include "include/GlobalConfig.hpp"

using namespace smart;

struct WriteReq {
	uv_write_t req;
	uv_buf_t buf;
	AfterWriteCallback callback;
};

struct WriteArgs
{
	WriteArgs(std::shared_ptr<TcpConnection> conn = nullptr, const char* buf = nullptr, ssize_t size = 0, AfterWriteCallback callback = nullptr): 
		connection(conn), 
		buf(buf), 
		size(size), 
		callback(callback) {
		// constructor
	}
	
	std::shared_ptr<TcpConnection> connection;
	const char* buf;
	ssize_t size;
	AfterWriteCallback callback;
};

TcpConnection:: ~TcpConnection() {
}

TcpConnection::TcpConnection(EventLoop* loop, std::string& name, UVTcpPtr client, bool isConnected): 
	name_(name), 
	connected_(isConnected), 
	loop_(loop), 
	handle_(client), 
	buffer_(nullptr), 
	onMessageCallback_(nullptr), 
	onConnectCloseCallback_(nullptr), 
	closeCompleteCallback_(nullptr) {
	// constructor
	handle_->data = static_cast<void*>(this);
	
	::uv_read_start((uv_stream_t*)handle_.get(), [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
		auto conn = static_cast<TcpConnection*>(handle->data);
		buf->base = conn->resize_data(suggested_size);
		buf->len = suggested_size;
	}, &TcpConnection::on_mesage_receive);

	if (GlobalConfig::BufferModeStatus == GlobalConfig::ListBuffer) {
		buffer_ = std::make_shared<ListBuffer>();
	} 
	/*
	else if(GlobalConfig::BufferModeStatus == GlobalConfig::CycleBuffer) {
		buffer_ = std::make_shared<CycleBuffer>();
	}*/
}

void TcpConnection::on_message(const char* buf, ssize_t size) {
	if (onMessageCallback_) {
		onMessageCallback_(shared_from_this(), buf, size);
	}
}

void TcpConnection::on_socket_close() {
	if (onConnectCloseCallback_) {
		onConnectCloseCallback_(name_);
	}
}

void TcpConnection::close(std::function<void(std::string&)> callback) {
	onMessageCallback_ = nullptr;
	onConnectCloseCallback_ = nullptr;
	closeCompleteCallback_ = nullptr;

	closeCompleteCallback_ = callback;
	uv_tcp_t* ptr = handle_.get();
	if (::uv_is_active((uv_handle_t*)ptr)) {
		::uv_read_stop((uv_stream_t*)ptr);
	}

	if (::uv_is_closing((uv_handle_t*)ptr) == 0) {
		// libuv在loop轮询中会检测关闭句柄，delete会导致程序异常退出。
		::uv_close((uv_handle_t*)ptr, [](uv_handle_t* handle) {
			auto connection = static_cast<TcpConnection*>(handle->data);
			connection->close_complete();
		});
	} else {
		close_complete();
	}
}

int TcpConnection::write(const char* buf, ssize_t size, AfterWriteCallback callback) {
	int rst;
	if (connected_) {
		WriteReq* req = new WriteReq();
		req->buf = uv_buf_init(const_cast<char*>(buf), static_cast<unsigned int>(size));
		req->callback = callback;
		auto ptr = handle_.get();
		rst = ::uv_write((uv_write_t*)req, (uv_stream_t*)ptr, &req->buf, 1, [](uv_write_t* req, int status) {
			WriteReq* wr = (WriteReq*)req;
			if (nullptr != wr->callback) {
				struct WriteInfo info;
				info.buf = const_cast<char*>(wr->buf.base);
				info.size = wr->buf.len;
				info.status = status;
				wr->callback(info);
			}
			delete wr;
		});

		if (rst != 0) {
			LogWriter::instance()->error(std::string("write data error:" + std::to_string(rst)));
			if (callback != nullptr) {
				struct WriteInfo info = { rst,const_cast<char*>(buf),static_cast<unsigned long>(size) };
				callback(info);
			}
			delete req;
		}
	} else {
		rst = -1;
		if (callback != nullptr) {
			struct WriteInfo info = { 
				WriteInfo::Disconnected,
				const_cast<char*>(buf),
				static_cast<unsigned long>(size) 
			};
			callback(info);
		}
	}
	return rst;
}

void TcpConnection::write_in_loop(const char* buf, ssize_t size, AfterWriteCallback callback) {
	std::weak_ptr<TcpConnection> conn = shared_from_this();
	loop_->run_in_this_loop([conn,buf,size, callback]() {
		std::shared_ptr<TcpConnection> ptr = conn.lock();
		if (ptr != nullptr) {
			ptr->write(buf, size, callback);
		} else {
			struct WriteInfo info = { WriteInfo::Disconnected,const_cast<char*>(buf),static_cast<unsigned long>(size) };
			callback(info);
		}
	});
}

void TcpConnection::set_wrapper(ConnectionWrapperPtr wrapper) {
	wrapper_ = wrapper;
}

std::shared_ptr<ConnectionWrapper> TcpConnection::get_wrapper() {
	return wrapper_.lock();
}

void  TcpConnection::on_mesage_receive(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
	auto connection = static_cast<TcpConnection*>(client->data);
	if (nread > 0) {
		connection->on_message(buf->base, nread);
	} else if (nread < 0) {
		connection->set_connect_status(false);
		LogWriter::instance()->error(uv_err_name((int)nread));

		if (nread != UV_EOF) {
			connection->on_socket_close();
			return;
		}

		uv_shutdown_t* sreq = new uv_shutdown_t();
		sreq->data = static_cast<void*>(connection);
		::uv_shutdown(sreq, (uv_stream_t*)client, [](uv_shutdown_t* req, int status) {
			auto connection = static_cast<TcpConnection*>(req->data);
			connection->on_socket_close();
			delete req;
		});
	} else {
		/* Everything OK, but nothing read. */
	}
}

void TcpConnection::set_message_callback(OnMessageCallback callback) {
	onMessageCallback_ = callback;
}

void TcpConnection::set_connect_close_callback(OnCloseCallback callback) {
	onConnectCloseCallback_ = callback;
}

void TcpConnection::close_complete() {
	if (closeCompleteCallback_) {
		closeCompleteCallback_(name_);
	}
}

void TcpConnection::set_connect_status(bool status) {
	connected_ = status;
}

bool TcpConnection::is_connected() {
	return connected_;
}

const std::string& TcpConnection::name() {
	return name_;
}

char* TcpConnection::resize_data(size_t size) {
	data_.resize(size);
	return const_cast<char*>(data_.c_str());
}

PacketBufferPtr TcpConnection::get_packet_buffer() {
	return buffer_;
}
