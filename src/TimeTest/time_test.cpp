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
// 2024-8-19 ½ª°²¸»
#include "impl/timer.h"
#include "util/stopwatch.h"
#include <condition_variable>
#include <format>
#include <gtest/gtest.h>
#include <mutex>
#include <atomic>

TEST(time_test, timer)
{
    jaf::time::Timer timer;
    timer.Start();

    jaf::Stopwatch stopwatch;
    std::mutex wait_time_mutex;
    std::condition_variable wait_time_cv;
    uint64_t elapsed_time = 0;

    jaf::time::STimerTask task;
    task.fun = [&](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {
        EXPECT_EQ(result_type, jaf::time::ETimerResultType::TRT_SUCCESS);
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();
        std::unique_lock<std::mutex> lock(wait_time_mutex);
        wait_time_cv.notify_all();
    };
    task.interval = 50;

    stopwatch.Reset();
    timer.StartTask(&task);

    {
        std::unique_lock<std::mutex> lock(wait_time_mutex);
        auto wait_result = wait_time_cv.wait_for(lock, std::chrono::milliseconds(task.interval * 2));
    }

    EXPECT_TRUE(elapsed_time >= task.interval - timer.GetLeadTime());
    EXPECT_TRUE(elapsed_time < task.interval + 50);

    timer.Stop();
}

TEST(time_test, stop_timer_task)
{
    jaf::time::Timer timer;
    timer.Start();

    jaf::Stopwatch stopwatch;
    std::mutex wait_time_mutex;
    std::condition_variable wait_time_cv;
    uint64_t elapsed_time = 0;

    jaf::time::STimerTask task;
    task.fun = [&](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {
        EXPECT_EQ(result_type, jaf::time::ETimerResultType::TRT_TASK_STOP);
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();
        std::unique_lock<std::mutex> lock(wait_time_mutex);
        wait_time_cv.notify_all();
    };
    task.interval = 1000;

    stopwatch.Reset();
    timer.StartTask(&task);
    timer.StopTask(&task);

    {
        std::unique_lock<std::mutex> lock(wait_time_mutex);
        auto wait_result = wait_time_cv.wait_for(lock, std::chrono::milliseconds(task.interval * 2));
    }

    EXPECT_TRUE(elapsed_time < task.interval + 50);

    timer.Stop();
}

TEST(time_test, stop_timer)
{
    jaf::time::Timer timer;
    timer.Start();

    jaf::Stopwatch stopwatch;
    uint64_t elapsed_time = 0;

    jaf::time::STimerTask task;
    task.fun = [&](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {
        EXPECT_EQ(result_type, jaf::time::ETimerResultType::TRT_TIMER_STOP);
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stopwatch.Time()).count();
    };
    task.interval = 1000;

    stopwatch.Reset();
    timer.StartTask(&task);
    timer.Stop();

    EXPECT_TRUE(elapsed_time < task.interval + 50);
}

TEST(time_test, mixture)
{
    jaf::time::Timer timer;
    timer.Start();

    std::atomic<size_t> task_counter = 0;

    auto fun = [&](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task, jaf::time::ETimerResultType expect_result_type) {
        EXPECT_EQ(result_type, expect_result_type);
        ++task_counter;
    };


    constexpr size_t task_mount = 10;
    jaf::time::STimerTask tasks[task_mount];
    for (size_t i = 0; i < task_mount; ++i)
    {
        jaf::time::STimerTask& task = tasks[i];
        uint64_t interval           = 0;
        jaf::time::ETimerResultType expect_result_type;
        switch (i % 3)
        {
        case 0:
            expect_result_type = jaf::time::ETimerResultType::TRT_SUCCESS;
            interval           = 50;
            break;
        case 1:
            expect_result_type = jaf::time::ETimerResultType::TRT_TASK_STOP;
            interval           = 10000;
            break;
        case 2:
            expect_result_type = jaf::time::ETimerResultType::TRT_TIMER_STOP;
            interval           = 10000;
            break;
        default:
            expect_result_type = jaf::time::ETimerResultType::TRT_TIMER_STOP;
            interval           = 10000;
            break;
        }
        task.fun      = std::bind(fun, std::placeholders::_1, std::placeholders::_2, expect_result_type);
        task.interval = interval;

        timer.StartTask(&task);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    for (size_t i = 0; i < task_mount; ++i)
    {
        jaf::time::STimerTask& task = tasks[i];
        switch (i % 3)
        {
        case 0:
            break;
        case 1:
            timer.StopTask(&task);
            break;
        case 2:
            break;
        default:
            break;
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    timer.Stop();
    EXPECT_EQ(task_counter, task_mount);
}
