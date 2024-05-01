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

// ��ʱ��
class Timer : public ITimer
{
public:
    // min_levelͨ���������־�ȼ�
    Timer(std::shared_ptr<IThreadPool> thread_pool = nullptr, std::shared_ptr<IGetTime> get_time = nullptr);
    virtual ~Timer();

public:
    // ����һ����ʱ����
    // rTask ��ʱ������Ϣ
    // ���ض�ʱ����Id������0ʱ��ʾ��Ӷ�ʱ����ʧ��
    virtual uint64_t StartTask(const STimerPara& task) override;
    // ������ж�ʱ����
    virtual void Clear() override;
    // ֹͣһ����ʱ����
    // nTimeId Ҫ�Ƴ��Ķ�ʱ�����Id
    virtual void StopTask(uint64_t task_id) override;

public:
    // ������ʱ
    void Start();
    // ֹͣ��ʱ
    void Stop();

    // ��ʱ�������߳�ִ�к���
    virtual void Work();

    // �ڲ�ʹ�õĶ�ʱ����
    struct STimerParaInter
    {
        STimerPara m_timerTask;
        uint64_t time    = 0; // ִ��ʱ��
        uint64_t task_id = 0; // ��ʱ����ID
    };

private:
    // ִ�дﵽʱ�������
    virtual void GainNeedExecuteTasks(std::list<std::map<uint64_t, std::shared_ptr<STimerParaInter>>>& need_execute_tasks);
    virtual void ExecuteTasks(std::list<std::map<uint64_t, std::shared_ptr<STimerParaInter>>>& need_execute_tasks);

private:
    std::shared_ptr<IThreadPool> thread_pool_;
    std::shared_ptr<IGetTime> get_time_;

    std::atomic<bool> run_flag_ = false; // �����߳����б�־


    uint64_t next_task_id_ = 1; // ��һ����ʱ����ID

    std::atomic<uint64_t> lead_time_ = 5; // ִ���������ǰ����ÿ�����������ǰlead_time_����ִ��

    std::shared_ptr<std::latch> work_threads_latch_ = std::make_shared<std::latch>(0);
    std::condition_variable_any m_workCondition;                                               // ��ʱ���������������䳬ʱ��������ʱ���ڶ�ʱ�Ĺ�����Ҳ����ʱ����
    std::mutex tasks_mutex_;                                                                   // ��ʱ������
    std::map<uint64_t, std::shared_ptr<STimerParaInter>> tasks_time_id_;                       // ��Ϊ�����Ķ�ʱ���񼯺� keyΪ��ʱ����ID
    std::map<uint64_t, std::map<uint64_t, std::shared_ptr<STimerParaInter>>> tasks_time_time_; // ��ʱ���񼯺� ��һ��keyΪִ��ʱ�䣬�ڶ���KeyΪ��ʱ����ID
};

} // namespace time
} // namespace jaf