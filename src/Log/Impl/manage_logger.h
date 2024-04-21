#pragma once
#include <memory>
#include <string>
#include <mutex>
#include "../../util/fix_string.h"
#include "Interface/log/i_logger.h"
#include "empty_logger.h"

namespace jaf
{
namespace log
{

// ������־
// LogName ��־��
template <::jaf::fixed_string LogName>
class CommonLogger
{
	CommonLogger() {}
	CommonLogger(const CommonLogger&) = delete;
	CommonLogger& operator=(const CommonLogger&) = delete;
public:
	~CommonLogger() {}

public:
	static void SetLogger(std::shared_ptr<ILogger> logger)
	{
		std::unique_lock<std::mutex> ul(instance_.logger_mutex_);
		instance_.logger_ = logger;
	}
	static std::shared_ptr<ILogger> Logger()
	{
		std::unique_lock<std::mutex> ul(instance_.logger_mutex_);
		return instance_.logger_;
	}

protected:
	static CommonLogger instance_; // ����ʵ��

	std::mutex logger_mutex_;
	std::shared_ptr<ILogger> logger_ = std::make_shared<EmptyLogger>();
};

template <::jaf::fixed_string LogName>
CommonLogger<LogName> CommonLogger<LogName>::instance_;  // ����ʵ��

}
}