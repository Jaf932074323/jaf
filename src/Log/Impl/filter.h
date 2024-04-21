#pragma once
#include <memory>
#include "Interface/log/i_filter.h"

namespace jaf
{
namespace log
{

// 控制台日志输出器
class Filter:public IFilter
{
public:
	// min_level通过的最低日志等级
	Filter(uint32_t min_level);
	virtual ~Filter(){}

public:
	// 筛选日志 true通过筛选，false不通过筛选
	virtual bool Filtration(const Event& log_event) override;

protected:
	uint32_t min_level_; // 通过的最低日志等级
};

}
}