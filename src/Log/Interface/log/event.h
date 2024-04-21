#pragma once
#include <memory>
#include <thread>

namespace jaf
{
namespace log
{

constexpr uint32_t LOG_LEVEL_FATAL = 0; // 致命错误
constexpr uint32_t LOG_LEVEL_ERROR = 1; // 错误
constexpr uint32_t LOG_LEVEL_WARNING = 2; // 警告
constexpr uint32_t LOG_LEVEL_INFO = 3; // 信息
constexpr uint32_t LOG_LEVEL_DEBUG = 4; // 调试信息
constexpr uint32_t LOG_LEVEL_TRANCE = 5; // 追踪信息

// 日志事件
struct Event
{
	uint32_t level_;
	uint32_t group_number_;
	std::thread::id thread_id_;
	std::chrono::system_clock::time_point time_;
	std::string file_name_;
	uint32_t line_;
	std::string fun_name_;
	std::string info_;
};

}
}