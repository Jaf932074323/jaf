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
#include "global_timer.h"
#include "time/interface/i_timer.h"
#include "util/co_coroutine.h"
#include "util/control_start_stop.h"
#include "util/finally.h"
#include <assert.h>
#include <memory>
#include <mutex>

namespace jaf
{
namespace time
{

struct CoAwaitTime
{
    CoAwaitTime(uint64_t millisecond, ControlStartStop& control_start_stop, std::shared_ptr<ITimer> timer = nullptr)
        : timer_(timer == nullptr ? GlobalTimer::Timer() : timer)
        , control_start_stop_(control_start_stop)
        , timeout_task_{.fun = [this](ETimerResultType result_type, STimerTask* task) { TimerCallback(result_type); }, .interval = millisecond}
    {
        agent_    = control_start_stop_.Register([this]() { InterStop(); });
        run_flag_ = agent_ != nullptr;
    }

    ~CoAwaitTime()
    {
    }

    void Stop()
    {
        if (agent_ == nullptr)
        {
            return;
        }
        agent_->Stop();
    }
private:
    void InterStop()
    {
        {
            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            assert(run_flag_ ? true : !wait_flag_); // 如果没有运行，则一定也不会等待
            run_flag_ = false;
            if (!wait_flag_)
            {
                return;
            }
            timer_->StopTask(&timeout_task_);
        }
        agent_ = nullptr;
    }

public:
    void Notify()
    {
        std::unique_lock<std::mutex> lock(wait_flag_mutex_);
        if (!wait_flag_)
        {
            return;
        }
        timer_->StopTask(&timeout_task_);
    }

    bool await_ready() const
    {
        return false;
    }

    bool await_suspend(std::coroutine_handle<> co_handle)
    {
        std::unique_lock<std::mutex> lock(wait_flag_mutex_);
        if (!run_flag_)
        {
            wait_result_flag_ = false;
            return false;
        }
        assert(!wait_flag_); // 不能同时等待多个
        wait_flag_ = true;

        handle_ = co_handle;

        timer_->StartTask(&timeout_task_);
        return true;
    }

    bool await_resume() const
    {
        return wait_result_flag_;
    }

    void TimerCallback(ETimerResultType result_type)
    {
        {
            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            assert(wait_flag_);
            wait_flag_        = false;
            wait_result_flag_ = run_flag_ && result_type == ETimerResultType::TRT_TASK_STOP;
        }

        handle_.resume();
    }

private:
    std::shared_ptr<ITimer> timer_;
    std::coroutine_handle<> handle_;

    ControlStartStop& control_start_stop_;

    std::mutex wait_flag_mutex_;
    bool run_flag_  = false;
    bool wait_flag_ = false;

    STimerTask timeout_task_;

    bool wait_result_flag_ = false; // 等待结果，等待到通知时为ture，超时为false

    std::shared_ptr<ControlStartStop::Agent> agent_;
};

} // namespace time
} // namespace jaf