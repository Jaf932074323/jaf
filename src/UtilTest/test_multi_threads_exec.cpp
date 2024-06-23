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
        // ���Է�ʽ��
        // �����������е�Ԫ��ȫ����0
        // Ȼ����ִ������ʱ��ÿ���������ò���������һ��Ԫ�ص�ֵ�������±�
        // ����ȫ��ִ����ɺ󣬼���������ĵ�ÿ��Ԫ��ֵ�Ƿ񸴺�Ԥ��

        // ׼����������
        test_numbers_.resize(deal_time);
        for (int64_t i = 0; i < deal_time; ++i)
        {
            test_numbers_[i] = 0;
        }

        // ִ�д�������
        auto run = multi_thread_exec_.Run();
        for (int64_t i = 0; i < deal_time; ++i)
        {
            Deal(i);
        }
        multi_thread_exec_.Stop();
        co_await run;

        // ���
        bool check_pass = true;
        for (int64_t i = 0; i < deal_time; ++i)
        {
            if (test_numbers_[i] != i)
            {
                check_pass = false;
                LOG_ERROR() << std::format("��{}�����ִ������Ԥ��={},ʵ��ֵ={}", i, i, test_numbers_[i]);
            }
        }
        if (check_pass)
        {
            LOG_ERROR() << "�������ͨ��";
        }
        else
        {
            LOG_ERROR() << "�������δͨ��";
        }

        wait_finish.CountDown();
        co_return;
    }

private:
    jaf::Coroutine<void> Deal(size_t number)
    {
        co_await multi_thread_exec_.Switch(); // �л��߳�ִ��

        test_numbers_[number] += number;
        LOG_INFO() << std::to_string(number);

        co_return;
    }

private:
    const int64_t deal_time = 1000;
    jaf::MultiThreadsExec multi_thread_exec_{10};
    std::vector<int64_t> test_numbers_; // �������飬���ڼ���Ƿ����ظ�����©
};

void test_multi_threads_exec()
{
    jaf::Latch wait_finish(1);
    TestMultiThreadsExec test;
    test.Test(wait_finish);

    wait_finish.Wait();

    return;
}
