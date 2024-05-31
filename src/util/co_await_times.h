#pragma once
#include "util/co_coroutine.h"
#include <assert.h>
#include <mutex>

namespace jaf
{

// �ȴ�һ������
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
        assert(!wait_flag_); // ͬʱֻ�ܵȴ�һ��
        wait_flag_ = true;

        co_await awaitable_;

        wait_flag_ = false;

        co_return;
    }

    // ֪ͨ
    // Ҫ֪ͨ��β���Ч��֪ͨ������SetTimes����
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
            assert(!wait_flag_); // �����ڵȴ��ڼ��޸Ĵ���
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

                wait_flag_    = false;
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