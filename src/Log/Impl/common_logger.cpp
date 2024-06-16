// MIT License
//
// Copyright(c) 2021 Jaf932074323
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 2024-6-16 姜安富
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