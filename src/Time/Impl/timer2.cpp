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
#include "timer2.h"
#include "get_time_tick.h"
#include <assert.h>
#include <list>

namespace jaf
{
namespace time
{

Timer2::Timer2(std::shared_ptr<IGetTime> get_time)
    : get_time_(get_time == nullptr ? std::make_shared<GetTimeTick>() : get_time)
{
    run_thread_ = std::thread([this]() { Work(); });
}

Timer2::~Timer2()
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

    assert(std::this_thread::get_id() != run_thread_.get_id());
    run_thread_.join();
}

bool Timer2::StartTask(STimerTask* task)
{
    assert(task->fun);
    assert(task->timer_data_ == nullptr);
    assert(run_flag_);

    uint64_t time = get_time_->GetNowTime() + task->interval;

    std::unique_lock<std::mutex> ul(tasks_mutex_);

    uint64_t task_id = next_task_id_++;

    STimerKey timer_key{time, task_id};

    std::shared_ptr<STimerParaInter> inter_task = std::make_shared<STimerParaInter>(task, task->fun, timer_key, this);
    task->timer_data_                           = inter_task.get();
    tasks_time_.insert(std::make_pair(timer_key, inter_task));

    m_workCondition.notify_all();

    return true;
}

void Timer2::StopTask(STimerTask* task)
{
    if (task->timer_data_ == nullptr)
    {
        return;
    }
    STimerParaInter* inter_task = (STimerParaInter*) task->timer_data_;
    assert(inter_task->timer_ == this);
    assert(inter_task->timer_task == task);
    StopTask(inter_task->key);
}

void Timer2::StopTask(STimerKey key)
{

    std::unique_lock<std::mutex> ul(tasks_mutex_);
    auto it = tasks_time_.find(key);
    if (it == tasks_time_.end())
    {
        return;
    }

    tasks_stop_.push_back(it->second);
    tasks_time_.erase(it);
    m_workCondition.notify_all();
}

void Timer2::Work()
{
    std::list<std::shared_ptr<STimerParaInter>> need_execute_tasks;
    std::list<std::shared_ptr<STimerParaInter>> need_stop_tasks;
    uint64_t min_wait_time = 0xffffffffffffffff; // 所有任务中最小的等待时间，用于计算下一次执行需要多久

    {
        std::unique_lock<std::mutex> ul(tasks_mutex_);
        GainNeedExecuteTasks(need_execute_tasks, need_stop_tasks);
    }

    while (true)
    {
        ExecuteTasks(need_execute_tasks, ETimerResultType::TRT_SUCCESS);
        ExecuteTasks(need_stop_tasks, ETimerResultType::TRT_TASK_STOP);
        need_execute_tasks.clear();
        need_stop_tasks.clear();

        std::unique_lock<std::mutex> ul(tasks_mutex_);
        if (!run_flag_)
        {
            break;
        }

        if (!tasks_stop_.empty())
        {
            GainNeedExecuteTasks(need_execute_tasks, need_stop_tasks);
            continue;
        }

        if (tasks_time_.empty())
        {
            m_workCondition.wait(ul);
        }
        else
        {
            min_wait_time    = tasks_time_.begin()->first.time - get_time_->GetNowTime();
            auto wait_result = m_workCondition.wait_for(ul, std::chrono::milliseconds(min_wait_time));
        }
        if (!run_flag_)
        {
            break;
        }

        GainNeedExecuteTasks(need_execute_tasks, need_stop_tasks);
    }

    // 处理剩余的所有定时任务
    std::map<STimerKey, std::shared_ptr<STimerParaInter>> residue_tasks_time_time;
    {
        std::unique_lock<std::mutex> ul(tasks_mutex_);
        std::swap(residue_tasks_time_time, tasks_time_);
        std::swap(need_stop_tasks, tasks_stop_);
    }

    ExecuteTasks(need_stop_tasks, ETimerResultType::TRT_TASK_STOP);
    for (auto it : residue_tasks_time_time)
    {
        assert(it.second->fun);
        std::function<void(ETimerResultType result_type, STimerTask * task)> fun = it.second->fun;
        STimerTask* timerTask                                                    = it.second->timer_task;
        timerTask->timer_data_                                                   = nullptr;
        it.second->timer_task                                                    = nullptr;
        fun(ETimerResultType::TRT_TIMER_STOP, timerTask);
    }
}

void Timer2::GainNeedExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks, std::list<std::shared_ptr<STimerParaInter>>& need_stop_tasks)
{
    uint64_t now_time      = get_time_->GetNowTime();
    uint64_t deadline_time = now_time + (int64_t) lead_time_.load();

    auto end_it = tasks_time_.upper_bound(STimerKey{deadline_time, 0});
    for (auto it = tasks_time_.begin(); it != end_it; ++it)
    {
        need_execute_tasks.emplace_back(std::move(it->second));
    }
    tasks_time_.erase(tasks_time_.begin(), end_it);

    std::swap(need_stop_tasks, tasks_stop_);
}

void Timer2::ExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks, ETimerResultType result_type)
{
    for (auto it : need_execute_tasks)
    {
        assert(it->fun);
        std::function<void(ETimerResultType result_type, STimerTask * task)> fun = it->fun;
        STimerTask* timerTask                                                    = it->timer_task;
        timerTask->timer_data_                                                   = nullptr;
        it->timer_task                                                           = nullptr;
        fun(result_type, timerTask);
    }
}

} // namespace time
} // namespace jaf