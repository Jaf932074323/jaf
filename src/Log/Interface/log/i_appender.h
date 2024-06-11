#pragma once
#include "define_log_export.h"
#include "i_event.h"
#include <memory>

namespace jaf
{
namespace log
{

// ��־�����
class API_LOG_EXPORT IAppender
{
public:
    virtual ~IAppender(){};

public:
    // ������־�¼�
    virtual void OnLogEvent(const IEvent& log_event) = 0;
};

} // namespace log
} // namespace jaf