#include "log_format.h"
#include <format>
#include <chrono>
#include "Interface/log/event_level_str.h"

namespace jaf
{
namespace log
{

LogFormat::LogFormat()
{
}

std::string LogFormat::EventToString(const IEvent& log_event)
{
	std::string str_log = std::format("[{:%Y-%m-%d %H:%M:%S}][{}][{:6X}]:{}"
		, std::chrono::system_clock::time_point{ std::chrono::milliseconds{log_event.Time()} }
		, jaf::log::LogLevelManage::Get(log_event.Level())
		, log_event.ThreadId()
		, log_event.Info()
	);

	return str_log;
}

}
}