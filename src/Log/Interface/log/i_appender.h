#pragma once
#include <memory>
#include "define_log_export.h"
#include "i_event.h"

namespace jaf
{
namespace log
{

// 日志输出器
class API_LOG_EXPORT IAppender
{
public:
	virtual ~IAppender() {};

public:
	// 处理日志事件
	virtual void OnLogEvent(const IEvent& log_event) = 0;
};

}
}