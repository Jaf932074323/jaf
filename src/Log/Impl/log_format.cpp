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

std::string LogFormat::EventToString(const Event& log_event)
{
	std::string str_log = std::format("[{:%Y-%m-%d %H:%M:%S}][{}][{:16X}]:{}"
		, floor<std::chrono::milliseconds>(log_event.time_)
		, jaf::log::LogLevelManage::Get(log_event.level_)
		, std::hash<std::thread::id>()(log_event.thread_id_) // TODO:这里有点坑，正常手段难以获取到线程的整数值
		, log_event.info_
	);
	return str_log;
}

}
}