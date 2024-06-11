#pragma once
#include "define_log_export.h"
#include "i_event.h"
#include <string>

namespace jaf
{
namespace log
{

// 日志事件
class API_LOG_EXPORT IFormat
{
public:
    virtual ~IFormat(){};

public:
    // 日志事件转字符串
    virtual std::string EventToString(const IEvent& log_event) = 0;
};

} // namespace log
} // namespace jaf