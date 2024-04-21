#pragma once
#include <memory>
#include "i_appender.h"
#include "event.h"

namespace jaf
{
namespace log
{

// ��־
class ILogger
{
public:
	virtual ~ILogger() {};

public:
	// ������־�¼�
	virtual void OnLogEvent(const Event& log_event) = 0;
};

}
}