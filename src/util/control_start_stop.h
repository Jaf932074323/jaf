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
#include "util/co_coroutine.h"
#include <assert.h>
#include <functional>
#include <memory>
#include <mutex>
#include <set>

namespace jaf
{

// 控制执行
class ControlStartStop
{
public:
    class Agent
    {
    public:
        Agent(std::function<void()> fun_stop)
            : fun_stop_(fun_stop)
        {
        }

        ~Agent()
        {
        }

        void SetUnregisterFun(std::function<void()> fun_unregister)
        {
            fun_unregister_ = fun_unregister;
        }

        void Stop()
        {
            std::function<void()> fun_stop;
            std::function<void()> fun_unregister;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                fun_unregister  = fun_unregister_;
                fun_stop        = fun_stop_;
                fun_unregister_ = []() {};
                fun_stop_       = []() {};
            }

            fun_unregister();
            fun_stop();
        }

    private:
        std::mutex mutex_;
        std::function<void()> fun_unregister_;
        std::function<void()> fun_stop_;
    };

public:
    ControlStartStop(){};
    virtual ~ControlStartStop(){};

public:
    std::shared_ptr<Agent> Register(std::function<void()> fun_stop)
    {
        std::shared_ptr<Agent> agent = std::make_shared<Agent>(fun_stop);
        agent->SetUnregisterFun([this, agent]() { Unregister(agent); });

        std::unique_lock<std::mutex> lock(mutex_);
        if (run_flag_)
        {
            be_controls_.insert(agent);
            return agent;
        }
        else
        {
            return nullptr;
        }
    }

    void Unregister(std::shared_ptr<Agent> agent)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        be_controls_.erase(agent);
    }

    bool IsRun()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return run_flag_;
    }

    void Start()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (run_flag_)
        {
            return;
        }
        run_flag_ = true;
    }

    void Stop()
    {
        std::set<std::shared_ptr<Agent>> be_controls;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!run_flag_)
            {
                return;
            }
            run_flag_ = false;
            std::swap(be_controls_, be_controls);
        }

        for (std::shared_ptr<Agent> be_control : be_controls)
        {
            be_control->Stop();
        }
    }

private:
    std::mutex mutex_;
    bool run_flag_ = false;
    std::set<std::shared_ptr<Agent>> be_controls_;
};

} // namespace jaf