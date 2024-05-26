#pragma once
#include <functional>
#include <memory>

namespace jaf
{
namespace time
{

enum class TimerResultType
{
    TRT_SUCCESS   = 0, // 定时成功
    TRT_TASK_STOP = 1, // 这个定时任务停止
    TRT_TIMER_STOP = 2, // 定时器停止
};

// 定时参数
struct STimerPara
{
    std::function<void(TimerResultType result_type)> fun; // 定时执行函数
    uint64_t interval = 1000;      // 定时时间间隔（毫秒）
};

// 定时器接口
class ITimer
{
public:
    virtual ~ITimer(){};

public:
    // 启动一个定时任务
    // rTask 定时任务信息
    // 返回定时任务ID，返回0时表示添加定时任务失败
    virtual uint64_t StartTask(const STimerPara& para) = 0;
    // 清除所有定时任务
    virtual void Clear() = 0;
    // 停止一个定时任务
    // nTimeId 要移除的定时任务的Id
    virtual void StopTask(uint64_t task_id) = 0;
};

} // namespace time
} // namespace jaf