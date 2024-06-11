#include "common_logger.h"
#include "empty_logger.h"
#include <mutex>
#include <string>
#include <unordered_map>

namespace jaf
{
namespace log
{

struct CommonLogger::Impl
{
    std::mutex logger_mutex_;
    std::shared_ptr<ILogger> default_logger_ = std::make_shared<EmptyLogger>();
    std::unordered_map<std::string, std::shared_ptr<ILogger>> loggers_; // 日志集合
};

CommonLogger::CommonLogger()
{
    m_impl = new Impl();
}

CommonLogger::~CommonLogger()
{
    delete m_impl;
}

void CommonLogger::SetDefaultLogger(std::shared_ptr<ILogger> logger)
{
    std::unique_lock<std::mutex> ul(Instance().m_impl->logger_mutex_);
    Instance().m_impl->default_logger_ = logger;
}

std::shared_ptr<ILogger> CommonLogger::DefaultLogger()
{
    std::unique_lock<std::mutex> ul(Instance().m_impl->logger_mutex_);
    return Instance().m_impl->default_logger_;
}

void CommonLogger::SetLogger(const std::string& log_name, std::shared_ptr<ILogger> logger)
{
    std::unique_lock<std::mutex> ul(Instance().m_impl->logger_mutex_);
    Instance().m_impl->loggers_[log_name] = logger;
}

std::shared_ptr<ILogger> CommonLogger::Logger(const std::string& log_name)
{
    std::unique_lock<std::mutex> ul(Instance().m_impl->logger_mutex_);
    auto it = Instance().m_impl->loggers_.find(log_name);
    if (it == Instance().m_impl->loggers_.end())
    {
        return nullptr;
    }
    return it->second;
}

CommonLogger& CommonLogger::Instance()
{
    static CommonLogger instance; // 单例实例
    return instance;
}

} // namespace log
} // namespace jaf