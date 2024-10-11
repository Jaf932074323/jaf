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
// 2024-10-6 ½ª°²¸»
#include "util/co_coroutine.h"
#include "util/co_coroutine_with_wait.h"
#include "util/control_start_stop.h"
#include "util/finally.h"
#include "gtest/gtest.h"
#include <atomic>
#include <mutex>
#include <list>
#include <thread>

TEST(test_control_start_stop, usual)
{
    const size_t total_number  = 100;
    std::atomic<size_t> number = 0;
    std::atomic<size_t> wait_number = 0;

    auto co_wait_stop_fun = [&number](jaf::ControlStartStop& control_start_stop) -> jaf::Coroutine<void> {
        jaf::CoWaitStop wait_stop(control_start_stop);
        co_await wait_stop;
        ++number;
    };

    jaf::ControlStartStop control_start_stop;
    control_start_stop.Start();

    std::mutex coroutines_mutex;
    std::list<jaf::Coroutine<void>> coroutines;
    std::list<std::thread> threads;
    for (size_t i = 0; i < total_number; ++i)
    {
        std::thread thread([&]() {
            jaf::Coroutine<void> coroutine = co_wait_stop_fun(control_start_stop);

            std::unique_lock<std::mutex> lock(coroutines_mutex);
            coroutines.push_back(std::move(coroutine));
        });
        threads.push_back(std::move(thread));
    }

    control_start_stop.Stop();

    std::thread wait_thread_1([&]() -> jaf::Coroutine<void> {
        std::unique_lock<std::mutex> lock(coroutines_mutex);
        for (jaf::Coroutine<void>& coroutine : coroutines)
        {
            co_await coroutine;
            ++wait_number;
        }
        coroutines.clear();
        });

    for (std::thread& thread : threads)
    {
        thread.join();
    }

    std::thread wait_thread_2([&]() -> jaf::Coroutine<void> {
        std::unique_lock<std::mutex> lock(coroutines_mutex);
        for (jaf::Coroutine<void>& coroutine : coroutines)
        {
            co_await coroutine;
            ++wait_number;
        }
        coroutines.clear();
        });

    wait_thread_1.join();
    wait_thread_2.join();

    EXPECT_EQ(number, total_number);
    EXPECT_EQ(wait_number, total_number);
}
