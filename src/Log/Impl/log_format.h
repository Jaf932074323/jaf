#pragma once
#include <memory>
#include "Interface/log/i_format.h"

namespace jaf
{
namespace log
{

// 控制台日志输出器
class LogFormat:public IFormat
{
public:
	LogFormat();
	virtual ~LogFormat(){}

public:
	// 日志事件转字符串
	virtual std::string EventToString(const Event& log_event) override;

protected:

};

}
}