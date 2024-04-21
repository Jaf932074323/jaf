#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include "log_head.h"

int main()
{
	std::shared_ptr<jaf::log::ConsoleAppender> appender = std::make_shared<jaf::log::ConsoleAppender>();
	std::shared_ptr<jaf::log::ILogger> logger = std::make_shared<jaf::log::Logger>(appender);
	jaf::log::CommonLogger::SetDefaultLogger(logger);

	for (size_t i = 0; i < 100; ++i)
	{
		LOG_WARNING() << "123";
	}

	return 0;
}