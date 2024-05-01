#pragma once
#include <functional>

namespace jaf
{

class Finally
{
public:
    Finally(std::function<void()>&& func)
        : m_func(std::move(func))
    {
    }

    ~Finally()
    {
        m_func();
    }

private:
    std::function<void()> m_func;
};

} // namespace jaf

#define FINALLY_CAT(x, y) FINALLY_CAT_HELPER(x, y)
#define FINALLY_CAT_HELPER(x, y) x##y
#define FINALLY(code) Finally FINALLY_CAT(_Finally_, __LINE__)([&]() { code })
