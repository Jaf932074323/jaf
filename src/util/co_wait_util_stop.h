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
#include "util/co_coroutine.h"
#include <assert.h>
#include <mutex>

namespace jaf
{

// 等待通知
class CoWaitUtilStop
{
public:
    CoWaitUtilStop(){};
    virtual ~CoWaitUtilStop(){};

public:
    void Start()
    {
        awaitable_.Start();
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
            return false;
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

    private:
        std::coroutine_handle<> handle_;

        std::mutex wait_flag_mutex_;
        bool run_flag_  = false;
        bool wait_flag_ = false;
    };

private:
    Awaitable awaitable_;

    bool wait_flag_ = false;
};

} // namespace jaf