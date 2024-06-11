#pragma once
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

    void arrive_and_wait(const int64_t update = 1)
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
