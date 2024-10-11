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
// 2024-8-59 姜安富
#include "global_timer/co_await_time.h"
#include "global_timer/co_timer.h"
#include "global_timer/global_timer.h"
#include "impl/timer.h"
#include "impl/tool/run_with_timeout.h"
#include "util/co_coroutine_with_wait.h"
#include "util/control_start_stop.h"
#include "util/single_thread_exec.h"
#include "util/stopwatch.h"
#include "gtest/gtest.h"
#include <memory>

TEST(run_with_timeout, sleep)
{
    class Awaitable
    {
    public:
        Awaitable(jaf::SimpleThreadExec& simple_thread_exec)
            : simple_thread_exec_(simple_thread_exec)
        {
        }
        ~Awaitable()
        {
        }
        bool await_ready()
        {
            return false;
        }
        bool await_suspend(std::coroutine_handle<> co_handle)
        {
            handle_ = co_handle;

            simple_thread_exec_.Post(std::bind(&Awaitable::Stop, this));

            return true;
        }
        bool await_resume() const
        {
            return true;
        }
        void Stop()
        {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (callback_flag_)
                {
                    return;
                }
                callback_flag_ = true;
            }

            handle_.resume();
        }

    private:
        std::coroutine_handle<> handle_;

        jaf::SimpleThreadExec& simple_thread_exec_;

        std::mutex mutex_;
        bool callback_flag_ = false; // 已经回调标记
    };


    jaf::SimpleThreadExec simple_thread_exec;
    simple_thread_exec.Start();

    auto co_fun = [&]() -> jaf::CoroutineWithWait<void> {

        jaf::Stopwatch stopwatch;
        jaf::ControlStartStop control_start_stop;
        control_start_stop.Start();

        Awaitable awaitable(simple_thread_exec);
        jaf::comm::RunWithTimeout<Awaitable> run_with_timeout(control_start_stop, awaitable, 1000, jaf::time::GlobalTimer::Timer());

        stopwatch.Reset();
        co_await run_with_timeout.Run();
        uint64_t elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();

        EXPECT_TRUE(elapsed_time <= 25) << std::format("elapsed_time:{}", elapsed_time);

        control_start_stop.Stop();

        co_return;
    };


    auto co_test_sleep = co_fun();
    co_test_sleep.Wait();

    simple_thread_exec.Stop();
}
