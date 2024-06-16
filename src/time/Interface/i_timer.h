#pragma once
#include <functional>
#include <memory>

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
struct STimerPara
{
    std::function<void(ETimerResultType result_type)> fun; // ��ʱִ�к���
    uint64_t interval = 1000;                                                // ��ʱʱ���������룩
};

class ITimerTask
{
public:
    virtual ~ITimerTask(){};

public:
    // ������ʱ����
    virtual void Stop() = 0;
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
    // rTask ��ʱ������Ϣ
    // ���ض�ʱ���񣬷���nullptrʱ��ʾ��Ӷ�ʱ����ʧ��
    // ��StartTask�ɹ���para�еĶ�ʱִ�к���һ��Ҫִ��һ�Σ���ֻ��ִ��һ��
    virtual std::shared_ptr<ITimerTask> StartTask(const STimerPara& para) = 0;
    // ������ж�ʱ����
    virtual void Clear() = 0;
};

} // namespace time
} // namespace jaf