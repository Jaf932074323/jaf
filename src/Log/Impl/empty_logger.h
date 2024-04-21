#pragma once
#include <memory>
#include <list>
#include "define_log_export.h"
#include "Interface/log/i_logger.h"

namespace jaf
{
namespace log
{

// 空日志 会忽略所有日志
class API_LOG_EXPORT EmptyLogger:public ILogger
{
public:
	// min_level通过的最低日志等级
	EmptyLogger(){}
	virtual ~EmptyLogger(){}

public:
	// 处理日志事件
	virtual void OnLogEvent(const IEvent& log_event) override
	{
	}
};

}
}