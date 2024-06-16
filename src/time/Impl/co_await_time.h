#pragma once
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