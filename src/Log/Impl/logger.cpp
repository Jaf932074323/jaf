#include "logger.h"

namespace jaf
{
namespace log
{

Logger::Logger(std::shared_ptr<IAppender> appender)
	: appender_(appender)
{
}

void Logger::OnLogEvent(const Event& log_event)
{
	appender_->OnLogEvent(log_event);
}

}
}