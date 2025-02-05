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
#include "util/co_coroutine.h"
#include "co_timer.h"
#include <assert.h>

namespace jaf
{
namespace time
{

inline jaf::Coroutine<void> CoSleep(uint64_t millisecond)
{
    assert(GlobalTimer::Timer() != nullptr);

    struct SleepAwaitable
    {
        STimerTask timer_task_;
        std::coroutine_handle<> handle_;

        SleepAwaitable(uint64_t sleep_time)
        {
            timer_task_.interval = sleep_time;
            timer_task_.fun      = [this](ETimerResultType result_type, STimerTask* task) { TimerCallback(); };
        }

        ~SleepAwaitable() {}

        bool await_ready()
        {
            if (timer_task_.interval == 0)
            {
                return true;
            }

            return false;
        }

        bool await_suspend(std::coroutine_handle<> co_handle)
        {
            handle_ = co_handle;
            GlobalTimer::Timer()->StartTask(&timer_task_);
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

} // namespace time
} // namespace jaf