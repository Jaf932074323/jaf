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
// 2024-6-16 ������
#include "define_constant.h"
#include "log_head.h"
#include "main_class.h"
#include "time_head.h"
#include <iostream>
#include <thread>
#include <winsock2.h>
#include "gtest/gtest.h"
#include "global_thread_pool/global_thread_pool.h"
#include "util/simple_thread_pool.h"

int main(int argc, char** argv)
{
    std::shared_ptr<jaf::log::ConsoleAppender> appender = std::make_shared<jaf::log::ConsoleAppender>();
    jaf::log::CommonLogger::SetDefaultLogger(std::make_shared<jaf::log::Logger>(appender));
    jaf::log::CommonLogger::SetLogger(jaf::comm::LOG_NAME, std::make_shared<jaf::log::Logger>(appender));
    LOG_INFO() << "��־��ʼ�����";

    std::shared_ptr<jaf::time::Timer> timer = std::make_shared<jaf::time::Timer>();
    jaf::time::GlobalTimer::SetTimer(timer);

    jaf::GlobalThreadPool::SetThreadPool(std::make_shared<jaf::SimpleThreadPool>(1));

    WSAData version;
    WSAStartup(WINSOCK_VERSION, &version);

    //Main main;
    //main.Run();

    //getchar();

    //main.Stop();
    //main.WaitFinish();

    //LOG_INFO() << "�������";

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

    WSACleanup();

    return 0;
}