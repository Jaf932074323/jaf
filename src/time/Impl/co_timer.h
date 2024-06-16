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
// 2024-6-16 ½ª°²¸»
#include "common_timer.h"
#include "interface/i_timer.h"
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
            CommonTimer::Timer()->StartTask(jaf::time::STimerPara{[this](ETimerResultType result_type) { TimerCallback(); }, sleep_time_});
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
                co_timer_->timer_->StartTask(jaf::time::STimerPara{[this](ETimerResultType result_type) { TimerCallback(); }, sleep_time_});
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