#pragma once
#include "CommonTimer.h"
#include "Interface/i_timer.h"
#include "util/co_coroutine.h"
#include <assert.h>

namespace jaf
{
namespace time
{

inline jaf::Coroutine<void> CoSleep(uint64_t millisecond)
{
    assert(CommonTimer::Timer() != nullptr);

    struct SleepAwaitable
    {
        uint64_t sleep_time_;
        std::coroutine_handle<> handle_;

        SleepAwaitable(uint64_t sleep_time)
            : sleep_time_(sleep_time)
        {
        }

        ~SleepAwaitable() {}

        bool await_ready()
        {
            if (sleep_time_ == 0)
            {
                return true;
            }

            return false;
        }

        bool await_suspend(std::coroutine_handle<> co_handle)
        {
            handle_ = co_handle;
            CommonTimer::Timer()->StartTask(jaf::time::STimerPara{[this](TimerResultType result_type) { TimerCallback(); }, sleep_time_});
            return true;
        }

        void await_resume()
        {
            return;
        }

        void TimerCallback()
        {
            handle_.resume();
        }
    };
    co_await SleepAwaitable(millisecond);
}

class CoTimer
{
public:
    CoTimer(std::shared_ptr<ITimer> timer = nullptr)
        : timer_(timer == nullptr ? CommonTimer::Timer() : timer)
    {
        assert(timer_ != nullptr);
    };
    virtual ~CoTimer(){};

public:
    jaf::Coroutine<void> Sleep(uint64_t millisecond)
    {
        struct SleepAwaitable
        {
            CoTimer* co_timer_;
            uint64_t sleep_time_;
            std::coroutine_handle<> handle_;

            SleepAwaitable(CoTimer* co_timer, uint64_t sleep_time)
                : co_timer_(co_timer)
                , sleep_time_(sleep_time)
            {
            }

            ~SleepAwaitable() {}

            bool await_ready()
            {
                if (sleep_time_ == 0)
                {
                    return true;
                }

                return false;
            }

            bool await_suspend(std::coroutine_handle<> co_handle)
            {
                handle_ = co_handle;
                co_timer_->timer_->StartTask(jaf::time::STimerPara{[this](TimerResultType result_type) { TimerCallback(); }, sleep_time_});
                return true;
            }

            void await_resume()
            {
                return;
            }

            void TimerCallback()
            {
                handle_.resume();
            }
        };
        co_await SleepAwaitable(this, millisecond);
    }

private:
    std::shared_ptr<ITimer> timer_;
};

} // namespace time
} // namespace jaf