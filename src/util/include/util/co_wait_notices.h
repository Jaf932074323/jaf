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

// 等待多次通知
struct CoWaitNotices
{
    CoWaitNotices()
    {
    }

    ~CoWaitNotices() {}

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
        return false;
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

    // 重置通知次数，重置之后需要重新通知times次
    void Reset(size_t times)
    {
        assert(times != 0);

        {
            std::unique_lock<std::mutex> lock(wait_flag_mutex_);
            times_ = times;
        }
    }

    // 通知
    // 要通知多次才生效
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

} // namespace jaf