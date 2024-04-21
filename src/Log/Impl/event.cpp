#include "event.h"

namespace jaf
{
namespace log
{
Event::Event(
	uint32_t level
	, std::string file_name
	, uint32_t line
	, std::string fun_name
	, uint64_t time
	, uint64_t thread_id
	, uint32_t group_number)
	:level_(level)
	, group_number_(group_number)
	, thread_id_(thread_id)
	, time_(time)
	, file_name_(file_name)
	, line_(line)
	, fun_name_(fun_name)
{
}

Event::~Event() {}

uint32_t Event::Level() const
{
	return level_;
}

uint32_t Event::GroupNumber() const
{
	return group_number_;
}

uint64_t Event::ThreadId() const
{
	return thread_id_;
}

uint64_t Event::Time() const
{
	return time_;
}

const char* Event::FileName() const
{
	return file_name_.c_str();
}

uint32_t Event::Line() const
{
	return line_;
}

const char* Event::Info() const
{
	return info_.c_str();
}

}
}