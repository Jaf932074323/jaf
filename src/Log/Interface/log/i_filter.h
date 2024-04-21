#pragma once
#include <memory>
#include "i_appender.h"
#include "event.h"

namespace jaf
{
namespace log
{

// 过滤器
class IFilter
{
public:
	virtual ~IFilter() {};

public:
	// 筛选日志 true通过筛选，false不通过筛选
	virtual bool Filtration(const Event& log_event) = 0;
};

}
}