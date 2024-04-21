#pragma once
#include <string>
#include "event.h"

namespace jaf
{
namespace log
{

// 日志事件
class IFormat
{
public:
	virtual ~IFormat() {};

public:
	// 日志事件转字符串
	virtual std::string EventToString(const Event& log_event) = 0;
};

}
}