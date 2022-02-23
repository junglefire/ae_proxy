#include <string>

#include "tcp_client.h"
#include "log_writer.h"
#include "packet.h"

using namespace smart;
using namespace std;

TcpClient::TcpClient(EventLoop* loop, bool tcpNoDelay):
	loop_(loop),
	connect_(new uv_connect_t()),
	ipv(SocketAddr::Ipv4),
	tcpNoDelay_(tcpNoDelay),
	connectCallback_(nullptr),
	onMessageCallback_(nullptr),
	connection_(nullptr) {
	// constructor
	connect_->data = static_cast<void*>(this);
}

TcpClient::~TcpClient() {
	delete connect_;
}

bool TcpClient::is_tcp_no_delay() {
	return tcpNoDelay_;
}

void TcpClient::set_tcp_no_delay(bool isNoDelay) {
	tcpNoDelay_ = isNoDelay;
}

void TcpClient::create_tcp_socket() {
	socket_ = make_shared<uv_tcp_t>();
	::uv_tcp_init(loop_->handle(), socket_.get());
	if (tcpNoDelay_) {
		::uv_tcp_nodelay(socket_.get(), 1 );
	}
}

void TcpClient::connect(SocketAddr& addr) {
	create_tcp_socket();
	ipv = addr.ipv();	
	::uv_tcp_connect(connect_, socket_.get(), addr.addr(), [](uv_connect_t* req, int status) {
		auto handle = static_cast<TcpClient*>((req->data));
		if (status != 0) {
			handle->on_connect(false);
			return;
		}
		handle->on_connect(true);
	});
}

void TcpClient::on_connect(bool success) {
	if(success) {
		string name;
		SocketAddr::addr_to_str(socket_.get(), name, ipv);
		connection_ = make_shared<TcpConnection>(loop_, name, socket_);
		connection_->set_message_callback(std::bind(&TcpClient::on_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		connection_->set_connect_close_callback(std::bind(&TcpClient::on_connect_close, this, std::placeholders::_1));
		run_connect_callback(ConnectStatus::ConnectSuccess);
	} else {
		if (::uv_is_active((uv_handle_t*)socket_.get())) {
			::uv_read_stop((uv_stream_t*)socket_.get());
		}
		if (::uv_is_closing((uv_handle_t*)socket_.get()) == 0) {
			socket_->data = static_cast<void*>(this);
			::uv_close((uv_handle_t*)socket_.get(), [](uv_handle_t* handle) {
				auto client = static_cast<TcpClient*>(handle->data);
				client->after_connect_fail();
			});
		}
	}
}

void TcpClient::on_connect_close(string& name) {
	if (connection_) {
		connection_->close(std::bind(&TcpClient::on_close,this,std::placeholders::_1));
	}
}

void TcpClient::on_message(shared_ptr<TcpConnection> connection,const char* buf,ssize_t size) {
	if(onMessageCallback_) {
		onMessageCallback_(buf,size);
	}
}

void TcpClient::close(TcpClientCloseCallback callback) {
	if (connection_) {
		connection_->close([this, callback](std::string&) {
			//onClose(name);
			if (callback) {
				callback(this);
			}
		});
	} else if(callback) {
		callback(this);
	}
}

void TcpClient::after_connect_fail() {
	run_connect_callback(ConnectStatus::ConnectFail);
}

int TcpClient::write(const char* buf, unsigned int size, AfterWriteCallback callback) {
	if (connection_) {
		return connection_->write(buf, size, callback);
	} else if(callback) {
		LogWriter::instance()->warn("try write a disconnect connection.");
		WriteInfo info = {WriteInfo::Disconnected, const_cast<char*>(buf), size};
		callback(info);
	}
	return -1;
}

void TcpClient::write_in_loop(const char * buf, unsigned int size, AfterWriteCallback callback) {
	if (connection_) {
		connection_->write_in_loop(buf, size, callback);
	} else if(callback) {
		LogWriter::instance()->warn("try write a disconnect connection.");
		WriteInfo info = { WriteInfo::Disconnected,const_cast<char*>(buf),size };
		callback(info);
	}
}

void TcpClient::set_connect_status_callback(ConnectStatusCallback callback) {
	connectCallback_ = callback;
}

void TcpClient::set_message_callback(NewMessageCallback callback) {
	onMessageCallback_ = callback;
}

EventLoop* TcpClient::loop() {
	return loop_;
}

PacketBufferPtr TcpClient::get_current_buf() {
	if (connection_)
		return connection_->get_packet_buffer();
	return nullptr;
}

void TcpClient::run_connect_callback(ConnectStatus status) {
	if (connectCallback_)
		connectCallback_(status);
}

void TcpClient::on_close(std::string& name) {
	LogWriter::instance()->info("Close tcp client connection complete.");
	run_connect_callback(ConnectStatus::ConnectClose);
}
