#pragma once
#include "define_log_export.h"
#include "i_appender.h"
#include "i_event.h"
#include <memory>

namespace jaf
{
namespace log
{

// 过滤器
class API_LOG_EXPORT IFilter
{
public:
    virtual ~IFilter(){};

public:
    // 筛选日志 true通过筛选，false不通过筛选
    virtual bool Filtration(const IEvent& log_event) = 0;
};

} // namespace log
} // namespace jaf