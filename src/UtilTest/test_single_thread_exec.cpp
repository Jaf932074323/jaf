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
#include "util/single_thread_exec.h"
#include "gtest/gtest.h"
#include <list>

class TestSingleThreadExec
{
public:
    TestSingleThreadExec()
    {
        simple_thread_exec_.Start();
    }
    ~TestSingleThreadExec()
    {
        simple_thread_exec_.Stop();

    }
public:
    jaf::CoroutineWithWait<void> Test()
    {
        // 准备
        latch.Reset();
        number_                = 0;
        simple_thread_exec_id_ = simple_thread_exec_.GetThreadId();

        // 启动多个线程
        for (int64_t i = 0; i < thread_number_; ++i)
        {
            std::thread run_thread([this, i]() { ThreadRun(i); });
            run_thread.join();
        }

        // 等待多线程执行结束
        latch.Wait();

        EXPECT_EQ(number_, thread_number_ * thread_add_time_);

        co_return;
    }

private:
    jaf::Coroutine<void> ThreadRun(int64_t thread_number)
    {
        std::vector<jaf::Coroutine<void>> vec_add;
        vec_add.reserve(thread_add_time_);

        for (int64_t i = 0; i < thread_add_time_; ++i)
        {
            vec_add.push_back(Add(thread_number, i));
        }

        for (int64_t i = 0; i < thread_add_time_; ++i)
        {
            co_await vec_add[i];
        }

        latch.CountDown();
    }

    jaf::Coroutine<void> Add(int64_t thread_number, int64_t thread_add_time)
    {
        auto co_switch = simple_thread_exec_.Switch();
        co_await co_switch; // 切换到单线程执行

        EXPECT_EQ(simple_thread_exec_id_, std::this_thread::get_id());

        number_ = number_ + 1; // 已经切换到单个线程了，不需要加锁
    }

private:
    const int64_t thread_number_  = 10;
    const int64_t thread_add_time_ = 10000;

    int64_t number_ = 0;
    jaf::Latch latch{thread_number_};
    jaf::SimpleThreadExec simple_thread_exec_;

    std::thread::id simple_thread_exec_id_;
};

TEST(single_thread_exec, usual)
{
    TestSingleThreadExec test;
    auto co_test = test.Test();
    co_test.Wait();
}
