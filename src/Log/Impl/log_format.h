#pragma once
#include "define_log_export.h"
#include "interface/log/i_format.h"
#include <memory>

namespace jaf
{
namespace log
{

// 控制台日志输出器
class API_LOG_EXPORT LogFormat : public IFormat
{
public:
    LogFormat();
    virtual ~LogFormat() {}

public:
    // 日志事件转字符串
    virtual std::string EventToString(const IEvent& log_event) override;

protected:
};

} // namespace log
} // namespace jaf