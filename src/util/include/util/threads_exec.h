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
#include "co_coroutine.h"
#include "concurrent_queue.h"
#include "latch.h"
#include <assert.h>
#include <functional>

namespace jaf
{

// 线程执行
class ThreadsExec
{
public:
    ThreadsExec()
    {
    }
    ~ThreadsExec()
    {
    }

    void Start()
    {
        stop_ = false;
    }

    // 运行,每调用一次则多一个工作线程
    // 会阻塞直到停止
    void Run()
    {
        WorkerRun();
    }

    void Stop()
    {
        stop_ = true;
        task_queue_.QuitAllWait();
    }


public:
    // 切换线程的等待对象
    class SwitchAwaitable
    {
        friend ThreadsExec;
        SwitchAwaitable(ThreadsExec* simple_thread_exec)
            : simple_thread_exec_(simple_thread_exec)
        {
            assert(simple_thread_exec != nullptr);
        }
    public:
        ~SwitchAwaitable() {}

        // 禁止拷贝
        SwitchAwaitable(const SwitchAwaitable &) = delete;
        SwitchAwaitable &operator=(const SwitchAwaitable &) = delete;

    public:
        bool await_ready()
        {
            return false;
        }
        void await_suspend(std::coroutine_handle<> co_handle)
        {
            handle_ = co_handle;

            std::function<void(void)> callback = std::bind(&SwitchAwaitable::TaskCallback, this);
            simple_thread_exec_->task_queue_.Push(callback);
        }
        void await_resume() const
        {
            return;
        }
        void TaskCallback()
        {
            handle_.resume();
        }

    private:
        ThreadsExec* simple_thread_exec_;
        std::coroutine_handle<> handle_;
    };

public:
    // 获取切换线程的等待对象
    SwitchAwaitable Switch()
    {
        assert(!stop_);
        return SwitchAwaitable(this);
    }

    void Post(std::function<void()> fun)
    {
        assert(!stop_);
        task_queue_.Push(fun);
    }

private:
    void WorkerRun()
    {
        std::function<void()> task;

        while (!stop_)
        {
            bool wait_result = task_queue_.WaitAndPop(task);
            if (!wait_result)
            {
                continue;
            }
            task();
        }

        // 处理掉剩余的任务
        while (task_queue_.TryPop(task))
        {
            task();
        }
    }

private:
    bool stop_ = false;
    ConcurrentQueue<std::function<void(void)>> task_queue_;
};

} // namespace jaf
