#include "udp_socket.h"

using namespace smart;

UdpSocket::UdpSocket(EventLoop* loop):
	handle_(new uv_udp_t()),
	on_message_callback_(nullptr) {
	// constructor
	::uv_udp_init(loop->handle(), handle_);
	handle_->data = this;
}

UdpSocket::~UdpSocket() {
	delete handle_;
}

// act as `udp server`
int UdpSocket::try_bind_and_read(SocketAddr& addr) {
	ipv_ = addr.ipv();
	auto rst = uv_udp_bind(handle_, addr.addr(), 0);
	if (0 != rst) {
		return rst;
	}
	return ::uv_udp_recv_start(handle_, [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
		buf->base = new char[suggested_size];
		buf->len = suggested_size;
	}, &UdpSocket::on_mesage_receive);
}

// act as `udp client`
int UdpSocket::try_read() {
	return ::uv_udp_recv_start(handle_, [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
		buf->base = new char[suggested_size];
		buf->len = suggested_size;
	}, &UdpSocket::on_mesage_receive);
}

int UdpSocket::send(SocketAddr& to, const char* buf, unsigned size) {
	uv_udp_send_t* sendHandle = new uv_udp_send_t();
	const uv_buf_t uvbuf = uv_buf_init(const_cast<char*>(buf), size);
	return ::uv_udp_send(sendHandle, handle_, &uvbuf, 1, to.addr(), [](uv_udp_send_t* handle, int status) {
		if (status) {
			std::string err_info("udp send error :");
			err_info += EventLoop::get_error_message(status);
			LogWriter::instance()->error(err_info);
		}
		delete handle;
	});
}

void UdpSocket::close(DefaultCallback callback) {
	on_close_ = callback; 
	if (uv_is_active((uv_handle_t*)handle_)) {
		uv_udp_recv_stop(handle_);
	}
	if (uv_is_closing((uv_handle_t*)handle_) == 0) {
		::uv_close((uv_handle_t*)handle_, [](uv_handle_t* handle) {
			UdpSocket* ptr = static_cast<UdpSocket*>(handle->data);
			ptr->on_close_completed();
		});
	} else {
		on_close_completed();
	}
}

void UdpSocket::on_close_completed() {
	if (on_close_) {
		on_close_();
	}
}

void UdpSocket::on_message(const sockaddr* from, const char* data, unsigned size) {
	if (on_message_callback_ != nullptr) {
		SocketAddr addr(from, ipv_);
		on_message_callback_(addr, data, size);
	}
}

void UdpSocket::set_message_callback(OnUdpMessageCallback callback) {
	on_message_callback_ = callback;
}

void UdpSocket::on_mesage_receive(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const sockaddr* addr, unsigned flags) {
	if (nread < 0) {
		std::string info("udp read error :");
		info += EventLoop::get_error_message((int)nread);
		LogWriter::instance()->error(info);
	} else if(nread >0) {
		UdpSocket* obj = static_cast<UdpSocket*>(handle->data);
		obj->on_message(addr, buf->base, (unsigned)nread);
	} else {
	}
	delete[](buf->base);
}

