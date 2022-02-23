#include <functional>
#include <memory>
#include <string>

#include "tcp_server.h"
#include "log_writer.h"

using namespace std;
using namespace smart;

void TcpServer::set_buffer_mode(GlobalConfig::BufferMode mode) {
	GlobalConfig::BufferModeStatus = mode;
}

TcpServer::TcpServer(EventLoop* loop, bool tcpNoDelay):
	loop_(loop),
	tcpNoDelay_(tcpNoDelay),
	accetper_(nullptr),
	onMessageCallback_(nullptr),
	onNewConnectCallback_(nullptr),
	timerWheel_(loop),
	onConnectCloseCallback_(nullptr) {
	// constructor
	// pass
}

TcpServer:: ~TcpServer() {
	// pass
}

void TcpServer::set_timeout(unsigned int seconds) {
	timerWheel_.set_timeout(seconds);
}

void TcpServer::on_accept(EventLoop* loop, UVTcpPtr client) {
	string key;
	SocketAddr::addr_to_str(client.get(), key, ipv_);
	LogWriter::instance()->debug("new connect  " + key);
	shared_ptr<TcpConnection> connection(new TcpConnection(loop, key, client));

	if (connection) {
		connection->set_message_callback(std::bind(&TcpServer::on_message, this, placeholders::_1, placeholders::_2, placeholders::_3));
		connection->set_connect_close_callback(std::bind(&TcpServer::close_connection, this, placeholders::_1));
		add_connection(key, connection);
		if (timerWheel_.get_timeout() > 0) {
			auto wrapper = std::make_shared<ConnectionWrapper>(connection);
			connection->set_wrapper(wrapper);
			timerWheel_.insert(wrapper);
		}
		if (onNewConnectCallback_) {
			onNewConnectCallback_(connection);
		}
	} else {
		LogWriter::instance()->error("create connection fail. :" + key);
	}
}

int TcpServer::listen(SocketAddr& addr) {
	ipv_ = addr.ipv();
	accetper_ = std::make_shared<TcpAcceptor>(loop_, tcpNoDelay_);
	auto rst = accetper_->bind(addr);
	if (0 != rst) {
		return rst;
	}
	accetper_->set_new_connection_callback(std::bind(&TcpServer::on_accept, this, std::placeholders::_1, std::placeholders::_2));
	timerWheel_.start();
	return accetper_->listen();
}

void TcpServer::close(DefaultCallback callback) {
	if (accetper_) {
		accetper_->close([this, callback]() {
			for (auto& connection : connnections_) {
				connection.second->on_socket_close();
			}
			callback();
		});
	}
}

void TcpServer::on_message(TcpConnectionPtr connection,const char* buf,ssize_t size) {
	if(onMessageCallback_) {
		onMessageCallback_(connection,buf,size);
	}

	if (timerWheel_.get_timeout() > 0) {
		timerWheel_.insert(connection->get_wrapper());
	}
}

void TcpServer::set_message_callback(OnMessageCallback callback) {
	onMessageCallback_ = callback;
}

void TcpServer::write(shared_ptr<TcpConnection> connection,const char* buf,unsigned int size, AfterWriteCallback callback) {
	if(connection != nullptr) {
		connection->write(buf,size, callback);
	} else if (callback) {
		WriteInfo info = { WriteInfo::Disconnected,const_cast<char*>(buf),size };
		callback(info);
	}
}

void TcpServer::write(string& name, const char* buf,unsigned int size,AfterWriteCallback callback) {
	auto connection = get_connection(name);
	write(connection, buf, size, callback);
}

void TcpServer::write_in_loop(shared_ptr<TcpConnection> connection,const char* buf,unsigned int size,AfterWriteCallback callback) {
	if(connection != nullptr) {
		connection->write_in_loop(buf,size,callback);
	} else if (callback) {
		LogWriter::instance()->warn("try write a disconnect connection.");
		WriteInfo info = { WriteInfo::Disconnected,const_cast<char*>(buf),size };
		callback(info);
	}
}

void TcpServer::write_in_loop(string& name,const char* buf,unsigned int size,AfterWriteCallback callback) {
	auto connection = get_connection(name);
	write_in_loop(connection, buf, size, callback);
}

void TcpServer::set_new_connect_callback(OnConnectionStatusCallback callback) {
	onNewConnectCallback_ = callback;
}

void  TcpServer::set_connect_close_callback(OnConnectionStatusCallback callback) {
	onConnectCloseCallback_ = callback;
}

void TcpServer::add_connection(std::string& name, TcpConnectionPtr connection) {
	connnections_.insert(pair<string,shared_ptr<TcpConnection>>(std::move(name),connection));
}

void TcpServer::remove_connection(string& name) {
	connnections_.erase(name);
}

shared_ptr<TcpConnection> TcpServer::get_connection(const string &name) {
	auto rst = connnections_.find(name);
	if(rst == connnections_.end()) {
		return nullptr;
	}
	return rst->second;
}

void TcpServer::close_connection(const string& name) {
	auto connection = get_connection(name);
	if (connection != nullptr) {
		connection->close([this](std::string& name) {
			auto connection = get_connection(name);
			if (nullptr != connection) {
				if (onConnectCloseCallback_) {
					onConnectCloseCallback_(connection);
				}
				remove_connection(name);
			}
		});
	}
}
