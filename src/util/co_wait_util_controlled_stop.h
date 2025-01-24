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
// 2024-9-27 姜安富
#include "util/control_start_stop.h"
#include <assert.h>
#include <memory>
#include <mutex>

namespace jaf
{

// 等待直到被控制停止
struct CoWaitUtilControlledStop
{
    CoWaitUtilControlledStop(ControlStartStop& control_start_stop)
    {
        agent_ = control_start_stop.Register([this]() { InterStop(); });

        if (agent_ == nullptr)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            run_flag_ = false;
        }
    }

    ~CoWaitUtilControlledStop()
    {
    }

    void Stop()
    {
        if (agent_ == nullptr)
        {
            return;
        }
        agent_->Stop();
    }

private:
    void InterStop()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            assert(run_flag_ ? true : !wait_flag_); // 如果没有运行，则一定也不会等待
            run_flag_ = false;
            if (!wait_flag_)
            {
                return;
            }
            wait_flag_ = false;
        }
        agent_ = nullptr;

        handle_.resume();
    }

public:
    void Notify()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
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
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!run_flag_)
            {
                return false;
            }
            assert(!wait_flag_); // 不能同时等待多次
            wait_flag_ = true;
        }

        handle_ = co_handle;
        return true;
    }

    void await_resume() const
    {
        return;
    }

private:
    std::coroutine_handle<> handle_;

    std::shared_ptr<ControlStartStop::Agent> agent_;

    std::mutex mutex_;
    bool run_flag_  = true;  // 当前是否正在运行
    bool wait_flag_ = false; // 当前是否有等待对象
};

} // namespace jaf