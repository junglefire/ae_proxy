#ifndef SMART_TIMER_H__
#define SMART_TIMER_H__

#include "event_loop.h"
#include "ae.h"

NAMESPACE_BEGIN(smart)

// libuv timer wrapper class
class Timer {
public:
	Timer(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback callback);
	virtual ~Timer();
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;
public:
	void start();
	void close(TimerCloseComplete callback);
	void set_timer_repeat(uint64_t ms);
private:
	bool started_;
	uv_timer_t* handle_;
	uint64_t timeout_;
	uint64_t repeat_;
	TimerCallback callback_;
	TimerCloseComplete closeComplete_;
private:
	void on_time_out();
	void close_complete();
private:
	static void process(uv_timer_t* handle);
};

NAMESPACE_END(smart)
#endif
