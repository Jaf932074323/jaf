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
#include "threads_exec.h"
#include <thread>
#include "co_await.h"

namespace jaf
{

// 单线程执行
class SimpleThreadExec
{
public:
    SimpleThreadExec()
    {
    }
    ~SimpleThreadExec()
    {
    }

    jaf::Coroutine<void> Run()
    {
        assert(!runing_); // 不能重复调用run
        runing_ = true;
        wait_stop_.Start();
        threads_exec_.Start();


        auto fun_thread_run = [this]() {
            threads_exec_.Run();
            };
        std::thread work_thread(fun_thread_run);

        co_await wait_stop_.Wait();

        threads_exec_.Stop();
        work_thread.join();

        co_return;
    }

    void Stop()
    {
        wait_stop_.Stop();
    }

public:
    // 切换线程
    jaf::Coroutine<void> Switch()
    {
        assert(runing_);
        co_await threads_exec_.Switch();
    }

private:
    bool runing_ = false;
    CoAwait wait_stop_;

    ThreadsExec threads_exec_;
};

} // namespace jaf
