#pragma once
#include <functional>
#include <memory>

namespace jaf
{
namespace time
{

enum class ETimerResultType
{
    TRT_SUCCESS    = 0, // 定时成功
    TRT_TASK_STOP  = 1, // 这个定时任务停止
    TRT_TIMER_STOP = 2, // 定时器停止
};

// 定时参数
struct STimerPara
{
    std::function<void(ETimerResultType result_type, uint64_t task_id)> fun; // 定时执行函数
    uint64_t interval = 1000;                                                // 定时时间间隔（毫秒）
};

// 定时器接口
class ITimer
{
public:
    virtual ~ITimer(){};

public:
    // 启动定时器
    virtual  void Start() = 0;
    // 停止定时器
    virtual void Stop() = 0;
    // 启动一个定时任务
    // rTask 定时任务信息
    // 返回定时任务ID，返回0时表示添加定时任务失败
    // 当StartTask成功后，para中的定时执行函数一定要执行一次，且只会执行一次
    virtual uint64_t StartTask(const STimerPara& para) = 0;
    // 清除所有定时任务
    virtual void Clear() = 0;
    // 停止一个定时任务
    // nTimeId 要移除的定时任务的Id
    virtual void StopTask(uint64_t task_id) = 0;
};

} // namespace time
} // namespace jaf