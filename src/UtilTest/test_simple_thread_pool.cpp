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
// 2024-6-20 姜安富
#include "util/co_coroutine.h"
#include "util/co_coroutine_with_wait.h"
#include "util/simple_thread_pool.h"
#include "gtest/gtest.h"
#include <atomic>
#include <format>
#include <latch>
#include <list>

TEST(simple_thread_pool , usual)
{
    size_t task_count = 100;
    std::latch the_latch(task_count);
    std::atomic sum = 0;

    jaf::SimpleThreadPool thread_pool(10);

    for (size_t i = 0; i < task_count; ++i)
    {
        thread_pool.Post([&]() {
            ++sum;
            the_latch.count_down();
        });
    }

    the_latch.wait();

    EXPECT_EQ(sum, task_count);
}
