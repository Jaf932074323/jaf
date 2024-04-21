#include "create_log_event.h"
#include <format>
#include <assert.h>
#include "manage_logger.h"

namespace jaf
{
namespace log
{

CreateLogEvent::CreateLogEvent(
	uint32_t level
	, std::string file_name
	, uint32_t line
	, std::string fun_name
	, std::shared_ptr<jaf::log::ILogger> logger
	, std::chrono::system_clock::time_point time
	, std::thread::id thread_id
	, uint32_t group_number)
	: logger_(logger)
	, log_event_{ level, group_number, thread_id, time, file_name, line, fun_name}
{
	assert(logger != nullptr);
}

CreateLogEvent::CreateLogEvent(
	uint32_t level
	, std::string file_name
	, uint32_t line
	, std::string fun_name
	, std::chrono::system_clock::time_point time
	, std::thread::id thread_id
	, uint32_t group_number)
	: CreateLogEvent(level, file_name, line, fun_name, jaf::log::CommonLogger<"">::Logger(), time, thread_id, group_number)
{
}

CreateLogEvent::~CreateLogEvent()
{
	logger_->OnLogEvent(log_event_);
}


}
}