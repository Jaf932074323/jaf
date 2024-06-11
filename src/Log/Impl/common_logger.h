#pragma once
#include "define_log_export.h"
#include "interface/log/i_logger.h"
#include <memory>

namespace jaf
{
namespace log
{

// 公用日志
class API_LOG_EXPORT CommonLogger
{
    CommonLogger();
    CommonLogger(const CommonLogger&)            = delete;
    CommonLogger& operator=(const CommonLogger&) = delete;

public:
    ~CommonLogger();

public:
    static void SetDefaultLogger(std::shared_ptr<ILogger> logger);
    static std::shared_ptr<ILogger> DefaultLogger();

    static void SetLogger(const std::string& log_name, std::shared_ptr<ILogger> logger);
    static std::shared_ptr<ILogger> Logger(const std::string& log_name);

private:
    static CommonLogger& Instance();

protected:
    struct Impl;
    Impl* m_impl;
};


} // namespace log
} // namespace jaf