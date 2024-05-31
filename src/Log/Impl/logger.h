#pragma once
#include <memory>
#include "define_log_export.h"
#include "interface/log/i_logger.h"

namespace jaf
{
namespace log
{

// ��־
class API_LOG_EXPORT Logger:public ILogger
{
public:
	// min_levelͨ���������־�ȼ�
	Logger(std::shared_ptr<IAppender> appender);
	virtual ~Logger(){}

public:
	// ������־�¼�
	virtual void OnLogEvent(const IEvent& log_event) override;

protected:
	struct Impl;
	Impl* m_impl = nullptr;
};

}
}