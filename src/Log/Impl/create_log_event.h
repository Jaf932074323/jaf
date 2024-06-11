#pragma once
#include "define_log_export.h"
#include "interface/log/i_logger.h"
#include <memory>

namespace jaf
{
namespace log
{

// 控制台日志输出器
class API_LOG_EXPORT CreateLogEvent
{
public:
    CreateLogEvent(
        uint32_t level, std::string file_name, uint32_t line, std::string fun_name, std::shared_ptr<jaf::log::ILogger> logger, uint64_t time = GetCurTime(), uint64_t thread_id = GetCurThreadId(), uint32_t group_number = 0);

    CreateLogEvent(
        uint32_t level, std::string file_name, uint32_t line, std::string fun_name, uint64_t time = GetCurTime(), uint64_t thread_id = GetCurThreadId(), uint32_t group_number = 0);

    ~CreateLogEvent();

    CreateLogEvent(const CreateLogEvent&)            = delete;
    CreateLogEvent& operator=(const CreateLogEvent&) = delete;

public:
    CreateLogEvent& operator<<(const std::string_view& arg);
    CreateLogEvent& operator<<(const char* arg);

    static uint64_t GetCurThreadId();
    static uint64_t GetCurTime();

protected:
    struct Impl;
    Impl* m_impl;
};

} // namespace log
} // namespace jaf