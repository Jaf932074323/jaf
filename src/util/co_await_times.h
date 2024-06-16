#pragma once
#include "util/co_coroutine.h"
#include <assert.h>
#include <mutex>

namespace jaf
{

// 等待一定次数
class CoAwaitTimes
{
public:
    CoAwaitTimes(){};
    virtual ~CoAwaitTimes(){};

public:
    void Start(size_t times)
    {
        awaitable_.Start(times);
    }

    void Stop()
    {
        awaitable_.Stop();
    }

    jaf::Coroutine<void> Wait()
    {
        assert(!wait_flag_); // 同时只能等待一个
        wait_flag_ = true;

        co_await awaitable_;

        wait_flag_ = false;

        co_return;
    }

    // 通知
    // 要通知多次才生效，通知次数由SetTimes设置
    void Notify()
    {
        awaitable_.Notify();
    }

private:
    struct Awaitable
    {
        Awaitable()
        {
        }

        ~Awaitable() {}

        void Start(size_t times)
        {
            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            run_flag_ = true;
            times_    = times;
        }

        void Stop()
        {
            {
                std::unique_lock<std::mutex> lock(wait_flag_mutex_);
                run_flag_ = false;
                times_    = 0;
                if (!wait_flag_)
                {
                    return;
                }
                wait_flag_ = false;
            }
            handle_.resume();
        }

        bool await_ready() const
        {
            return true;
        }

        bool await_suspend(std::coroutine_handle<> co_handle)
        {
            handle_ = co_handle;

            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            if (times_ == 0)
            {
                return false;
            }
            if (!run_flag_)
            {
                return false;
            }

            assert(!wait_flag_);
            wait_flag_ = true;

            return true;
        }

        void await_resume() const
        {
            return;
        }

        void Notify()
        {
            {
                std::unique_lock<std::mutex> lock(wait_flag_mutex_);
                if (times_ != 0)
                {
                    --times_;
                }
                if (times_ != 0)
                {
                    return;
                }
                if (!wait_flag_)
                {
                    return;
                }

                wait_flag_ = false;
            }

            handle_.resume();
        }

    private:
        std::coroutine_handle<> handle_;

        std::mutex wait_flag_mutex_;
        size_t times_   = 0;
        bool run_flag_  = false;
        bool wait_flag_ = false;
    };

private:
    Awaitable awaitable_;
    bool wait_flag_ = false;
};

} // namespace jaf