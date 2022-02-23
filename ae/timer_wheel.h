#ifndef SMART_TIMER_WHEEL_HPP
#define SMART_TIMER_WHEEL_HPP

#include <vector>
#include <set>
#include <memory>
#include "timer.h"
#include "tcp_connection.h"

NAMESPACE_BEGIN(smart)

// algorithm complexity o(1).
template<typename T>
class TimerWheel {
public:
	TimerWheel(EventLoop* loop);
	TimerWheel(EventLoop* loop, unsigned int timeout);
	TimerWheel(const TimerWheel&) = delete;
	TimerWheel& operator=(const TimerWheel&) = delete;
public:
	void set_timeout(unsigned int seconds);
	int get_timeout();
	void start();
	void insert(std::shared_ptr<T> value);
private:
	unsigned int index_;
	unsigned int timeoutSec_;
	Timer timer_;
	std::vector<std::set<std::shared_ptr<T>>> wheel_;
	void wheel_callback();
};

template<typename T>
inline TimerWheel<T>::TimerWheel(EventLoop* loop): TimerWheel(loop, 0) {
}

template<typename T>
inline TimerWheel<T>::TimerWheel(EventLoop* loop, unsigned int timeout): 
	index_(0),
	timeoutSec_(timeout),
	timer_(loop, 1000, 1000, std::bind(&TimerWheel::wheel_callback, this)) {
	// function
}

template<typename T>
inline void TimerWheel<T>::set_timeout(unsigned int seconds) {
	timeoutSec_ = seconds;
}

template<typename T>
inline void TimerWheel<T>::start() {
	if (timeoutSec_) {
		wheel_.resize(timeoutSec_);
		timer_.start();
	}
}

template<typename T>
inline void TimerWheel<T>::insert(std::shared_ptr<T> value) {
	if (timeoutSec_ > 0 && value != nullptr) {
		wheel_[index_].insert(value);
	}
}

template<typename T>
inline int TimerWheel<T>::get_timeout() {
	return timeoutSec_;
}

template<typename T>
inline void TimerWheel<T>::wheel_callback() {
	if (!timeoutSec_)
		return;
	if (++index_ == timeoutSec_) {
		index_ = 0;
	}
	wheel_[index_].clear();
}

NAMESPACE_END(smart)
#endif
