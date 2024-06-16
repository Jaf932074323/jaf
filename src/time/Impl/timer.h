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


// ��ʱ��
class Timer : public ITimer
{
    class TimerTask : public ITimerTask
    {
    public:
        TimerTask(Timer* timer, uint64_t task_id);
    public:
        uint64_t TaskId();
        // ������ʱ����
        virtual void Stop() override;
    private:
        uint64_t task_id_ = 0; // ��ʱ����ID   
        Timer* timer_ = nullptr;
    };
public:
    // min_levelͨ���������־�ȼ�
    Timer(std::shared_ptr<IThreadPool> thread_pool = nullptr, std::shared_ptr<IGetTime> get_time = nullptr);
    virtual ~Timer();

public:
    // ������ʱ
    virtual void Start() override;
    // ֹͣ��ʱ
    virtual void Stop() override;

    // ����һ����ʱ����
    // rTask ��ʱ������Ϣ
    // ���ض�ʱ���񣬷���nullptrʱ��ʾ��Ӷ�ʱ����ʧ��
    // ��StartTask�ɹ���para�еĶ�ʱִ�к���һ��Ҫִ��һ�Σ���ֻ��ִ��һ��
    virtual std::shared_ptr<ITimerTask> StartTask(const STimerPara& para) override;
    // ������ж�ʱ����
    virtual void Clear() override;
private:
    // ֹͣһ����ʱ����
    // task_id Ҫ�Ƴ��Ķ�ʱ�����Id
    virtual void StopTask(uint64_t task_id);

public:
    // ��ʱ�������߳�ִ�к���
    virtual void Work();

    // �ڲ�ʹ�õĶ�ʱ����
    struct STimerParaInter
    {
        STimerPara m_timerTask;
        uint64_t time = 0; // ִ��ʱ��
        std::shared_ptr<TimerTask> task = nullptr;
    };

private:
    // ִ�дﵽʱ�������
    virtual void GainNeedExecuteTasks(std::list<std::map<uint64_t, std::shared_ptr<STimerParaInter>>>& need_execute_tasks);
    virtual void ExecuteTasks(std::list<std::map<uint64_t, std::shared_ptr<STimerParaInter>>>& need_execute_tasks, ETimerResultType result_type);

private:
    std::shared_ptr<IThreadPool> thread_pool_;
    std::shared_ptr<IGetTime> get_time_;

    std::atomic<bool> run_flag_ = false; // �����߳����б�־


    uint64_t next_task_id_ = 1; // ��һ����ʱ����ID

    std::atomic<uint64_t> lead_time_ = 5; // ִ���������ǰ����ÿ�����������ǰlead_time_����ִ��

    Latch work_threads_latch_{1};
    std::condition_variable_any m_workCondition;                                               // ��ʱ���������������䳬ʱ��������ʱ���ڶ�ʱ�Ĺ�����Ҳ����ʱ����
    std::mutex tasks_mutex_;                                                                   // ��ʱ������
    std::map<uint64_t, std::shared_ptr<STimerParaInter>> tasks_time_id_;                       // ��Ϊ�����Ķ�ʱ���񼯺� keyΪ��ʱ����ID
    std::map<uint64_t, std::map<uint64_t, std::shared_ptr<STimerParaInter>>> tasks_time_time_; // ��ʱ���񼯺� ��һ��keyΪִ��ʱ�䣬�ڶ���KeyΪ��ʱ����ID
};

} // namespace time
} // namespace jaf