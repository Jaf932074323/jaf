#include "logger.h"

namespace jaf
{
namespace log
{

struct Logger::Impl
{
    std::shared_ptr<IAppender> appender_; // ÈÕÖ¾Êä³öÆ÷
};

Logger::Logger(std::shared_ptr<IAppender> appender)
    : m_impl(new Impl{.appender_ = appender})
{
}

void Logger::OnLogEvent(const IEvent& log_event)
{
    m_impl->appender_->OnLogEvent(log_event);
}

} // namespace log
} // namespace jaf