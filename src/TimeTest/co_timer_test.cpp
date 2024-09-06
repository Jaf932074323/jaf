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
// 2024-8-59 ½ª°²¸»
#include "global_timer/co_await_time.h"
#include "global_timer/co_sleep.h"
#include "global_timer/co_timer.h"
#include "global_timer/global_timer.h"
#include "impl/timer.h"
#include "util/co_coroutine_with_wait.h"
#include "util/stopwatch.h"
#include "gtest/gtest.h"
#include <memory>

TEST(co_timer_test, sleep)
{
    auto co_fun = []() -> jaf::CoroutineWithWait<void> {
        jaf::Stopwatch stopwatch;
        uint64_t sleep_time = 100;

        stopwatch.Reset();
        co_await jaf::time::CoSleep(sleep_time);
        uint64_t elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();
        EXPECT_TRUE(elapsed_time >= sleep_time - 20) << std::format("elapsed_time:{}, sleep_time {}", elapsed_time, sleep_time);
    };

    auto co_test_sleep = co_fun();
    co_test_sleep.Wait();
}

TEST(co_timer_test, co_timer)
{
    auto co_fun = [](std::shared_ptr<jaf::time::Timer> timer) -> jaf::CoroutineWithWait<void> {
        jaf::Stopwatch stopwatch;
        uint64_t sleep_time = 100;

        jaf::time::CoTimer co_timer(timer);
        stopwatch.Reset();
        co_await co_timer.Sleep(sleep_time);
        uint64_t elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();
        EXPECT_TRUE(elapsed_time >= sleep_time - 20) << std::format("elapsed_time:{}, sleep_time {}", elapsed_time, sleep_time);
    };

    std::shared_ptr<jaf::time::Timer> timer = std::make_shared<jaf::time::Timer>();
    timer->Start();

    auto co_test_co_timer = co_fun(timer);
    co_test_co_timer.Wait();

    timer->Stop();
}


TEST(co_timer_test, co_await_time_wait)
{
    auto co_fun = []() -> jaf::CoroutineWithWait<void> {
        jaf::Stopwatch stopwatch;

        jaf::time::CoAwaitTime await_time;
        await_time.Start();

        uint64_t sleep_time = 100;
        stopwatch.Reset();
        bool success          = co_await await_time.Wait(sleep_time);
        uint64_t elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();
        EXPECT_FALSE(success);
        EXPECT_TRUE(elapsed_time >= sleep_time - 20) << std::format("elapsed_time:{}, sleep_time {}", elapsed_time, sleep_time);

        await_time.Stop();
    };

    auto co_test_co_await_time = co_fun();
    co_test_co_await_time.Wait();
}

TEST(co_timer_test, co_await_time_notify)
{
    auto co_fun = []() -> jaf::CoroutineWithWait<void> {
        jaf::Stopwatch stopwatch;

        jaf::time::CoAwaitTime await_time;
        await_time.Start();

        uint64_t sleep_time = 100;

        jaf::time::STimerTask task;
        task.fun = [&](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {
            await_time.Notify();
        };
        task.interval = sleep_time;
        stopwatch.Reset();

        jaf::time::GlobalTimer::Timer()->StartTask(&task);
        bool success          = co_await await_time.Wait(1000);
        uint64_t elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();

        EXPECT_TRUE(success);
        EXPECT_TRUE(elapsed_time >= sleep_time - 20) << std::format("elapsed_time:{}, sleep_time {}", elapsed_time, sleep_time);

        await_time.Stop();
    };

    auto co_test_co_await_time = co_fun();
    co_test_co_await_time.Wait();
}

TEST(co_timer_test, co_await_time_stop)
{
    auto co_fun = []() -> jaf::CoroutineWithWait<void> {
        jaf::Stopwatch stopwatch;

        jaf::time::CoAwaitTime await_time;

        uint64_t sleep_time = 100;

        jaf::time::STimerTask task;
        task.fun = [&](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {
            await_time.Stop();
        };
        task.interval = sleep_time;
        stopwatch.Reset();

        await_time.Start();
        jaf::time::GlobalTimer::Timer()->StartTask(&task);
        bool success          = co_await await_time.Wait(500);
        uint64_t elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();

        EXPECT_FALSE(success);
        EXPECT_TRUE(elapsed_time >= sleep_time - 20) << std::format("elapsed_time:{}, sleep_time {}", elapsed_time, sleep_time);
    };

    auto co_test_co_await_time = co_fun();
    co_test_co_await_time.Wait();
}
