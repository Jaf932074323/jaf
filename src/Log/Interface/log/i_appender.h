#pragma once
#include <memory>
#include "event.h"

namespace jaf
{
namespace log
{

// 日志输出器
class IAppender
{
public:
	virtual ~IAppender() {};

public:
	// 处理日志事件
	virtual void OnLogEvent(const Event& log_event) = 0;
};

}
}