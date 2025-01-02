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
// 2024-6-16 ������
#include "global_timer/co_await_time.h"
#include "Time/Interface/i_timer.h"
#include "util/co_coroutine.h"
#include "util/control_start_stop.h"
#include <type_traits>

namespace jaf
{
namespace comm
{

// ����ֱ����ʱ
// AwaitObj ��һ��Э����صĿɵȴ����󣬲���Ҫӵ��ֹͣ����Stop()
template <typename AwaitObj>
struct RunWithTimeout
{
    using AwaitObjResult = std::invoke_result_t<decltype(&AwaitObj::await_resume), AwaitObj*>;

public:
    RunWithTimeout(jaf::ControlStartStop& control_start_stop, AwaitObj& await_obj, uint64_t timeout, std::shared_ptr<jaf::time::ITimer> timer)
        : control_start_stop_(control_start_stop)
        , await_obj_(await_obj)
        , timeout_(timeout)
        , timer_(timer)
    {
        assert(timer_ != nullptr);
    }

public:
    // �Ƿ�ʱtrue��ʱ,falseδ��ʱ
    bool IsTimeout()
    {
        return timeout_flag_;
    }
    // ��ȡ�ȴ����
    decltype(auto) Result()
    {
        return std::move(await_obj_result_);
    }

public:
    Coroutine<void> Run()
    {
        jaf::time::CoAwaitTime await_time(timeout_, control_start_stop_, timer_);

        Coroutine<void> run_wait_obj = RunAwaitObj(await_time);
        Coroutine<void> wait_timeout = WaitTimeout(await_time);

        co_await run_wait_obj;
        co_await wait_timeout;
    }

private:
    Coroutine<void> RunAwaitObj(jaf::time::CoAwaitTime& await_time)
    {
        await_obj_result_ = co_await await_obj_;
        await_time.Stop();
        co_return;
    };

    Coroutine<void> WaitTimeout(jaf::time::CoAwaitTime& await_time)
    {
        timeout_flag_ = co_await await_time;
        await_obj_.Stop();
        co_return;
    };

private:
    jaf::ControlStartStop& control_start_stop_;
    AwaitObj& await_obj_; // Э����صĿɵȴ��Ķ���
    uint64_t timeout_;    // ��ʱʱ�� ����
    std::shared_ptr<jaf::time::ITimer> timer_;

    bool timeout_flag_ = false;       // �Ƿ�ʱtrue��ʱ,falseδ��ʱ
    AwaitObjResult await_obj_result_; // �ȴ����
};

} // namespace comm
} // namespace jaf