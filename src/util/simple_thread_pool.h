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
#include "i_thread_pool.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace jaf
{

// 简易线程池
class SimpleThreadPool : public IThreadPool
{
public:
    SimpleThreadPool(size_t thread_count = 10)
        : thread_count_(thread_count)
    {
        Start();
    }
    ~SimpleThreadPool()
    {
        Stop();
    }

    virtual void Post(std::function<void(void)> fun)
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        tasks_.emplace(std::move(fun));
        condition_.notify_one();
    }

private:
    void Start()
    {
        for (size_t i = 0; i < thread_count_; ++i)
        {
            workers_.emplace_back([this]() { WorkerRun(); });
        }
    }
    void Stop()
    {
        stop_ = true;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.notify_all();
        }

        for (auto& worker : workers_)
        {
            worker.join();
        }
    }
    void WorkerRun()
    {
        while (!stop_)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                if (stop_)
                {
                    return;
                }
                if (tasks_.empty())
                {
                    continue;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

private:
    bool stop_ = false;

    size_t thread_count_; // 线程个数
    std::vector<std::thread> workers_;

    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::queue<std::function<void(void)>> tasks_;
};

} // namespace jaf
