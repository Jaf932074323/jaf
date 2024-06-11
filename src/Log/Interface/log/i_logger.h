#pragma once
#include "define_log_export.h"
#include "i_appender.h"
#include "i_event.h"
#include <memory>

namespace jaf
{
namespace log
{

// 日志
class API_LOG_EXPORT ILogger
{
public:
    virtual ~ILogger(){};

public:
    // 处理日志事件
    virtual void OnLogEvent(const IEvent& log_event) = 0;
};

} // namespace log
} // namespace jaf