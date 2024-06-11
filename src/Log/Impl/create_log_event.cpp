#include "create_log_event.h"
#include "common_logger.h"
#include "impl/event.h"
#include <assert.h>
#include <format>
#include <sstream>

namespace jaf
{
namespace log
{

struct CreateLogEvent::Impl
{
    std::shared_ptr<jaf::log::ILogger> logger_;
    Event log_event_;
};

CreateLogEvent::CreateLogEvent(
    uint32_t level, std::string file_name, uint32_t line, std::string fun_name, std::shared_ptr<jaf::log::ILogger> logger, uint64_t time, uint64_t thread_id, uint32_t group_number)
    : m_impl(new Impl{
        .logger_    = logger,
        .log_event_ = Event(level, file_name, line, fun_name, time, thread_id, group_number)})
{
    assert(logger != nullptr);
}

CreateLogEvent::CreateLogEvent(
    uint32_t level, std::string file_name, uint32_t line, std::string fun_name, uint64_t time, uint64_t thread_id, uint32_t group_number)
    : CreateLogEvent(level, file_name, line, fun_name, jaf::log::CommonLogger::DefaultLogger(), time, thread_id, group_number)
{
}

CreateLogEvent::~CreateLogEvent()
{
    m_impl->logger_->OnLogEvent(m_impl->log_event_);
    delete m_impl;
}

CreateLogEvent& CreateLogEvent::operator<<(const std::string_view& arg)
{
    m_impl->log_event_ << arg;
    return *this;
}

CreateLogEvent& CreateLogEvent::operator<<(const char* arg)
{
    m_impl->log_event_ << arg;
    return *this;
}

uint64_t CreateLogEvent::GetCurThreadId()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    uint64_t id = (uint64_t) std::stoull(ss.str());
    return id;
}

uint64_t CreateLogEvent::GetCurTime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


} // namespace log
} // namespace jaf