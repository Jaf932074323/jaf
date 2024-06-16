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
#include "impl/co_await_time.h"
#include "log_head.h"
#include "time_head.h"
#include <chrono>
#include <format>
#include <iostream>
#include <string>
#include <thread>

jaf::Coroutine<void> TestCoTimer()
{
    //jaf::time::CoTimer co_timer;
    LOG_INFO() << "start sleep";
    //co_await co_timer.Sleep(5000);
    co_await jaf::time::CoSleep(2000);
    LOG_INFO() << "finilly sleep";
    co_return;
}

jaf::Coroutine<void> WaitWait(jaf::time::CoAwaitTime& await_time)
{
    LOG_INFO() << "void WaitWait() 1";
    bool success = co_await await_time.Wait(1000);
    LOG_INFO() << std::format("void WaitWait() 2 success = {}", success);
    co_return;
}

jaf::Coroutine<void> TestWait()
{
    jaf::time::CoAwaitTime await_time;
    await_time.Start();
    jaf::Coroutine<void> wait_Wait = WaitWait(await_time);
    LOG_INFO() << "void TestWait() 1";
    co_await jaf::time::CoSleep(2000);
    await_time.Notify();
    LOG_INFO() << "void TestWait() 2";
    //WaitWait(Wait);
    //LOG_INFO() << "void TestWait() 3";
    //Wait.Wait();
    //LOG_INFO() << "void TestWait() 4";

    co_await wait_Wait;

    await_time.Stop();
    co_return;
}

jaf::Coroutine<void> TestTimer()
{
    jaf::time::Timer timer;
    timer.Start();

    std::mutex task_mutex;
    bool run = true;

    jaf::time::STimerPara task;
    task.fun = [&](jaf::time::ETimerResultType result_type) {
        LOG_INFO() << "timer";
        std::unique_lock<std::mutex> lock(task_mutex);
        if (run)
        {
            timer.StartTask(task);
        }
    };
    task.interval = 0;
    timer.StartTask(task);

    co_await jaf::time::CoSleep(1000);

    {
        std::unique_lock<std::mutex> lock(task_mutex);
        run = false;
    }

    timer.Stop();
}


int main()
{
    std::shared_ptr<jaf::log::ConsoleAppender> appender = std::make_shared<jaf::log::ConsoleAppender>();
    jaf::log::CommonLogger::SetDefaultLogger(std::make_shared<jaf::log::Logger>(appender));
    LOG_INFO() << "定时测试启动";

    std::shared_ptr<jaf::time::Timer> timer = std::make_shared<jaf::time::Timer>();
    timer->Start();
    jaf::time::CommonTimer::SetTimer(timer);

    //TestWait();
    TestCoTimer();
    //TestTimer();

    getchar();

    timer->Stop();

    return 0;
}