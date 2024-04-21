#pragma once
#include <memory>
#include "i_appender.h"
#include "event.h"

namespace jaf
{
namespace log
{

// 日志
class ILogger
{
public:
	virtual ~ILogger() {};

public:
	// 处理日志事件
	virtual void OnLogEvent(const Event& log_event) = 0;
};

}
}