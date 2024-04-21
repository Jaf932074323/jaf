#pragma once
#include <memory>
#include <type_traits>
#include "Interface/log/i_logger.h"
#include "manage_logger.h"

namespace jaf
{
namespace log
{

template <jaf::fixed_string Str = "">
struct RemoveStrQuotation
{
	constexpr static size_t start_ = Str.m_data[0] == '"' ? 1 : 0;
	constexpr static size_t len_ = Str.len_ - start_ - (Str.m_data[Str.len_ - start_ - 1] == '"' ? 1 : 0);

	constexpr static jaf::fixed_string<len_> str_{ Str.m_data + start_, 1};
};

template<::jaf::fixed_string LogName>
std::shared_ptr<ILogger> GetLog()
{
	return jaf::log::CommonLogger<LogName>::Logger();
};

template<::jaf::fixed_string LogName>
std::shared_ptr<ILogger> GetLog(const char[])
{
	return jaf::log::CommonLogger<LogName>::Logger();
};

template<::jaf::fixed_string LogName>
inline std::shared_ptr<ILogger> GetLog(std::shared_ptr<ILogger> logger)
{
	return logger;
};

}
}