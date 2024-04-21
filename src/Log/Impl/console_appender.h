#pragma once
#include <memory>
#include "Interface/log/i_appender.h"
#include "Interface/log/i_filter.h"
#include "Interface/log/i_format.h"

namespace jaf
{
namespace log
{

// ����̨��־�����
class ConsoleAppender:public IAppender
{
public:
	ConsoleAppender(std::shared_ptr < IFormat> format = nullptr, std::shared_ptr< IFilter> filter = nullptr);
	virtual ~ConsoleAppender(){}

public:
	// ������־�¼�
	virtual void OnLogEvent(const Event& log_event) override;

protected:
	std::shared_ptr< IFilter> filter_; // ������
	std::shared_ptr < IFormat> format_; // ��־��ʽ��
};

}
}