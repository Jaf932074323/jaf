#pragma once
#include <memory>
#include <thread>

namespace jaf
{
namespace log
{

constexpr uint32_t LOG_LEVEL_FATAL = 0; // ��������
constexpr uint32_t LOG_LEVEL_ERROR = 1; // ����
constexpr uint32_t LOG_LEVEL_WARNING = 2; // ����
constexpr uint32_t LOG_LEVEL_INFO = 3; // ��Ϣ
constexpr uint32_t LOG_LEVEL_DEBUG = 4; // ������Ϣ
constexpr uint32_t LOG_LEVEL_TRANCE = 5; // ׷����Ϣ

// ��־�¼�
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