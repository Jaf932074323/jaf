#pragma once
#include "define_log_export.h"
#include "interface/log/i_logger.h"
#include <list>
#include <memory>

namespace jaf
{
namespace log
{

// ����־ �����������־
class API_LOG_EXPORT EmptyLogger : public ILogger
{
public:
    // min_levelͨ���������־�ȼ�
    EmptyLogger() {}
    virtual ~EmptyLogger() {}

public:
    // ������־�¼�
    virtual void OnLogEvent(const IEvent& log_event) override
    {
    }
};

} // namespace log
} // namespace jaf