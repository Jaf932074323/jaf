#pragma once

namespace jaf
{

template <std::size_t N>
struct fixed_string
{

	constexpr fixed_string()
		:m_data{ 0 }
	{
	}

	constexpr fixed_string(const char(&foo)[N])
	{
		std::copy(foo, foo + N, m_data);
	}

	constexpr explicit fixed_string(const char* foo, int)
		:m_data{ 0 }
	{
		std::copy(foo, foo + N - 1, m_data);
	}

	auto operator<=>(const fixed_string&) const = default;

	constexpr static std::size_t len_ = N;
	char m_data[N];

};

}