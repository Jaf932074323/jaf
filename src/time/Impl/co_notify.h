#pragma once
#include "CommonTimer.h"
#include "Interface/i_timer.h"
#include "util/co_coroutine.h"
#include "util/finally.h"
#include <assert.h>
#include <latch>
#include <memory>
#include <mutex>

namespace jaf
{
namespace time
{

class CoNotify
{
public:
    CoNotify(std::shared_ptr<ITimer> timer = nullptr)
        : timer_(timer == nullptr ? CommonTimer::Timer() : timer)
    {
        assert(timer_ != nullptr);
    };
    virtual ~CoNotify(){};

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
        Awaitable(CoNotify* co_timer)
            : co_timer_(co_timer)
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
                wait_flag_ = false;

                timeout_flag_ = false;
            }
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

            time_latch_      = std::unique_ptr<std::latch>(new std::latch(1));
            timeout_task_id_ = co_timer_->timer_->StartTask(jaf::time::STimerPara{[this](TimerResultType result_type) { TimerCallback(result_type); }, timeout_});
            return true;
        }

        bool await_resume() const
        {
            return !timeout_flag_;
        }

        void TimerCallback(TimerResultType result_type)
        {
            FINALLY(time_latch_->count_down(););

            if (result_type != TimerResultType::TRT_SUCCESS)
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
                co_timer_->timer_->StopTask(timeout_task_id_);
            }

            time_latch_->wait();
            handle_.resume();
        }

    private:
        CoNotify* co_timer_;
        std::coroutine_handle<> handle_;


        std::mutex wait_flag_mutex_;
        bool run_flag_  = false;
        bool wait_flag_ = false;

        bool timeout_flag_        = true;
        uint64_t timeout_         = 0;
        uint64_t timeout_task_id_ = 0;
        std::unique_ptr<std::latch> time_latch_;
    };

private:
    std::shared_ptr<ITimer> timer_;
    Awaitable awaitable_{this};

    bool wait_flag_ = false;
};

} // namespace time
} // namespace jaf