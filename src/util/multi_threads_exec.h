#pragma once
#include "co_coroutine.h"
#include "concurrent_queue.h"
#include "latch.h"
#include <assert.h>
#include <functional>

namespace jaf
{

// 线程池执行
class MultiThreadsExec
{
public:
    MultiThreadsExec()
    {
    }
    ~MultiThreadsExec()
    {
    }

    // 运行,每调用一次则多一个工作线程
    // 会阻塞直到停止
    void Run()
    {
        CountUpWorker();
        WorkerRun();
        stop_ = false;
        CountDownWorker();
    }

    void Stop()
    {
        stop_ = true;
        task_queue_.QuitAllWait();
    }

    void Wait()
    {
        WaitZeroWorker();
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

private:
    void CountUpWorker(const int64_t update = 1)
    {
        assert(update > 0);
        const int64_t current = worker_counter_.fetch_add(update);
        assert(current >= 0);
    }
    void CountDownWorker(const int64_t update = 1)
    {
        assert(update >= 0);
        const int64_t current = worker_counter_.fetch_sub(update) - update;
        if (current == 0)
        {
            worker_counter_.notify_all();
        }
        else
        {
            assert(current >= 0);
        }
    }

    void WaitZeroWorker() const noexcept
    {
        for (;;)
        {
            const int64_t current = worker_counter_.load();
            if (current == 0)
            {
                return;
            }
            else
            {
                assert(current >= 0);
            }
            worker_counter_.wait(current, std::memory_order_relaxed);
        }
    }

private:
    class SwitchAwaitable
    {
    public:
        SwitchAwaitable(MultiThreadsExec* simple_thread_exec)
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
        MultiThreadsExec* simple_thread_exec_;
        std::coroutine_handle<> handle_;
    };

private:
    bool stop_ = false;
    ConcurrentQueue<std::function<void(void)>> task_queue_;

    std::atomic<int64_t> worker_counter_ = 0; // 工作线程计数
};

} // namespace jaf
