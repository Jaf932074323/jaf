#pragma once
#include "Impl/co_notify.h"
#include "util/co_coroutine.h"
#include <type_traits>

namespace jaf
{
namespace comm
{

// 等待直到超时
// WaitObj 是一个协程相关的可等待对象，并且要拥有附带停止函数Stop()
template <typename WaitObj>
struct RunUntilTimeout
{
    //using Result = decltype(std::declval<WaitObj>().await_resume());
    using WaitObjResult = std::invoke_result_t<decltype(&WaitObj::await_resume), WaitObj*>;

public:
    RunUntilTimeout(jaf::time::CoNotify& notify, WaitObj& wait_obj, uint64_t timeout)
        : notify_(notify)
        , wait_obj_(wait_obj)
        , timeout_(timeout)
    {
    }

public:
    // 是否超时true超时,false未超时
    bool IsTimeout()
    {
        return timeout_flag_;
    }
    // 获取等待结果
    const WaitObjResult& Result()
    {
        return wait_obj_result_;
    }

public:
    Coroutine<void> Run()
    {
        Coroutine<void> run_wait_obj = RunWaitObj();
        Coroutine<void> wait_timeout = WaitTimeout();

        co_await run_wait_obj;
        co_await wait_timeout;
    }

private:
    Coroutine<void> RunWaitObj()
    {
        wait_obj_result_ = co_await wait_obj_;
        notify_.Notify();
        co_return;
    };

    Coroutine<void> WaitTimeout()
    {
        timeout_flag_ = !co_await notify_.Wait(timeout_);
        wait_obj_.Stop();
        co_return;
    };


private:
    jaf::time::CoNotify& notify_;
    WaitObj& wait_obj_; // 协程相关的可等待的对象
    uint64_t timeout_;  // 超时时间 毫秒

    bool timeout_flag_ = false;          // 是否超时true超时,false未超时
    WaitObjResult wait_obj_result_; // 等待结果
};

} // namespace comm
} // namespace jaf