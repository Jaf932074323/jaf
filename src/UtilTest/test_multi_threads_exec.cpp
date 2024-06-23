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
#include "test_multi_threads_exec.h"
#include "log_head.h"
#include "util/co_coroutine.h"
#include "util/latch.h"
#include "util/multi_threads_exec.h"
#include <format>
#include <iostream>
#include <list>

class TestMultiThreadsExec
{
public:
    jaf::Coroutine<void> Test(jaf::Latch& wait_finish)
    {
        // 测试方式：
        // 将测试数组中的元素全部置0
        // 然后在执行任务时，每个任务设置测试数组中一个元素的值加上其下标
        // 任务全部执行完成后，检查测试数组的的每项元素值是否复合预期

        // 准备测试数组
        test_numbers_.resize(deal_time);
        for (int64_t i = 0; i < deal_time; ++i)
        {
            test_numbers_[i] = 0;
        }

        // 执行处理任务
        auto run = multi_thread_exec_.Run();
        for (int64_t i = 0; i < deal_time; ++i)
        {
            Deal(i);
        }
        multi_thread_exec_.Stop();
        co_await run;

        // 检查
        bool check_pass = true;
        for (int64_t i = 0; i < deal_time; ++i)
        {
            if (test_numbers_[i] != i)
            {
                check_pass = false;
                LOG_ERROR() << std::format("第{}项数字处理错误，预计={},实际值={}", i, i, test_numbers_[i]);
            }
        }
        if (check_pass)
        {
            LOG_ERROR() << "检查数字通过";
        }
        else
        {
            LOG_ERROR() << "检查数字未通过";
        }

        wait_finish.CountDown();
        co_return;
    }

private:
    jaf::Coroutine<void> Deal(size_t number)
    {
        co_await multi_thread_exec_.Switch(); // 切换线程执行

        test_numbers_[number] += number;
        LOG_INFO() << std::to_string(number);

        co_return;
    }

private:
    const int64_t deal_time = 1000;
    jaf::MultiThreadsExec multi_thread_exec_{10};
    std::vector<int64_t> test_numbers_; // 测试数组，用于检查是否有重复和遗漏
};

void test_multi_threads_exec()
{
    jaf::Latch wait_finish(1);
    TestMultiThreadsExec test;
    test.Test(wait_finish);

    wait_finish.Wait();

    return;
}
