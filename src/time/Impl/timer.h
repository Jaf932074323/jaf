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
// 2024-6-16 姜安富
#include "interface/i_get_time.h"
#include "interface/i_timer.h"
#include "util/i_thread_pool.h"
#include "util/latch.h"
#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <mutex>

namespace jaf
{
namespace time
{


// 定时器
class Timer : public ITimer
{
public:
    // min_level通过的最低日志等级
    Timer(std::shared_ptr<IThreadPool> thread_pool = nullptr, std::shared_ptr<IGetTime> get_time = nullptr);
    virtual ~Timer();

public:
    // 启动定时
    virtual void Start() override;
    // 停止定时
    virtual void Stop() override;

    // 启动一个定时任务
    // task 定时任务信息
    // 返回是否成功
    // 当StartTask成功后，para中的定时执行函数一定要执行一次，且只会执行一次
    virtual bool StartTask(STimerTask* task);
    // 停止一个定时任务
    // task 定时任务信息
    virtual void StopTask(STimerTask* task);

private:
    struct STimerKey
    {
        uint64_t time    = 0; // 执行时间
        uint64_t task_id = 0; // 定时任务ID

        bool operator==(const STimerKey& other) const
        {
            return this->time == other.time && this->task_id == other.task_id;
        }

        bool operator<(const STimerKey& other) const
        {
            if (this->time != other.time)
            {
                return this->time < other.time;
            }
            return this->task_id < other.task_id;
        }
    };

    // 内部使用的定时任务
    struct STimerParaInter
    {
        STimerTask* timer_task = nullptr;
        std::function<void(ETimerResultType result_type, STimerTask* task)> fun; // 定时执行函数
        STimerKey key;
    };

    // 停止一个定时任务
    // task_id 要移除的定时任务的Id
    virtual void StopTask(STimerKey key);

public:
    // 定时器工作线程执行函数
    virtual void Work();

private:
    // 执行达到时间的任务
    virtual void GainNeedExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks);
    virtual void ExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks, ETimerResultType result_type);

private:
    std::shared_ptr<IThreadPool> thread_pool_;
    std::shared_ptr<IGetTime> get_time_;

    std::atomic<bool> run_flag_ = false; // 工作线程运行标志

    uint64_t next_task_id_ = 1; // 下一个定时任务ID

    std::atomic<uint64_t> lead_time_ = 5; // 执行任务的提前量，每个任务可以提前lead_time_毫秒执行

    Latch work_threads_latch_{1};
    std::condition_variable_any m_workCondition;                       // 定时用条件变量，用其超时特性来定时，在定时的过程中也能随时唤醒
    std::mutex tasks_mutex_;                                           // 定时任务锁
    std::map<STimerKey, std::shared_ptr<STimerParaInter>> tasks_time_; // 定时任务集合
};

} // namespace time
} // namespace jaf