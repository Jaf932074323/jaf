#pragma once
#include <memory>
#include "define_log_export.h"
#include "Interface/log/i_filter.h"
#include "Interface/log/i_format.h"

namespace jaf
{
namespace log
{

// 控制台日志输出器
class API_LOG_EXPORT ConsoleAppender:public IAppender
{
public:
	ConsoleAppender(std::shared_ptr< IFormat> format = nullptr, std::shared_ptr< IFilter> filter = nullptr);
	virtual ~ConsoleAppender();

public:
	// 处理日志事件
	virtual void OnLogEvent(const IEvent& log_event) override;

protected:
	struct Impl;
	Impl* m_impl = nullptr;
};

}
}