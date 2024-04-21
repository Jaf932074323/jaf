#pragma once
#include <memory>
#include "Interface/log/i_appender.h"
#include "Interface/log/i_filter.h"
#include "Interface/log/i_format.h"

namespace jaf
{
namespace log
{

// 控制台日志输出器
class ConsoleAppender:public IAppender
{
public:
	ConsoleAppender(std::shared_ptr < IFormat> format = nullptr, std::shared_ptr< IFilter> filter = nullptr);
	virtual ~ConsoleAppender(){}

public:
	// 处理日志事件
	virtual void OnLogEvent(const Event& log_event) override;

protected:
	std::shared_ptr< IFilter> filter_; // 过滤器
	std::shared_ptr < IFormat> format_; // 日志格式化
};

}
}