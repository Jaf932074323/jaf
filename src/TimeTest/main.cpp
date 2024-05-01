#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <format>
#include "Impl/co_notify.h"
#include "log_head.h"
#include "time_head.h"

jaf::Coroutine<void> TestCoTimer()
{
	//jaf::time::CoTimer co_timer;
	LOG_INFO() << "start sleep";
	//co_await co_timer.Sleep(5000);
	co_await jaf::time::CoSleep(2000);
	LOG_INFO() << "finilly sleep";
	co_return;
}

jaf::Coroutine<void> WaitNotify(jaf::time::CoNotify& notify)
{
	LOG_INFO() << "void WaitNotify() 1";
	bool success = co_await notify.Wait(1000);
	LOG_INFO() << std::format("void WaitNotify() 2 success = {}", success);
	co_return;
}

jaf::Coroutine<void> TestNotify()
{
	jaf::time::CoNotify notify;
	WaitNotify(notify);
	LOG_INFO() << "void TestNotify() 1";
	//co_await jaf::time::CoSleep(2000);
	notify.Notify();
	LOG_INFO() << "void TestNotify() 2";
	WaitNotify(notify);
	LOG_INFO() << "void TestNotify() 3";
	notify.Notify();
	LOG_INFO() << "void TestNotify() 4";
	co_return;
}

jaf::Coroutine<void> TestTimer()
{
	jaf::time::Timer timer;
	timer.Start();

	std::mutex task_mutex;
	bool run = true;

	jaf::time::STimerPara task;
	task.fun = [&]()
		{
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
	LOG_INFO() << "¶¨Ê±²âÊÔÆô¶¯";

	std::shared_ptr<jaf::time::Timer> timer = std::make_shared<jaf::time::Timer>();
	timer->Start();
	jaf::time::CommonTimer::SetTimer(timer);

	TestNotify();
	//TestCoTimer();
	//TestTimer();

	getchar();

	timer->Stop();

	return 0;
}