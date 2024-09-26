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
#include "util/finally.h"
#include <assert.h>
#include <memory>
#include <mutex>
#include <set>

namespace jaf
{
namespace time
{

class CoAwaitTime
{
public:
    CoAwaitTime(std::shared_ptr<ITimer> timer = nullptr)
        : timer_(timer == nullptr ? GlobalTimer::Timer() : timer)
    {
        assert(timer_ != nullptr);
    };
    virtual ~CoAwaitTime()
    {};

private:
    struct Awaitable;
public:
    struct WaitHandle
    {
    private:
        friend CoAwaitTime;
        Awaitable* awaitable_ = nullptr;
    };

public:
    void Start()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        run_flag_ = true;
    }

    void Stop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        run_flag_ = false;

        for (Awaitable* awaitable : awaitables_)
        {
            awaitable->Stop();
        }
    }

    jaf::Coroutine<bool> Wait(WaitHandle& handle, uint64_t millisecond)
    {
        Awaitable awaitable(timer_, millisecond);

        assert(handle.awaitable_ == nullptr);
        handle.awaitable_ = &awaitable;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!run_flag_)
            {
                co_return false;
            }

            awaitables_.insert(&awaitable);
        }

        bool result = co_await awaitable;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            awaitables_.erase(&awaitable);
        }

        co_return result;
    }

    void Notify(WaitHandle& handle)
    {
        assert(handle.awaitable_ != nullptr);
        handle.awaitable_->Notify();
    }

private:
    struct Awaitable
    {
        Awaitable(std::shared_ptr<ITimer> timer, uint64_t millisecond)
            : timer_(timer)
            , timeout_task_{.fun = [this](ETimerResultType result_type, STimerTask* task) { TimerCallback(result_type); }, .interval = millisecond}
        {
        }

        ~Awaitable()
        {
            assert(!wait_flag_); // 当wait_flag_为ture，说明后续还会有定时器回调过来，会导致崩溃
        }

        void Stop()
        {
            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            run_flag_ = false;
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

        void Notify()
        {
            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            if (!wait_flag_)
            {
                return;
            }
            timer_->StopTask(&timeout_task_);
        }

    private:
        std::shared_ptr<ITimer> timer_;
        std::coroutine_handle<> handle_;

        std::mutex wait_flag_mutex_;
        bool run_flag_  = true;
        bool wait_flag_ = false;

        bool wait_result_flag_ = false;
        STimerTask timeout_task_;
    };

private:
    std::shared_ptr<ITimer> timer_;

    std::mutex mutex_;
    std::set<Awaitable*> awaitables_;
    bool run_flag_ = false;
};

} // namespace time
} // namespace jaf