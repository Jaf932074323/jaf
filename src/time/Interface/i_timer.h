#pragma once
#include <functional>
#include <memory>

namespace jaf
{
namespace time
{

// ��ʱ����
struct STimerPara
{
    std::function<void(void)> fun; // ��ʱִ�к���
    uint64_t interval = 1000;      // ��ʱʱ���������룩
};

// ��ʱ���ӿ�
class ITimer
{
public:
    virtual ~ITimer(){};

public:
    // ����һ����ʱ����
    // rTask ��ʱ������Ϣ
    // ���ض�ʱ����ID������0ʱ��ʾ��Ӷ�ʱ����ʧ��
    virtual uint64_t StartTask(const STimerPara& para) = 0;
    // ������ж�ʱ����
    virtual void Clear() = 0;
    // ֹͣһ����ʱ����
    // nTimeId Ҫ�Ƴ��Ķ�ʱ�����Id
    virtual void StopTask(uint64_t task_id) = 0;
};

} // namespace time
} // namespace jaf