// #include "include/TcpConnection.hpp"
#include "event_loop.h"
#include "async_task.h"

using namespace smart;

EventLoop::EventLoop(): EventLoop(EventLoop::Mode::New) {
}

EventLoop::EventLoop(EventLoop::Mode mode): loop_(nullptr), async_(nullptr), status_(NotStarted) {
	if (mode == EventLoop::Mode::New) {
		loop_ = new uv_loop_t();
		::uv_loop_init(loop_);
	} else {
		loop_ = uv_default_loop();
	}
	async_ = new Async(this);
}

EventLoop::~EventLoop() {
	if (loop_ != uv_default_loop()) {   
		uv_loop_close(loop_);
		delete async_;
		delete loop_;
	}
}

// singleton instance
EventLoop* EventLoop::default_loop() {
	static EventLoop defaultLoop(EventLoop::Mode::Default);
	return &defaultLoop;
}

uv_loop_t* EventLoop::handle() {
	return loop_;
}

int EventLoop::run() {
	if (status_ == Status::NotStarted) {
		async_->init();
		loopThreadId_ = std::this_thread::get_id();
		status_ = Status::Started;
		auto rst = ::uv_run(loop_, UV_RUN_DEFAULT);
		status_ = Status::Stopped;
		return rst;
	}
	return -1;
}

int EventLoop::run_no_wait() {
	if (status_ == Status::NotStarted) {
		async_->init();
		loopThreadId_ = std::this_thread::get_id();
		status_ = Status::Started;
		auto rst = ::uv_run(loop_, UV_RUN_NOWAIT);
		status_ = Status::NotStarted;
		return rst;
	}
	return -1;
}

int EventLoop::stop() {
	if (status_ == Status::Started) {
		async_->close([](Async* ptr) {
			::uv_stop(ptr->Loop()->handle());
		});
		return 0;
	}
	return -1;
}

bool EventLoop::is_stopped() {
	return status_ == Status::Stopped;
}

EventLoop::Status EventLoop::get_status() {
	return status_;
}

bool EventLoop::is_run_in_loop_thread() {
	if (status_ == Status::Started) {
		return std::this_thread::get_id() == loopThreadId_;
	}
	return false;
}

void EventLoop::run_in_this_loop(const DefaultCallback func) {
	if (nullptr == func)
		return;

	if (is_run_in_loop_thread() || is_stopped()) {
		func();
		return;
	}
	async_->runInThisLoop(func);
}

const char* EventLoop::get_error_message(int status) {
	if (WriteInfo::Disconnected == status) {
		static char info[] = "the connection is disconnected";
		return info;
	}
	return uv_strerror(status);
}