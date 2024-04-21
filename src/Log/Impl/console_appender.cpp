#include "console_appender.h"
#include <iostream>
#include "filter.h"
#include "log_format.h"

namespace jaf
{
namespace log
{

struct ConsoleAppender::Impl
{
	std::shared_ptr< IFilter> filter_; // ������
	std::shared_ptr < IFormat> format_; // ��־��ʽ��
};

ConsoleAppender::ConsoleAppender(std::shared_ptr<IFormat> format, std::shared_ptr<IFilter> filter)
{
	m_impl = new Impl
	{
		.filter_ = filter == nullptr ? std::make_shared<Filter>(LOG_LEVEL_INFO) : filter,
		.format_ = format == nullptr ? std::make_shared<LogFormat>() : format
	};
}

ConsoleAppender::~ConsoleAppender() 
{
	delete m_impl;
}

void ConsoleAppender::OnLogEvent(const IEvent& log_event)
{
	if (!m_impl->filter_->Filtration(log_event))
	{
		return;
	}

	std::cout << m_impl->format_->EventToString(log_event) << std::endl;
}

}
}