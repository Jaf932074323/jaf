#include "console_appender.h"
#include <iostream>
#include "filter.h"
#include "log_format.h"

namespace jaf
{
namespace log
{

ConsoleAppender::ConsoleAppender(std::shared_ptr<IFormat> format, std::shared_ptr<IFilter> filter)
	: format_(format == nullptr ? std::make_shared<LogFormat>() : format)
	, filter_(filter == nullptr ? std::make_shared<Filter>(LOG_LEVEL_INFO) : filter)
{
}

void ConsoleAppender::OnLogEvent(const Event& log_event)
{
	if (!filter_->Filtration(log_event))
	{
		return;
	}

	std::cout << format_->EventToString(log_event) << std::endl;
}

}
}