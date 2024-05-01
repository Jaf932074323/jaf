#pragma once
#include "Interface/i_get_time.h"
#include "Interface/i_timer.h"
#include "util/i_thread_pool.h"
#include <condition_variable>
#include <latch>
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
    // 启动一个定时任务
    // rTask 定时任务信息
    // 返回定时任务Id，返回0时表示添加定时任务失败
    virtual uint64_t StartTask(const STimerPara& task) override;
    // 清除所有定时任务
    virtual void Clear() override;
    // 停止一个定时任务
    // nTimeId 要移除的定时任务的Id
    virtual void StopTask(uint64_t task_id) override;

public:
    // 启动定时
    void Start();
    // 停止定时
    void Stop();

    // 定时器工作线程执行函数
    virtual void Work();

    // 内部使用的定时任务
    struct STimerParaInter
    {
        STimerPara m_timerTask;
        uint64_t time    = 0; // 执行时间
        uint64_t task_id = 0; // 定时任务ID
    };

private:
    // 执行达到时间的任务
    virtual void GainNeedExecuteTasks(std::list<std::map<uint64_t, std::shared_ptr<STimerParaInter>>>& need_execute_tasks);
    virtual void ExecuteTasks(std::list<std::map<uint64_t, std::shared_ptr<STimerParaInter>>>& need_execute_tasks);

private:
    std::shared_ptr<IThreadPool> thread_pool_;
    std::shared_ptr<IGetTime> get_time_;

    std::atomic<bool> run_flag_ = false; // 工作线程运行标志


    uint64_t next_task_id_ = 1; // 下一个定时任务ID

    std::atomic<uint64_t> lead_time_ = 5; // 执行任务的提前量，每个任务可以提前lead_time_毫秒执行

    std::shared_ptr<std::latch> work_threads_latch_ = std::make_shared<std::latch>(0);
    std::condition_variable_any m_workCondition;                                               // 定时用条件变量，用其超时特性来定时，在定时的过程中也能随时唤醒
    std::mutex tasks_mutex_;                                                                   // 定时任务锁
    std::map<uint64_t, std::shared_ptr<STimerParaInter>> tasks_time_id_;                       // 作为索引的定时任务集合 key为定时任务ID
    std::map<uint64_t, std::map<uint64_t, std::shared_ptr<STimerParaInter>>> tasks_time_time_; // 定时任务集合 第一层key为执行时间，第二层Key为定时任务ID
};

} // namespace time
} // namespace jaf