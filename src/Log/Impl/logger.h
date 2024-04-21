#pragma once
#include <memory>
#include <list>
#include "Interface/log/i_logger.h"

namespace jaf
{
namespace log
{

// ��־
class Logger:public ILogger
{
public:
	// min_levelͨ���������־�ȼ�
	Logger(std::shared_ptr<IAppender> appender);
	virtual ~Logger(){}

public:
	// ������־�¼�
	virtual void OnLogEvent(const Event& log_event) override;

protected:
	std::shared_ptr<IAppender> appender_; // ��־�����
};

}
}