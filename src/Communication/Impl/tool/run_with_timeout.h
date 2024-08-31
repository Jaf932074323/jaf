#pragma once
// MIT License
//
// Copyright(c) 2021 Jaf932074323
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 2024-6-16 姜安富
#include "global_timer/co_await_time.h"
#include "util/co_coroutine.h"
#include <type_traits>

namespace jaf
{
namespace comm
{

// 运行直到超时
// AwaitObj 是一个协程相关的可等待对象，并且要拥有停止函数Stop()
template <typename AwaitObj>
struct RunWithTimeout
{
    using AwaitObjResult = std::invoke_result_t<decltype(&AwaitObj::await_resume), AwaitObj*>;

public:
    RunWithTimeout(jaf::time::CoAwaitTime& await_time, AwaitObj& await_obj, uint64_t timeout)
        : await_time_(await_time)
        , await_obj_(await_obj)
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
    const AwaitObjResult& Result()
    {
        return await_obj_result_;
    }

public:
    Coroutine<void> Run()
    {
        Coroutine<void> run_wait_obj = RunAwaitObj();
        Coroutine<void> wait_timeout = WaitTimeout();

        co_await run_wait_obj;
        co_await wait_timeout;
    }

private:
    Coroutine<void> RunAwaitObj()
    {
        await_obj_result_ = co_await await_obj_;
        await_time_.Notify();
        co_return;
    };

    Coroutine<void> WaitTimeout()
    {
        timeout_flag_ = !co_await await_time_.Wait(timeout_);
        await_obj_.Stop();
        co_return;
    };

private:
    jaf::time::CoAwaitTime& await_time_;
    AwaitObj& await_obj_; // 协程相关的可等待的对象
    uint64_t timeout_;    // 超时时间 毫秒

    bool timeout_flag_ = false;       // 是否超时true超时,false未超时
    AwaitObjResult await_obj_result_; // 等待结果
};

} // namespace comm
} // namespace jaf