#pragma once
#include <memory>
#include <list>
#include "Interface/log/i_logger.h"

namespace jaf
{
namespace log
{

// ����־ �����������־
class EmptyLogger:public ILogger
{
public:
	// min_levelͨ���������־�ȼ�
	EmptyLogger(){}
	virtual ~EmptyLogger(){}

public:
	// ������־�¼�
	virtual void OnLogEvent(const Event& log_event) override
	{
	}
};

}
}