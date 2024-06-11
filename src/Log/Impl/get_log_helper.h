#pragma once
#include "Interface/log/i_logger.h"
#include "common_logger.h"
#include <memory>
#include <type_traits>

namespace jaf
{
namespace log
{

inline std::shared_ptr<ILogger> GetLog()
{
    return jaf::log::CommonLogger::DefaultLogger();
};

inline std::shared_ptr<ILogger> GetLog(const char* log_name)
{
    return jaf::log::CommonLogger::Logger(log_name);
};

inline std::shared_ptr<ILogger> GetLog(std::shared_ptr<ILogger> logger)
{
    return logger;
};

} // namespace log
} // namespace jaf