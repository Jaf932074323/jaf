#pragma once
#include "define_log_export.h"
#include "i_appender.h"
#include "i_event.h"
#include <memory>

namespace jaf
{
namespace log
{

// ������
class API_LOG_EXPORT IFilter
{
public:
    virtual ~IFilter(){};

public:
    // ɸѡ��־ trueͨ��ɸѡ��false��ͨ��ɸѡ
    virtual bool Filtration(const IEvent& log_event) = 0;
};

} // namespace log
} // namespace jaf