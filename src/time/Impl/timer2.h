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
// 2024-6-16 ������
#include "interface/i_get_time.h"
#include "interface/i_timer.h"
#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

namespace jaf
{
namespace time
{

// ��ʱ��
// ʹ��std::map�洢ÿ����ʱ����
class Timer2 : public ITimer
{
public:
    Timer2(std::shared_ptr<IGetTime> get_time = nullptr);
    virtual ~Timer2();
    Timer2(const Timer2&)            = delete; // ������Ϊ���ӣ�Ŀǰ�Ƚ�ֹ
    Timer2& operator=(const Timer2&) = delete; // ������Ϊ���ӣ�Ŀǰ�Ƚ�ֹ

public:
    // ��ȡִ���������ǰ����ÿ�����������ǰִ�е�ʱ��(����)
    uint64_t GetLeadTime()
    {
        return lead_time_;
    }
    // ����ִ���������ǰ����ÿ�����������ǰִ�е�ʱ��(����)
    void SetLeadTime(uint64_t lead_time)
    {
        lead_time_ = lead_time;
    }

public:
    // ������ʱ
    virtual void Start() override;
    // ֹͣ��ʱ
    virtual void Stop() override;

    // ����һ����ʱ����
    // task ��ʱ������Ϣ���ڶ�ʱ�������֮ǰ����Ҫ��֤task��Ч�������ڶ�ʱ�ص�ʱ�����ܻ���Ϊtask��Ч������
    // �����Ƿ�ɹ�
    // ��StartTask�ɹ���para�еĶ�ʱִ�к���һ��Ҫִ��һ�Σ���ֻ��ִ��һ��
    virtual bool StartTask(STimerTask* task);
    // ֹͣһ����ʱ����
    // task ��ʱ������Ϣ
    virtual void StopTask(STimerTask* task);

private:
    struct STimerKey
    {
        uint64_t time    = 0; // ִ��ʱ��
        uint64_t task_id = 0; // ��ʱ����ID

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

    // �ڲ�ʹ�õĶ�ʱ����
    struct STimerParaInter
    {
        STimerTask* timer_task = nullptr;
        std::function<void(ETimerResultType result_type, STimerTask* task)> fun; // ��ʱִ�к���
        STimerKey key;
        Timer2* timer_ = nullptr; // ������ʱ��
    };

    // ֹͣһ����ʱ����
    // task_id Ҫ�Ƴ��Ķ�ʱ�����Id
    virtual void StopTask(STimerKey key);

public:
    // ��ʱ�������߳�ִ�к���
    virtual void Work();

private:
    // ִ�дﵽʱ�������
    virtual void GainNeedExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks, std::list<std::shared_ptr<STimerParaInter>>& need_stop_tasks);
    virtual void ExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks, ETimerResultType result_type);

private:
    std::shared_ptr<IGetTime> get_time_;

    std::atomic<bool> run_flag_ = false; // �����߳����б�־

    uint64_t next_task_id_ = 1; // ��һ����ʱ����ID

    std::atomic<uint64_t> lead_time_ = 5; // ִ���������ǰ����ÿ�����������ǰlead_time_����ִ��
    
    std::thread run_thread_;
    std::condition_variable_any m_workCondition;                       // ��ʱ���������������䳬ʱ��������ʱ���ڶ�ʱ�Ĺ�����Ҳ����ʱ����
    std::mutex tasks_mutex_;                                           // ��ʱ������
    std::map<STimerKey, std::shared_ptr<STimerParaInter>> tasks_time_; // ��ʱ���񼯺�
    std::list<std::shared_ptr<STimerParaInter>> tasks_stop_; // ����ֹͣ�Ķ�ʱ���񼯺�
};

} // namespace time
} // namespace jaf