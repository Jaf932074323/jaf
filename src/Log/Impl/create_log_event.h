#pragma once
#include <memory>
#include "Interface/log/i_logger.h"
#include "Interface/log/event.h"

namespace jaf
{
namespace log
{

// 控制台日志输出器
class CreateLogEvent
{
public:
	CreateLogEvent(
		uint32_t level
		, std::string file_name
		, uint32_t line
		, std::string fun_name
		, std::shared_ptr<jaf::log::ILogger> logger
		, std::chrono::system_clock::time_point time = std::chrono::system_clock::now()
		, std::thread::id thread_id = std::this_thread::get_id()
		, uint32_t group_number = 0
	);

	CreateLogEvent(
		uint32_t level
		, std::string file_name
		, uint32_t line
		, std::string fun_name
		, std::chrono::system_clock::time_point time = std::chrono::system_clock::now()
		, std::thread::id thread_id = std::this_thread::get_id()
		, uint32_t group_number = 0
	);

	~CreateLogEvent();

	CreateLogEvent(const CreateLogEvent&) = delete;
	CreateLogEvent& operator=(const CreateLogEvent&) = delete;
public:
	CreateLogEvent& operator<<(const std::string_view& arg)
	{
		log_event_.info_ += arg;
		return *this;
	}

	CreateLogEvent& operator<<(const char* arg)
	{
		log_event_.info_ += std::string_view(arg);
		return *this;
	}

protected:
	std::shared_ptr<jaf::log::ILogger> logger_;
	Event log_event_;
};

}
}