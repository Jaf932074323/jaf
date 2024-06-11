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

uint64_t Timer::StartTask(const STimerPara& para)
{
    assert(para.fun);
    assert(run_flag_);

    uint64_t time = get_time_->GetNowTime() + para.interval;

    std::unique_lock<std::mutex> ul(tasks_mutex_);

    uint64_t task_id_ = next_task_id_++;

    std::shared_ptr<STimerParaInter> inter_task = std::make_shared<STimerParaInter>(para, time, task_id_);
    tasks_time_id_.insert(std::make_pair(task_id_, inter_task));
    if (auto it = tasks_time_time_.find(time); it == tasks_time_time_.end())
    {
        tasks_time_time_.insert(std::make_pair(time, std::map<uint64_t, std::shared_ptr<STimerParaInter>>({{task_id_, inter_task}})));
    }
    else
    {
        it->second.insert(std::make_pair(task_id_, inter_task));
    }

    m_workCondition.notify_all();

    return task_id_;
}

void Timer::Clear()
{
    std::unique_lock<std::mutex> ul(tasks_mutex_);
    tasks_time_id_.clear();
    tasks_time_time_.clear();
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
        std::function<void(ETimerResultType result_type, uint64_t task_id)> fun = timer_task->m_timerTask.fun;
        uint64_t task_id                                                        = timer_task->task_id;
        thread_pool_->Post([fun, task_id]() { fun(ETimerResultType::TRT_TASK_STOP, task_id); });
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
            assert(it_2.second->m_timerTask.fun);
            std::function<void(ETimerResultType result_type, uint64_t task_id)> fun = it_2.second->m_timerTask.fun;
            uint64_t task_id                                                        = it_2.second->task_id;
            thread_pool_->Post([fun, task_id]() { fun(ETimerResultType::TRT_TIMER_STOP, task_id); });
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
            assert(it_2.second->m_timerTask.fun);
            std::function<void(ETimerResultType result_type, uint64_t task_id)> fun = it_2.second->m_timerTask.fun;
            uint64_t task_id                                                        = it_2.second->task_id;
            thread_pool_->Post([fun, task_id, result_type]() { fun(result_type, task_id); });
        }
    }
}

} // namespace time
} // namespace jaf