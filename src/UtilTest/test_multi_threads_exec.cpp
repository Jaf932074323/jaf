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
#include "test_multi_threads_exec.h"
#include "log_head.h"
#include "util/co_coroutine.h"
#include "util/multi_threads_exec.h"
#include <format>
#include <iostream>
#include <thread>
#include <list>

class TestMultiThreadsExec
{
public:
    void Test()
    {
        std::list<std::thread> exec_threads;
        // 准备
        for (int64_t i = 0; i < thread_number_; ++i)
        {
            exec_threads.push_back(std::thread(
                [this]() {
                    multi_thread_exec_.Run();
                }));
        }

        for (int64_t i = 0; i < show_time; ++i)
        {
            Show(i);
        }

        multi_thread_exec_.Stop();

        for (auto& exce_thread : exec_threads)
        {
            exce_thread.join();
        }

        multi_thread_exec_.Wait();

    }

private:
    jaf::Coroutine<void> Show(size_t number)
    {
        co_await multi_thread_exec_.Switch(); // 切换线程执行

        LOG_INFO() << std::to_string(number);

        co_return;
    }

private:
    const int64_t thread_number_ = 10;
    const int64_t show_time      = 1000;

    jaf::MultiThreadsExec multi_thread_exec_;
};

void test_multi_threads_exec()
{
    TestMultiThreadsExec test;
    test.Test();
    return;
}
