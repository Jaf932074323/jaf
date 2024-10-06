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

// 等待所有任务完成
class CoWaitAllTasksDone
{
public:
    CoWaitAllTasksDone(){};
    virtual ~CoWaitAllTasksDone(){};

public:
    void CountUp(uint64_t update = 1)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ++count_;
    }

    void CountDown(uint64_t update = 1)
    {
        bool resume_flag = false;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            assert(count_ >= update);
            count_ -= update;
            if (count_ == 0 && wait_flag_)
            {
                wait_flag_  = false;
                resume_flag = true;
            }
        }

        if (resume_flag)
        {
            handle_.resume();
        }
    }

    bool await_ready() const
    {
        return false;
    }

    bool await_suspend(std::coroutine_handle<> co_handle)
    {
        handle_ = co_handle;

        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == 0)
        {
            return false;
        }
        assert(!wait_flag_); // 只能等待一个
        wait_flag_ = true;

        return true;
    }

    void await_resume() const
    {
        return;
    }

private:
    std::coroutine_handle<> handle_;

    std::mutex mutex_;
    bool wait_flag_ = false; // 当前是否正在等待状态
    uint64_t count_ = 0;     // 计数
};

} // namespace jaf