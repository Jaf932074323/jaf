#pragma once
#include <exception>
#include <coroutine>

namespace jaf
{

template <typename T>
struct Coroutine;
template <typename T, bool is_void = std::is_void_v<T>>
struct PromiseType;
template <typename T>
using HandleType = std::coroutine_handle<PromiseType<T>>;

template <typename T>
struct PromiseBase
{
    std::coroutine_handle<> parent_handle;

    auto get_return_object()
    {
        return Coroutine<T>{ HandleType<T>::from_promise(static_cast<PromiseType<T>&>(*this)) };
    }

    auto initial_suspend()
    {
        return std::suspend_never{};
    }

    auto final_suspend() noexcept
    {
        struct Awaitable
        {
            bool await_ready() noexcept
            {
                return false;
            }

            auto await_suspend(HandleType<T> co_handle) noexcept
            {
                auto parent = co_handle.promise().parent_handle;
                return parent ? parent : std::noop_coroutine();
            }

            void await_resume() noexcept
            {
            }
        };

        return Awaitable{};
    }

    void unhandled_exception()
    {
        std::terminate();
    }
};

template <typename T, bool is_void>
struct PromiseType : public PromiseBase<T>
{
    T value;

    void return_value(T v)
    {
        value = std::move(v);
    }
};

template <typename T>
struct PromiseType<T, true> : public PromiseBase<T>
{
    void return_void()
    {
    }
};

template <typename T>
struct Coroutine
{
    using promise_type = PromiseType<T>;

    Coroutine(HandleType<T> h) : handle{ h }
    {
    }
    ~Coroutine()
    {
    }

    Coroutine(const Coroutine&) = delete;
    Coroutine& operator=(const Coroutine&) = delete;

    Coroutine(Coroutine&& other) noexcept : handle{ other.handle }
    {
        other.handle = nullptr;
    }

    Coroutine& operator=(Coroutine&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        handle = other.handle;
        other.handle = nullptr;
        return *this;
    }

    bool await_ready()
    {
        return handle.done();
    }

    void await_suspend(std::coroutine_handle<> co_handle)
    {
        handle.promise().parent_handle = co_handle;
    }

    decltype(auto) await_resume()
    {
        if constexpr (std::is_void_v<T>)
        {
            return;
        }
        else
        {
            return std::move(handle.promise().value);
        }
    }

    HandleType<T> handle;
};

}

