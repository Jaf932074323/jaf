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
// 2024-6-20 ½ª°²¸»
#include "util/co_coroutine.h"
#include "util/co_coroutine_with_wait.h"
#include "util/concurrent_queue.h"
#include "gtest/gtest.h"
#include <atomic>
#include <format>
#include <functional>
#include <thread>
#include <vector>

TEST(concurrent_queue, usual)
{
    size_t count          = 100;
    size_t producer_count = 2;
    size_t consumer_count = 2;

    std::vector<int> consumer_cal_sum(consumer_count);
    ConcurrentQueue<int> queue;
    std::atomic<bool> finish_flag = false;

    auto producer_fun = [&]() {
        for (size_t i = 0; i < count; ++i)
        {
            queue.Push(i);
        }
    };

    auto consumer_fun = [&](int& sum) {
        int number = 0;

        while (!finish_flag)
        {
            if (queue.WaitAndPop(number))
            {
                sum += number;
            }
        }
        while (queue.TryPop(number))
        {
            sum += number;
        }
    };

    std::vector<std::thread> producer_threads;
    std::vector<std::thread> consumer_threads;
    producer_threads.reserve(producer_count);
    consumer_threads.reserve(consumer_count);

    for (size_t i = 0; i < producer_count; ++i)
    {
        producer_threads.push_back(std::thread(producer_fun));
    }

    for (size_t i = 0; i < consumer_count; ++i)
    {
        consumer_threads.push_back(std::thread(std::bind(consumer_fun, std::ref(consumer_cal_sum[i]))));
    }

    for (size_t i = 0; i < producer_count; ++i)
    {
        producer_threads[i].join();
    }
    finish_flag = true;
    queue.QuitAllWait();

    for (size_t i = 0; i < consumer_count; ++i)
    {
        consumer_threads[i].join();
    }

    int sum = 0;
    for (size_t i = 0; i < consumer_count; ++i)
    {
        sum += consumer_cal_sum[i];
    }

    EXPECT_EQ(sum, (count - 1 + 0) * count / 2 * producer_count);
}
