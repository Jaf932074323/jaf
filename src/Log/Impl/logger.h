#pragma once
#include "define_log_export.h"
#include "interface/log/i_logger.h"
#include <memory>

namespace jaf
{
namespace log
{

// 日志
class API_LOG_EXPORT Logger : public ILogger
{
public:
    // min_level通过的最低日志等级
    Logger(std::shared_ptr<IAppender> appender);
    virtual ~Logger() {}

public:
    // 处理日志事件
    virtual void OnLogEvent(const IEvent& log_event) override;

protected:
    struct Impl;
    Impl* m_impl = nullptr;
};

} // namespace log
} // namespace jaf