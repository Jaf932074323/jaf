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
// 2024-6-20 姜安富
#include "test_single_thread_exec.h"
#include "log_head.h"
#include "util/co_coroutine.h"
#include "util/single_thread_exec.h"
#include <format>
#include <iostream>
#include <thread>

class TestSingleThreadExec
{
public:
    void Test()
    {
        // 准备
        std::thread simple_thread_exec_thread(
            [this]() {
                simple_thread_exec_.Run();
            });
        latch.Reset();

        // 启动多个线程
        number_ = 0;
        for (int64_t i = 0; i < thread_number_; ++i)
        {
            std::thread run_thread([this]() { ThreadRun(); });
            run_thread.detach();
        }

        // 等待多线程执行结束
        latch.Wait();

        // 收尾
        simple_thread_exec_.Stop();
        simple_thread_exec_thread.join();

        LOG_INFO() << std::format("预期: {}, 结果: {}, 符合预期 = {}", thread_number_ * thread_add_time, number_, number_ == thread_number_ * thread_add_time);
    }

private:
    jaf::Coroutine<void> ThreadRun()
    {
        co_await Add();
        latch.CountDown();
    }

    jaf::Coroutine<void> Add()
    {
        co_await simple_thread_exec_.Switch(); // 切换到单线程执行

        for (int64_t i = 0; i < thread_add_time; ++i)
        {
            number_ = number_ + 1; // 已经切换到单个线程了，不需要加锁
        }

        co_return;
    }

private:
    const int64_t thread_number_  = 100;
    const int64_t thread_add_time = 1000000;

    int64_t number_ = 0;
    jaf::Latch latch{thread_number_};
    jaf::SimpleThreadExec simple_thread_exec_;
};

void test_single_thread_exec()
{
    TestSingleThreadExec test;
    test.Test();
    return;
}
