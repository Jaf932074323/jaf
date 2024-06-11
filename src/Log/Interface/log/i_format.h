#pragma once
#include "define_log_export.h"
#include "i_event.h"
#include <string>

namespace jaf
{
namespace log
{

// ��־�¼�
class API_LOG_EXPORT IFormat
{
public:
    virtual ~IFormat(){};

public:
    // ��־�¼�ת�ַ���
    virtual std::string EventToString(const IEvent& log_event) = 0;
};

} // namespace log
} // namespace jaf