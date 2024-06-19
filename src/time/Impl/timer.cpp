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
#include "timer.h"
#include "tick_timer.h"
#include "util/finally.h"
#include "util/simple_thread_pool.h"
#include <assert.h>
#include <list>

namespace jaf
{
namespace time
{

Timer::Timer(std::shared_ptr<IThreadPool> thread_pool, std::shared_ptr<IGetTime> get_time)
    : thread_pool_(thread_pool == nullptr ? std::make_shared<SimpleThreadPool>(5) : thread_pool)
    , get_time_(get_time == nullptr ? std::make_shared<CTickTimer>() : get_time)
{
}

Timer::~Timer()
{
}

void Timer::Start()
{
    {
        std::unique_lock<std::mutex> ul(tasks_mutex_);
        if (run_flag_)
        {
            return;
        }
        run_flag_ = true;
    }

    work_threads_latch_.Reset();
    thread_pool_->Post([this]() { Work(); });
}

void Timer::Stop()
{
    {
        std::unique_lock<std::mutex> ul(tasks_mutex_);
        if (!run_flag_)
        {
            return;
        }
        run_flag_ = false;

        m_workCondition.notify_all();
    }

    work_threads_latch_.Wait();
}

bool Timer::StartTask(STimerTask* task)
{
    assert(task->fun);
    assert(task->timer_data_ == nullptr);
    assert(run_flag_);

    uint64_t time = get_time_->GetNowTime() + task->interval;

    std::unique_lock<std::mutex> ul(tasks_mutex_);

    uint64_t task_id = next_task_id_++;

    std::shared_ptr<STimerParaInter> inter_task = std::make_shared<STimerParaInter>(task, task->fun, time, task_id);
    task->timer_data_                           = inter_task.get();
    tasks_time_id_.insert(std::make_pair(task_id, inter_task));
    if (auto it = tasks_time_time_.find(time); it == tasks_time_time_.end())
    {
        tasks_time_time_.insert(std::make_pair(time, std::map<uint64_t, std::shared_ptr<STimerParaInter>>({{task_id, inter_task}})));
    }
    else
    {
        it->second.insert(std::make_pair(task_id, inter_task));
    }

    m_workCondition.notify_all();

    return true;
}

void Timer::StopTask(STimerTask* task)
{
    if (task->timer_data_ == nullptr)
    {
        return;
    }
    STimerParaInter* inter_task = (STimerParaInter*)task->timer_data_;
    assert(inter_task->m_timerTask == task);
    StopTask(inter_task->task_id);
}

void Timer::StopTask(uint64_t task_id)
{
    std::shared_ptr<STimerParaInter> timer_task = nullptr;
    {
        std::unique_lock<std::mutex> ul(tasks_mutex_);
        auto it = tasks_time_id_.find(task_id);
        if (it == tasks_time_id_.end())
        {
            return;
        }

        auto time_it = tasks_time_time_.find(it->second->time);

        tasks_time_id_.erase(task_id);

        if (time_it == tasks_time_time_.end())
        {
            return;
        }

        auto time_it2 = time_it->second.find(task_id);
        if (time_it2 != time_it->second.end())
        {
            timer_task = time_it2->second;
            time_it->second.erase(time_it2);
        }

        if (time_it->second.empty())
        {
            tasks_time_time_.erase(time_it);
        }
    }

    if (timer_task != nullptr)
    {
        assert(timer_task->fun);
        std::function<void(ETimerResultType result_type, STimerTask * task)> fun = timer_task->fun;
        STimerTask* timerTask                                                    = timer_task->m_timerTask;
        timerTask->timer_data_                                                   = nullptr;
        timer_task->m_timerTask                                                  = nullptr;
        thread_pool_->Post([fun, timerTask]() { fun(ETimerResultType::TRT_TASK_STOP, timerTask); });
    }
}

void Timer::Work()
{
    FINALLY(work_threads_latch_.CountDown(););

    std::list<std::map<uint64_t, std::shared_ptr<STimerParaInter>>> need_execute_tasks;
    uint64_t min_wait_time = 0xffffffffffffffff; // 所有任务中最小的等待时间，用于计算下一次执行需要多久

    {
        std::unique_lock<std::mutex> ul(tasks_mutex_);
        GainNeedExecuteTasks(need_execute_tasks);
    }

    while (true)
    {
        ExecuteTasks(need_execute_tasks, ETimerResultType::TRT_SUCCESS);
        need_execute_tasks.clear();

        std::unique_lock<std::mutex> ul(tasks_mutex_);
        if (!run_flag_)
        {
            break;
        }

        if (tasks_time_time_.empty())
        {
            m_workCondition.wait(ul);
        }
        else
        {
            min_wait_time    = tasks_time_time_.begin()->first - get_time_->GetNowTime();
            auto wait_result = m_workCondition.wait_for(ul, std::chrono::milliseconds(min_wait_time));
        }
        if (!run_flag_)
        {
            break;
        }

        GainNeedExecuteTasks(need_execute_tasks);
    }

    // 处理剩余的所有定时任务
    std::map<uint64_t, std::map<uint64_t, std::shared_ptr<STimerParaInter>>> residue_tasks_time_time_;
    {
        std::unique_lock<std::mutex> ul(tasks_mutex_);
        residue_tasks_time_time_ = std::move(tasks_time_time_);

        tasks_time_id_.clear();
        tasks_time_time_.clear();
    }

    for (auto it : residue_tasks_time_time_)
    {
        for (auto it_2 : it.second)
        {
            assert(it_2.second->fun);
            std::function<void(ETimerResultType result_type, STimerTask * task)> fun = it_2.second->fun;
            STimerTask* timerTask                                                    = it_2.second->m_timerTask;
            timerTask->timer_data_                                                   = nullptr;
            it_2.second->m_timerTask                                                 = nullptr;
            thread_pool_->Post([fun, timerTask]() { fun(ETimerResultType::TRT_TIMER_STOP, timerTask); });
        }
    }
}

void Timer::GainNeedExecuteTasks(std::list<std::map<uint64_t, std::shared_ptr<STimerParaInter>>>& need_execute_tasks)
{
    uint64_t now_time = get_time_->GetNowTime();

    auto end_it = tasks_time_time_.upper_bound((now_time + (int64_t) lead_time_.load()));
    for (auto it = tasks_time_time_.begin(); it != end_it; ++it)
    {
        for (auto it_2 : it->second)
        {
            tasks_time_id_.erase(it_2.second->task_id);
        }

        need_execute_tasks.emplace_back(std::move(it->second));
    }
    tasks_time_time_.erase(tasks_time_time_.begin(), end_it);
}

void Timer::ExecuteTasks(std::list<std::map<uint64_t, std::shared_ptr<STimerParaInter>>>& need_execute_tasks, ETimerResultType result_type)
{
    for (auto it : need_execute_tasks)
    {
        for (auto it_2 : it)
        {
            assert(it_2.second->fun);
            std::function<void(ETimerResultType result_type, STimerTask * task)> fun = it_2.second->fun;
            STimerTask* timerTask                                                    = it_2.second->m_timerTask;
            timerTask->timer_data_                                                   = nullptr;
            it_2.second->m_timerTask                                                 = nullptr;

            thread_pool_->Post([fun, result_type, timerTask]() { fun(result_type, timerTask); });
        }
    }
}

} // namespace time
} // namespace jaf