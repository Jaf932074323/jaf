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
// 2024-6-16 姜安富
#include "define_constant.h"
#include "global_thread_pool/global_thread_pool.h"
#include "init_socket.h"
#include "log_head.h"
#include "main_class.h"
#include "time_head.h"
#include "util/simple_thread_pool.h"
#include "gtest/gtest.h"
#include <iostream>
#include <thread>
#ifdef _WIN32
#include <ConsoleApi2.h>
#elif defined(__linux__)
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#elif defined(__linux__)
#endif

    std::shared_ptr<jaf::log::ConsoleAppender> appender = std::make_shared<jaf::log::ConsoleAppender>();
    jaf::log::CommonLogger::SetDefaultLogger(std::make_shared<jaf::log::Logger>(appender));
    jaf::log::CommonLogger::SetLogger(jaf::comm::LOG_NAME, std::make_shared<jaf::log::Logger>(appender));
    LOG_INFO() << "日志初始化完成";

    std::shared_ptr<jaf::time::Timer> timer = std::make_shared<jaf::time::Timer>();
    jaf::time::GlobalTimer::SetTimer(timer);

    jaf::GlobalThreadPool::SetThreadPool(std::make_shared<jaf::SimpleThreadPool>(1));

    InitSocket init_socket;

    //Main main;
    //main.Run();

    //getchar();

    //main.Stop();
    //main.WaitFinish();

    //LOG_INFO() << "程序结束";

    //::testing::GTEST_FLAG(filter) = "tcp.usual";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}