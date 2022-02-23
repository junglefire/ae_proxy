#include "timer.h"

using namespace smart;

Timer::Timer(EventLoop * loop, uint64_t timeout, uint64_t repeat, TimerCallback callback):
	started_(false),
	handle_(new uv_timer_t),
	timeout_(timeout),
	repeat_(repeat),
	callback_(callback),
	closeComplete_(nullptr) {
	// constructor
	handle_->data = static_cast<void*>(this);
	::uv_timer_init(loop->handle(), handle_);
}

Timer::~Timer() {
}

void Timer::start() {
	if (!started_) {
		started_ = true;
		::uv_timer_start(handle_, Timer::process, timeout_, repeat_);
	}
}

void Timer::close(TimerCloseComplete callback) {
	closeComplete_ = callback;
	if (::uv_is_active((uv_handle_t*)handle_)) {
		::uv_timer_stop(handle_);
	}
	
	if (uv_is_closing((uv_handle_t*)handle_) == 0) {
		::uv_close((uv_handle_t*)handle_, [](uv_handle_t* handle) {
			auto ptr = static_cast<Timer*>(handle->data);
			ptr->close_complete();
			delete handle;
		});
	} else {
		close_complete();
	}
}

void Timer::set_timer_repeat(uint64_t ms) {
	repeat_ = ms;
	::uv_timer_set_repeat(handle_, ms);
}

void Timer::on_time_out() {
	if (callback_) {
		callback_(this);
	}
}

void Timer::close_complete() {
	if (closeComplete_) {
		closeComplete_(this);
	}
}

void Timer::process(uv_timer_t * handle) {
	auto ptr = static_cast<Timer*>(handle->data);
	ptr->on_time_out();
}


