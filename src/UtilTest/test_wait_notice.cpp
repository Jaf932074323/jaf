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
#include "util/co_wait_notice.h"
#include "util/co_wait_notices.h"
#include "gtest/gtest.h"
#include <format>
#include <list>

TEST(wait_notice, notice)
{
    int number = 0;
    jaf::CoWaitNotice wait_notice;
    wait_notice.Start();

    auto co_fun = [&]() -> jaf::CoroutineWithWait<void> {
        co_await wait_notice.Wait();
        ++number;
    };
    auto co_fun_obj = co_fun();
    wait_notice.Notify();
    co_fun_obj.Wait();

    wait_notice.Stop();

    EXPECT_EQ(number, 1);
}

TEST(wait_notice, stop)
{
    int number = 0;
    jaf::CoWaitNotice wait_notice;
    wait_notice.Start();

    auto co_fun = [&]() -> jaf::CoroutineWithWait<void> {
        co_await wait_notice.Wait();
        ++number;
    };
    auto co_fun_obj = co_fun();
    wait_notice.Stop();
    co_fun_obj.Wait();

    EXPECT_EQ(number, 1);
}


TEST(wait_notices, notice)
{
    std::size_t times = 10;
    int number        = 0;
    jaf::CoWaitNotices wait_notices;
    wait_notices.Start(times);

    auto co_fun = [&]() -> jaf::CoroutineWithWait<void> {
        co_await wait_notices.Wait();
        ++number;
    };
    auto co_fun_obj = co_fun();
    for (size_t i = 0; i < times; ++i)
    {
        wait_notices.Notify();
    }
    co_fun_obj.Wait();
    EXPECT_EQ(number, 1);

    wait_notices.Stop();
}

TEST(wait_notices, stop)
{
    std::size_t times = 10;
    int number = 0;
    jaf::CoWaitNotices wait_notices;
    wait_notices.Start(times);

    auto co_fun = [&]() -> jaf::CoroutineWithWait<void> {
        co_await wait_notices.Wait();
        ++number;
    };
    auto co_fun_obj = co_fun();
    wait_notices.Stop();
    co_fun_obj.Wait();
    EXPECT_EQ(number, 1);
}
