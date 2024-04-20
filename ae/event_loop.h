#ifndef SMART_EVENT_LOOP_HPP
#define SMART_EVENT_LOOP_HPP

#include "ae.h"

NAMESPACE_BEGIN(smart)

class EventLoop {
public:
	enum Mode {
		Default, 	// 使用uv_default_loop()创建uv_loop_t
		New 		// 创建新的uv_loop_t
	};
public:
	enum Status {
		NotStarted,
		Started,
		Stopped
	};
public:
	EventLoop();
	~EventLoop();
	EventLoop(const EventLoop&) = delete;
	EventLoop& operator=(const EventLoop&) = delete;
public:
	static EventLoop* default_loop();
	static const char* get_error_message(int status);
public:
	int run();
	int run_no_wait();
	int stop();
	bool is_stopped();
	Status get_status();
	bool is_run_in_loop_thread();
	void run_in_this_loop(const DefaultCallback func);
	uv_loop_t* handle();
private:
	EventLoop(Mode mode);
	std::thread::id loopThreadId_;
	uv_loop_t* loop_;
	Async* async_;
	std::atomic<Status> status_;
};

NAMESPACE_END(smart)

#endif

