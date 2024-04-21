#pragma once
#include <memory>
#include <list>
#include "Interface/log/i_logger.h"

namespace jaf
{
namespace log
{

// 空日志 会忽略所有日志
class EmptyLogger:public ILogger
{
public:
	// min_level通过的最低日志等级
	EmptyLogger(){}
	virtual ~EmptyLogger(){}

public:
	// 处理日志事件
	virtual void OnLogEvent(const Event& log_event) override
	{
	}
};

}
}