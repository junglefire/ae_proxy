#include "tcp_acceptor.h"
#include "log_writer.h"

using namespace smart;
using namespace std;

TcpAcceptor::TcpAcceptor(EventLoop* loop, bool tcpNoDelay):
	listened_(false),
	tcpNoDelay_(tcpNoDelay),
	loop_(loop),
	callback_(nullptr),
	onCloseCompletCallback_(nullptr) {
	// constructor
	::uv_tcp_init(loop_->handle(), &server_);
	if (tcpNoDelay_) {
		::uv_tcp_nodelay(&server_, 1);
	}
	server_.data = (void* )this;
}

TcpAcceptor:: ~TcpAcceptor() {
}

EventLoop* TcpAcceptor::loop() {
	return loop_;
}

void TcpAcceptor::on_new_connect(UVTcpPtr client) {
	if(nullptr != callback_) {
		callback_(loop_,client);
	}
}

void TcpAcceptor::on_close_complete() {
	if (onCloseCompletCallback_) {
		onCloseCompletCallback_();
	}
}

int TcpAcceptor::bind(SocketAddr& addr) {
	return ::uv_tcp_bind(&server_, addr.addr(), 0);
}

int TcpAcceptor::listen() {
	auto rst = ::uv_listen((uv_stream_t*) &server_, 128, [](uv_stream_t *server, int status) {
		if (status < 0) {
			LogWriter::instance()->error (std::string("New connection error :")+ EventLoop::get_error_message(status));
			return;
		}
		TcpAcceptor* accept = static_cast<TcpAcceptor*>(server->data);
		UVTcpPtr client = make_shared<uv_tcp_t>();
		::uv_tcp_init(accept->loop()->handle(), client.get());
		if (accept->is_tcp_no_delay()) {
			::uv_tcp_nodelay(client.get(), 1);
		}
		
		if ( 0 == ::uv_accept(server, (uv_stream_t*) client.get())) {
			accept->on_new_connect(client);
		} else {
			::uv_close((uv_handle_t*) client.get(), NULL);
		}
	});

	if (rst == 0) {
		listened_ = true;
	}
	return rst;
}

bool TcpAcceptor::is_listen() {
	return listened_;
}

void TcpAcceptor::close(DefaultCallback callback) {
	onCloseCompletCallback_ = callback;
	auto ptr = &server_;
	if (::uv_is_active((uv_handle_t*)ptr)) {
		::uv_read_stop((uv_stream_t*)ptr);
	}
	if (::uv_is_closing((uv_handle_t*)ptr) == 0) {
		// libuv在loop轮询中会检测关闭句柄，delete会导致程序异常退出。
		::uv_close((uv_handle_t*)ptr, [](uv_handle_t* handle) {
			auto accept = static_cast<TcpAcceptor*>(handle->data);
			accept->on_close_complete();
		});
	} else {
		on_close_complete();
	}
}

bool TcpAcceptor::is_tcp_no_delay() {
	return tcpNoDelay_;
}

void TcpAcceptor::set_new_connection_callback(NewConnectionCallback callback) {
	callback_ = callback;
}
