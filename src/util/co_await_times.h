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
    void Start()
    {
        awaitable_.Start();
    }

    void Stop()
    {
        awaitable_.Stop();
    }

    void SetTimes(size_t times)
    {
        awaitable_.SetTimes(times);
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

        void Start()
        {
            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            run_flag_ = true;
        }

        void Stop()
        {
            {
                std::unique_lock<std::mutex> lock(wait_flag_mutex_);
                run_flag_ = false;
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
            return times_ == 0;
        }

        bool await_suspend(std::coroutine_handle<> co_handle)
        {
            handle_ = co_handle;

            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
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

        void SetTimes(size_t times)
        {
            assert(!wait_flag_); // 不能在等待期间修改次数
            times_ = times;
        }

        void Notify()
        {
            {
                std::unique_lock<std::mutex> lock(wait_flag_mutex_);
                if (!wait_flag_)
                {
                    return;
                }

                --times_;
                if (times_ != 0)
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