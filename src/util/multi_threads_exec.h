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
// 2024-6-23 姜安富
#include "co_wait_notice.h"
#include "threads_exec.h"
#include <thread>
#include <vector>

namespace jaf
{

// 多线程执行
class MultiThreadsExec
{
public:
    MultiThreadsExec(size_t thread_count)
        : thread_count_(thread_count)
    {
    }
    ~MultiThreadsExec()
    {
    }

    void Start()
    {
        if (runing_)
        {
            return;
        }
        runing_ = true;

        threads_exec_.Start();

        threads_.reserve(thread_count_);

        auto fun_thread_run = [this]() {
            threads_exec_.Run();
        };
        for (size_t i = 0; i < thread_count_; ++i)
        {
            threads_.push_back(std::thread(fun_thread_run));
        }
    }

    void Stop()
    {
        if (!runing_)
        {
            return;
        }
        runing_ = false;

        threads_exec_.Stop();
        for (auto& thread : threads_)
        {
            thread.join();
        }
    }

    // 获取切换线程的等待对象
    auto Switch()
    {
        assert(runing_);
        return threads_exec_.Switch();
    }

    // 直接投递要执行的任务，不使用协程功能
    void Post(std::function<void()> fun)
    {
        threads_exec_.Post(fun);
    }

private:
    bool runing_ = false;

    ThreadsExec threads_exec_;

    size_t thread_count_;
    std::vector<std::thread> threads_;
};

} // namespace jaf
