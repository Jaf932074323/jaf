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
#include "Interface/i_timer.h"
#include "common_timer.h"
#include "util/co_coroutine.h"
#include "util/finally.h"
#include "util/latch.h"
#include <assert.h>
#include <memory>
#include <mutex>

namespace jaf
{
namespace time
{

class CoAwaitTime
{
public:
    CoAwaitTime(std::shared_ptr<ITimer> timer = nullptr)
        : timer_(timer == nullptr ? CommonTimer::Timer() : timer)
    {
        assert(timer_ != nullptr);
    };
    virtual ~CoAwaitTime(){};

public:
    void Start()
    {
        awaitable_.Start();
    }

    void Stop()
    {
        awaitable_.Stop();
    }

    jaf::Coroutine<bool> Wait(uint64_t millisecond)
    {
        if (wait_flag_)
        {
            awaitable_.Notify();
        }
        assert(!wait_flag_); // 同时只能等待一个
        wait_flag_ = true;

        awaitable_.SetTimeout(millisecond);
        bool result = co_await awaitable_;

        wait_flag_ = false;

        co_return result;
    }

    void Notify()
    {
        awaitable_.Notify();
    }

private:
    struct Awaitable
    {
        Awaitable(CoAwaitTime* co_await_time)
            : co_await_time_(co_await_time)
        {
        }

        ~Awaitable() {}

        void SetTimeout(uint64_t timeout)
        {
            timeout_ = timeout;
        }

        void Start()
        {
            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            run_flag_ = true;
        }

        void Stop()
        {
            {
                std::unique_lock<std::mutex> lock(wait_flag_mutex_);
                run_flag_ = false;
                if (!wait_flag_)
                {
                    return;
                }
                wait_flag_    = false;
                timeout_flag_ = false;
                timeout_task_->Stop();
            }
            time_latch_.Wait();
            handle_.resume();
        }

        bool await_ready() const
        {
            if (timeout_ == 0)
            {
                return true;
            }

            return false;
        }

        bool await_suspend(std::coroutine_handle<> co_handle)
        {
            handle_ = co_handle;

            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            if (!run_flag_)
            {
                timeout_flag_ = false;
                return false;
            }
            wait_flag_ = true;

            time_latch_.Reset();
            timeout_task_ = co_await_time_->timer_->StartTask(jaf::time::STimerPara{[this](ETimerResultType result_type) { TimerCallback(result_type); }, timeout_});
            return true;
        }

        bool await_resume() const
        {
            return !timeout_flag_;
        }

        void TimerCallback(ETimerResultType result_type)
        {
            {
                FINALLY(time_latch_.CountDown(););

                if (result_type != ETimerResultType::TRT_SUCCESS)
                {
                    return;
                }

                {
                    std::unique_lock<std::mutex> lock(wait_flag_mutex_);
                    if (!wait_flag_)
                    {
                        return;
                    }
                    wait_flag_    = false;
                    timeout_flag_ = true;
                }
            }

            handle_.resume();
        }

        void Notify()
        {
            {
                std::unique_lock<std::mutex> lock(wait_flag_mutex_);
                if (!wait_flag_)
                {
                    return;
                }
                wait_flag_    = false;
                timeout_flag_ = false;
                timeout_task_->Stop();
            }

            time_latch_.Wait();
            handle_.resume();
        }

    private:
        CoAwaitTime* co_await_time_;
        std::coroutine_handle<> handle_;


        std::mutex wait_flag_mutex_;
        bool run_flag_  = false;
        bool wait_flag_ = false;

        bool timeout_flag_        = true;
        uint64_t timeout_         = 0;
        std::shared_ptr<ITimerTask> timeout_task_ = 0;
        Latch time_latch_{1};
    };

private:
    std::shared_ptr<ITimer> timer_;
    Awaitable awaitable_{this};

    bool wait_flag_ = false;
};

} // namespace time
} // namespace jaf