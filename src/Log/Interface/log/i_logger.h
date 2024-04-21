#pragma once
#include <memory>
#include "define_log_export.h"
#include "i_appender.h"
#include "i_event.h"

namespace jaf
{
namespace log
{

// ��־
class API_LOG_EXPORT ILogger
{
public:
	virtual ~ILogger() {};

public:
	// ������־�¼�
	virtual void OnLogEvent(const IEvent& log_event) = 0;
};

}
}