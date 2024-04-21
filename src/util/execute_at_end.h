#pragma once
#include <functional>

class ExecuteAtEnd
{
public:
    ExecuteAtEnd(std::function<void()> &&func) : m_func(std::move(func))
    {
    }

    ~ExecuteAtEnd()
    {
        m_func();
    }

private:
    std::function<void()> m_func;
};

#define EXECUTE_AT_END_CAT(x, y) EXECUTE_AT_END_CAT_HELPER(x, y)
#define EXECUTE_AT_END_CAT_HELPER(x, y) x##y
#define EXECUTE_AT_END(code) ExecuteAtEnd EXECUTE_AT_END_CAT(_ExecuteAtEnd_, __LINE__)([&]() { code })

