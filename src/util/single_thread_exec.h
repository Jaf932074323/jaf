#pragma once
#include "co_coroutine.h"
#include "concurrent_queue.h"
#include "latch.h"
#include <assert.h>
#include <functional>

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

    // 运行
    // 会阻塞直到停止
    void Run()
    {
        assert(!stop_);
        WorkerRun();
        stop_ = false;
        latch_.Reset();
        latch_.CountDown();
    }
    void Stop()
    {
        stop_ = true;
        task_queue_.QuitAllWait();
    }

    void Wait()
    {
        latch_.Wait();
        return;
    }

public:
    // 切换线程
    jaf::Coroutine<void> Switch()
    {
        assert(!stop_);
        co_await SwitchAwaitable(this);
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

    class SwitchAwaitable
    {
    public:
        SwitchAwaitable(SimpleThreadExec* simple_thread_exec)
            : simple_thread_exec_(simple_thread_exec)
        {
        }
        ~SwitchAwaitable() {}

        bool await_ready()
        {
            return false;
        }
        bool await_suspend(std::coroutine_handle<> co_handle)
        {
            handle_ = co_handle;

            std::function<void(void)> callback = std::bind(&SwitchAwaitable::TaskCallback, this);
            simple_thread_exec_->task_queue_.Push(callback);

            return true;
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
        SimpleThreadExec* simple_thread_exec_;
        std::coroutine_handle<> handle_;
    };


private:
    bool stop_ = false;
    ConcurrentQueue<std::function<void(void)>> task_queue_;

    Latch latch_{1};
};

} // namespace jaf
