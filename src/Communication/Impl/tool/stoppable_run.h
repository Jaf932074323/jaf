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
#include "Time/Interface/i_timer.h"
#include "util/co_coroutine.h"
#include "util/co_wait_util_controlled_stop.h"
#include <type_traits>

namespace jaf
{
namespace comm
{

// 附带停止功能的运行运行
// AwaitObj 是一个协程相关的可等待对象，并且要拥有停止函数Stop()
template <typename AwaitObj>
struct StoppableRun
{
    using AwaitObjResult = std::invoke_result_t<decltype(&AwaitObj::await_resume), AwaitObj*>;

public:
    StoppableRun(jaf::ControlStartStop& control_start_stop, AwaitObj& await_obj)
        : control_start_stop_(control_start_stop)
        , await_obj_(await_obj)
    {
    }

public:
    // 获取等待结果
    decltype(auto) Result()
    {
        return std::move(await_obj_result_);
    }

public:
    Coroutine<void> Run()
    {
        jaf::CoWaitUtilControlledStop wait_stop(control_start_stop_);

        Coroutine<void> run_wait_obj = RunAwaitObj(wait_stop);
        Coroutine<void> wait_timeout = WaitTimeout(wait_stop);

        co_await run_wait_obj;
        co_await wait_timeout;
    }

private:
    Coroutine<void> RunAwaitObj(jaf::CoWaitUtilControlledStop& wait_stop)
    {
        await_obj_result_ = co_await await_obj_;
        wait_stop.Notify();
        co_return;
    };

    Coroutine<void> WaitTimeout(jaf::CoWaitUtilControlledStop& wait_stop)
    {
        co_await wait_stop;
        await_obj_.Stop();
        co_return;
    };

private:
    jaf::ControlStartStop& control_start_stop_;
    AwaitObj& await_obj_; // 协程相关的可等待的对象

    AwaitObjResult await_obj_result_; // 等待结果
};

} // namespace comm
} // namespace jaf