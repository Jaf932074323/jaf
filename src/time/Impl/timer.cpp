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
#include "get_time_tick.h"
#include "util/red_black_tree.h"
#include <assert.h>
#include <list>

namespace jaf
{
namespace time
{

Timer::Timer(std::shared_ptr<IGetTime> get_time)
    : get_time_(get_time == nullptr ? std::make_shared<GetTimeTick>() : get_time)
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

    run_thread_ = std::thread([this]() { Work(); });
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

    assert(std::this_thread::get_id() != run_thread_.get_id());
    run_thread_.join();
}

bool Timer::StartTask(STimerTask* task)
{
    assert(task->fun);
    assert(task->timer_data_ == nullptr);
    assert(run_flag_);

    uint64_t time = get_time_->GetNowTime() + task->interval;

    std::unique_lock<std::mutex> ul(tasks_mutex_);

    uint64_t task_id = next_task_id_++;

    std::shared_ptr<STimerParaInter> inter_task = std::make_shared<STimerParaInter>(task, task->fun, time, this, true);
    task->timer_data_                           = inter_task.get();
    auto it                                     = tasks_time_.Insert(time, inter_task);
    inter_task->timer_node                      = it.GetNode();
    m_workCondition.notify_all();

    return true;
}

void Timer::StopTask(STimerTask* task)
{
    if (task->timer_data_ == nullptr)
    {
        return;
    }
    STimerParaInter* inter_task = (STimerParaInter*) task->timer_data_;
    assert(inter_task->timer_ == this);
    assert(inter_task->timer_task == task);

    std::unique_lock<std::mutex> ul(tasks_mutex_);
    if (!inter_task->timing_flag_)
    {
        return;
    }
    inter_task->timing_flag_ = false;
    TimerTree::Node* timer_node = inter_task->timer_node;
    tasks_stop_.push_back(timer_node->value_);
    tasks_time_.Erase(timer_node);
    m_workCondition.notify_all();
}

void Timer::Work()
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

        if (tasks_time_.Empty())
        {
            m_workCondition.wait(ul);
        }
        else
        {
            min_wait_time    = tasks_time_.begin().GetValue()->time - get_time_->GetNowTime();
            auto wait_result = m_workCondition.wait_for(ul, std::chrono::milliseconds(min_wait_time));
        }
        if (!run_flag_)
        {
            break;
        }

        GainNeedExecuteTasks(need_execute_tasks, need_stop_tasks);
    }

    // 处理剩余的所有定时任务
    TimerTree residue_tasks_time_time;
    {
        std::unique_lock<std::mutex> ul(tasks_mutex_);
        residue_tasks_time_time.Swap(tasks_time_);
        std::swap(need_stop_tasks, tasks_stop_);
    }

    ExecuteTasks(need_stop_tasks, ETimerResultType::TRT_TASK_STOP);

    for (auto it = residue_tasks_time_time.begin(); it != residue_tasks_time_time.end(); ++it)
    {
        assert(it.GetValue()->fun);
        assert(it.GetValue()->timer_task != nullptr);
        assert(it.GetValue()->timer_task->timer_data_ != nullptr);
        std::function<void(ETimerResultType result_type, STimerTask * task)> fun = it.GetValue()->fun;
        STimerTask* timer_task                                                   = it.GetValue()->timer_task;
        timer_task->timer_data_                                                  = nullptr;
        it.GetValue()->timer_task                                                = nullptr;
        fun(ETimerResultType::TRT_TIMER_STOP, timer_task);
    }
}

void Timer::GainNeedExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks, std::list<std::shared_ptr<STimerParaInter>>& need_stop_tasks)
{
    uint64_t now_time      = get_time_->GetNowTime();
    uint64_t deadline_time = now_time + (int64_t) lead_time_.load();

    auto end_it = tasks_time_.UpperBound(deadline_time);
    for (auto it = tasks_time_.begin(); it != end_it; ++it)
    {
        assert(it.GetValue()->timing_flag_);
        it.GetValue()->timing_flag_ = false;
        need_execute_tasks.emplace_back(std::move(it.GetValue()));
    }
    for (auto it = tasks_time_.begin(); it != end_it;)
    {
        it = tasks_time_.Erase(it);
    }

    std::swap(need_stop_tasks, tasks_stop_);
}

void Timer::ExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks, ETimerResultType result_type)
{
    for (auto it : need_execute_tasks)
    {
        assert(it->fun);
        assert(it->timer_task != nullptr);
        assert(it->timer_task->timer_data_ != nullptr);
        std::function<void(ETimerResultType result_type, STimerTask * task)> fun = it->fun;
        STimerTask* timer_task                                                   = it->timer_task;
        timer_task->timer_data_                                                  = nullptr;
        it->timer_task                                                           = nullptr;
        fun(result_type, timer_task);
    }
}

} // namespace time
} // namespace jaf