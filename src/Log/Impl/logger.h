#pragma once
#include <memory>
#include <list>
#include "Interface/log/i_logger.h"

namespace jaf
{
namespace log
{

// 日志
class Logger:public ILogger
{
public:
	// min_level通过的最低日志等级
	Logger(std::shared_ptr<IAppender> appender);
	virtual ~Logger(){}

public:
	// 处理日志事件
	virtual void OnLogEvent(const Event& log_event) override;

protected:
	std::shared_ptr<IAppender> appender_; // 日志输出器
};

}
}