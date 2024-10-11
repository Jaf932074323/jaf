#pragma once
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
// 2024-8-27 姜安富
#include <coroutine>
#include <latch>
#include <mutex>

namespace jaf
{

template <typename T>
struct PromiseReturnWithWait
{
    void return_value(T v)
    {
        value = std::move(v);
        latch_.count_down();
    }

    T value;
    std::latch latch_{1};
};

template <>
struct PromiseReturnWithWait<void>
{
    void return_void()
    {
        latch_.count_down();
    }

    std::latch latch_{1};
};

template <typename T>
struct CoroutineWithWait
{
    struct promise_type : public PromiseReturnWithWait<T>
    {
        std::coroutine_handle<> parent_handle;
        bool final_flag_ = false;
        std::mutex parent_handle_mutex;

        auto get_return_object()
        {
            return CoroutineWithWait<T>{std::coroutine_handle<promise_type>::from_promise(static_cast<promise_type&>(*this))};
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

                std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> co_handle) noexcept
                {
                    promise_type& promise = co_handle.promise();
                    std::unique_lock<std::mutex> lock(promise.parent_handle_mutex);
                    promise.final_flag_ = true;
                    return promise.parent_handle ? promise.parent_handle : std::noop_coroutine();
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

        // 阻塞等待
        void Wait()
        {
            PromiseReturnWithWait<T>::latch_.wait();
        }
    };

    CoroutineWithWait(std::coroutine_handle<promise_type> h)
        : handle{h}
    {
    }
    ~CoroutineWithWait()
    {
    }

    CoroutineWithWait(const CoroutineWithWait&)            = delete;
    CoroutineWithWait& operator=(const CoroutineWithWait&) = delete;

    CoroutineWithWait(CoroutineWithWait&& other) noexcept
        : handle{other.handle}
    {
        other.handle = nullptr;
    }

    CoroutineWithWait& operator=(CoroutineWithWait&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        handle       = other.handle;
        other.handle = nullptr;
        return *this;
    }

    bool await_ready()
    {
        return false;
    }

    bool await_suspend(std::coroutine_handle<> co_handle)
    {
        promise_type& promise = handle.promise();
        std::unique_lock<std::mutex> lock(promise.parent_handle_mutex);
        if (promise.final_flag_)
        {
            return false;
        }
        promise.parent_handle = co_handle;
        return true;
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

    // 阻塞等待协程执行完成
    void Wait()
    {
        handle.promise().Wait();
    }

    std::coroutine_handle<promise_type> handle;
};

} // namespace jaf
