#pragma once
#include "Interface/log/i_event.h"

namespace jaf
{
namespace log
{

// 日志事件
class Event : public IEvent
{
public:
    Event(
        uint32_t level, std::string file_name, uint32_t line, std::string fun_name, uint64_t time, uint64_t thread_id, uint32_t group_number);
    ~Event();

    virtual uint32_t Level() const override;
    virtual uint32_t GroupNumber() const override;
    virtual uint64_t ThreadId() const override;
    virtual uint64_t Time() const override;
    virtual const char* FileName() const override;
    virtual uint32_t Line() const override;
    virtual const char* Info() const override;

public:
    Event& operator<<(const std::string_view& arg)
    {
        info_ += arg;
        return *this;
    }

    Event& operator<<(const char* arg)
    {
        info_ += std::string_view(arg);
        return *this;
    }

private:
    uint32_t level_        = LOG_LEVEL_INFO;
    uint32_t group_number_ = 0;
    uint64_t thread_id_    = 0;
    uint64_t time_         = 0;
    std::string file_name_;
    uint32_t line_ = 0;
    std::string fun_name_;
    std::string info_;
};

} // namespace log
} // namespace jaf