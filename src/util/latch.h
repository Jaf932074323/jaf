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
// 2024-6-23 姜安富
#include <assert.h>
#include <atomic>

namespace jaf
{

class Latch
{
public:
    Latch(const int64_t expected)
        : counter_(expected)
        , expected_(expected)
    {
        assert(expected >= 0);
    }

    Latch(const Latch&)            = delete;
    Latch& operator=(const Latch&) = delete;

    // 增加重置功能，让Latch可以重复使用
    void Reset()
    {
        counter_.store(expected_);
    }

    void CountDown(const int64_t update = 1)
    {
        assert(update >= 0);
        const int64_t current = counter_.fetch_sub(update) - update;
        if (current == 0)
        {
            counter_.notify_all();
        }
        else
        {
            assert(current >= 0);
        }
    }

    bool TryWait() const noexcept
    {
        return counter_.load() == 0;
    }

    void Wait() const noexcept /* strengthened */
    {
        for (;;)
        {
            const int64_t current = counter_.load();
            if (current == 0)
            {
                return;
            }
            else
            {
                assert(current >= 0);
            }
            counter_.wait(current, std::memory_order_relaxed);
        }
    }

    void ArriveAndWait(const int64_t update = 1)
    {
        assert(update >= 0);
        const int64_t current = counter_.fetch_sub(update) - update;
        if (current == 0)
        {
            counter_.notify_all();
        }
        else
        {
            assert(current > 0);
            counter_.wait(current, std::memory_order_relaxed);
            Wait();
        }
    }

private:
    const int64_t expected_;
    std::atomic<int64_t> counter_;
};

} // namespace jaf
