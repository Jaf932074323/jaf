
#pragma once
#include <functional>
#include <memory>

namespace jaf
{
namespace time
{

// 定时任务参数
struct STimerTask
{
	std::function<void(void)> fun; // 定时执行函数
	uint64_t interval = 1000; // 定时时间间隔（毫秒）
};

// 过滤器
class ITimer
{
public:
	virtual ~ITimer()
	{};

public:
	// 添加定时任务
	// rTask 定时任务信息
	// 返回定时任务Id，返回0时表示添加定时任务失败
	virtual uint64_t AddTask(const STimerTask& task) = 0;
	// 清除所有定时任务
	virtual void Clear() = 0;
	// 移除一个定时任务
	// nTimeId 要移除的定时任务的Id
	virtual void Remove(uint64_t task_id) = 0;
};

}
}