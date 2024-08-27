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
#include "impl/co_timer.h"
#include "impl/timer.h"
#include "util/co_coroutine_with_wait.h"
#include "util/stopwatch.h"
#include "gtest/gtest.h"

jaf::CoroutineWithWait<void> TestSleep(jaf::time::Timer& timer)
{
    jaf::time::CoTimer co_timer(&timer);
    jaf::Stopwatch stopwatch;

    uint64_t sleep_time = 200;

    stopwatch.Reset();
    co_await co_timer.Sleep(sleep_time);
    uint64_t elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();

    EXPECT_TRUE(elapsed_time >= sleep_time - 20) << std::format("elapsed_time:{}, sleep_time {}", elapsed_time, sleep_time);

}

TEST(co_timer_test, co_timer)
{
    jaf::time::Timer timer;
    timer.Start();

    auto co_test_sleep = TestSleep(timer);
    co_test_sleep.Wait();

    timer.Stop();

    std::this_thread::sleep_for(std::chrono::seconds(5));
}
