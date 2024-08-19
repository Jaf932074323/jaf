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
#include <functional>

namespace jaf
{
namespace time
{

enum class ETimerResultType
{
    TRT_SUCCESS    = 0, // ��ʱ�ɹ�
    TRT_TASK_STOP  = 1, // �����ʱ����ֹͣ
    TRT_TIMER_STOP = 2, // ��ʱ��ֹͣ
};

// ��ʱ����
struct STimerTask
{
    std::function<void(ETimerResultType result_type, STimerTask* task)> fun; // ��ʱִ�к���
    uint64_t interval = 1000;                              // ��ʱʱ���������룩
    void* timer_data_ = nullptr;                           // ��ʱ��ά�������ݣ��ɶ�ʱ����¼��������Ҫ�����ݣ�ֻ�ܶ�ʱ���޸�
};

// ��ʱ���ӿ�
class ITimer
{
public:
    virtual ~ITimer(){};

public:
    // ������ʱ��
    virtual void Start() = 0;
    // ֹͣ��ʱ��
    virtual void Stop() = 0;
    // ����һ����ʱ����
    // task ��ʱ������Ϣ���ڶ�ʱ�������֮ǰ����Ҫ��֤task��Ч�������ڶ�ʱ�ص�ʱ�����ܻ���Ϊtask��Ч������
    // �����Ƿ�ɹ�
    // ��StartTask�ɹ���para�еĶ�ʱִ�к���һ��Ҫִ��һ�Σ���ֻ��ִ��һ��
    virtual bool StartTask(STimerTask* task) = 0;
    // ֹͣһ����ʱ����
    // task ��ʱ������Ϣ
    virtual void StopTask(STimerTask* task) = 0;
};

} // namespace time
} // namespace jaf