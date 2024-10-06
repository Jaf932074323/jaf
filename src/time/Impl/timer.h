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
// 2024-8-19 ������
#include "interface/i_get_time.h"
#include "interface/i_timer.h"
#include "util/red_black_tree.h"
#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <mutex>

namespace jaf
{
namespace time
{

// ��ʱ��
// ʹ���Զ��������洢ÿ����ʱ����
class Timer : public ITimer
{
public:
    Timer(std::shared_ptr<IGetTime> get_time = nullptr);
    virtual ~Timer();

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
    // ����һ����ʱ����
    // task ��ʱ������Ϣ���ڶ�ʱ�������֮ǰ����Ҫ��֤task��Ч�������ڶ�ʱ�ص�ʱ�����ܻ���Ϊtask��Ч������
    // �����Ƿ�ɹ�
    // ��StartTask�ɹ���para�еĶ�ʱִ�к���һ��Ҫִ��һ�Σ���ֻ��ִ��һ��
    virtual bool StartTask(STimerTask* task);
    // ֹͣһ����ʱ����
    // task ��ʱ������Ϣ
    virtual void StopTask(STimerTask* task);

private:
    struct STimerParaInter;
    using TimerTree = jaf::RedBlackTree<uint64_t, std::shared_ptr<STimerParaInter>>;

    // �ڲ�ʹ�õĶ�ʱ�������
    struct STimerParaInter
    {
        STimerTask* timer_task = nullptr;
        std::function<void(ETimerResultType result_type, STimerTask* task)> fun; // ��ʱִ�к���
        uint64_t time;                                                           // ��ʱ�����ִ��ʱ���
        Timer* timer_               = nullptr;                                   // ������ʱ��
        bool timing_flag_           = false;                                     // ��ǰ�Ƿ������ڶ�ʱ״̬
        TimerTree::Node* timer_node = nullptr;                                   // ��ʱ����������ڵ�
    };

public:
    // ��ʱ�������߳�ִ�к���
    virtual void Work();

private:
    // ִ�дﵽʱ�������
    virtual void GainNeedExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks, std::list<std::shared_ptr<STimerParaInter>>& need_stop_tasks);
    virtual void ExecuteTasks(std::list<std::shared_ptr<STimerParaInter>>& need_execute_tasks, ETimerResultType result_type);

private:
    std::shared_ptr<IGetTime> get_time_;

    std::atomic<bool> run_flag_ = true; // �����߳����б�־

    uint64_t next_task_id_ = 1; // ��һ����ʱ����ID

    std::atomic<uint64_t> lead_time_ = 5; // ִ���������ǰ����ÿ�����������ǰlead_time_����ִ��

    std::thread run_thread_;
    std::condition_variable_any m_workCondition;             // ��ʱ���������������䳬ʱ��������ʱ���ڶ�ʱ�Ĺ�����Ҳ����ʱ����
    std::mutex tasks_mutex_;                                 // ��ʱ������
    TimerTree tasks_time_;                                   // ��ʱ���񼯺� keyΪ��ʱ�����ִ��ʱ���
    std::list<std::shared_ptr<STimerParaInter>> tasks_stop_; // ����ֹͣ�Ķ�ʱ���񼯺�
};

} // namespace time
} // namespace jaf