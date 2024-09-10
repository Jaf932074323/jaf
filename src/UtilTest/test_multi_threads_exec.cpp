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
// 2024-6-20 ������
#include "util/co_coroutine.h"
#include "util/co_coroutine_with_wait.h"
#include "util/multi_threads_exec.h"
#include "gtest/gtest.h"
#include <format>
#include <list>

class TestMultiThreadsExec
{
public:
    TestMultiThreadsExec()
    {
        multi_thread_exec_.Start();
    }
    ~TestMultiThreadsExec()
    {
        multi_thread_exec_.Stop();
    }

public:
    jaf::CoroutineWithWait<void> Test()
    {
        // ���Է�ʽ��
        // �����������е�Ԫ��ȫ����0
        // Ȼ����ִ������ʱ��ÿ���������ò���������һ��Ԫ�ص�ֵ�������±�
        // ����ȫ��ִ����ɺ󣬼���������ĵ�ÿ��Ԫ��ֵ�Ƿ񸴺�Ԥ��

        main_thread_id_ = std::this_thread::get_id();

        std::vector<jaf::Coroutine<void>> co_coroutines;
        co_coroutines.reserve(deal_time);

        // ׼����������
        test_numbers_.resize(deal_time);
        for (int64_t i = 0; i < deal_time; ++i)
        {
            test_numbers_[i] = 0;
        }

        // ִ�д�������
        for (int64_t i = 0; i < deal_time; ++i)
        {
            co_coroutines.push_back(Deal(i));
        }
        for (auto& co_coroutine : co_coroutines)
        {
            co_await co_coroutine;
        }

        // ���
        for (int64_t i = 0; i < deal_time; ++i)
        {
            EXPECT_EQ(test_numbers_[i], i) << std::format("��{}�����ִ������Ԥ��={},ʵ��ֵ={}", i, i, test_numbers_[i]);
        }

        co_return;
    }

private:
    jaf::Coroutine<void> Deal(size_t number)
    {
        co_await multi_thread_exec_.Switch(); // �л��߳�ִ��

        EXPECT_NE(main_thread_id_, std::this_thread::get_id());

        test_numbers_[number] += number;

        co_return;
    }

private:
    const int64_t deal_time = 10000;
    jaf::MultiThreadsExec multi_thread_exec_{10};
    std::vector<int64_t> test_numbers_; // �������飬���ڼ���Ƿ����ظ�����©
    std::thread::id main_thread_id_;    // ���߳�id�����ڼ���Ƿ�����л������߳�ִ��
};

TEST(multi_threads_exec, usual)
{
    TestMultiThreadsExec test;
    auto co_fun_test = test.Test();
    co_fun_test.Wait();
}
