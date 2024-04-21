#pragma once
#include <memory>
#include <list>
#include "define_log_export.h"
#include "Interface/log/i_logger.h"

namespace jaf
{
namespace log
{

// ����־ �����������־
class API_LOG_EXPORT EmptyLogger:public ILogger
{
public:
	// min_levelͨ���������־�ȼ�
	EmptyLogger(){}
	virtual ~EmptyLogger(){}

public:
	// ������־�¼�
	virtual void OnLogEvent(const IEvent& log_event) override
	{
	}
};

}
}