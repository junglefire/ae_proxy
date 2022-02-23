#ifndef SMART_ASYNC_TASK_HPP
#define SMART_ASYNC_TASK_HPP

#include <functional>
#include <memory>
#include <queue>
#include <mutex>

#include "event_loop.h"

NAMESPACE_BEGIN(smart)

class Async  : public std::enable_shared_from_this<Async>
{
public:
    using OnCloseCompletedCallback = std::function<void(Async*)>;
    Async(EventLoop* loop);
    void  init();
    virtual ~Async();

    void runInThisLoop(DefaultCallback callback);

    void close(OnCloseCompletedCallback callback);
    EventLoop* Loop();
private:
    EventLoop* loop_;
    std::mutex mutex_;
    uv_async_t* handle_;
    std::queue<DefaultCallback> callbacks_;
    OnCloseCompletedCallback onCloseCompletCallback_;
    void process();
    static void Callback(uv_async_t* handle);
    void onCloseCompleted();

};


using AsyncPtr = std::shared_ptr<Async>;

NAMESPACE_END(smart)
#endif
