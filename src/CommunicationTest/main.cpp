#include "define_constant.h"
#include "log_head.h"
#include "main_class.h"
#include "time_head.h"
#include <iostream>
#include <thread>
#include <winsock2.h>

int main()
{
    std::shared_ptr<jaf::log::ConsoleAppender> appender = std::make_shared<jaf::log::ConsoleAppender>();
    jaf::log::CommonLogger::SetDefaultLogger(std::make_shared<jaf::log::Logger>(appender));
    jaf::log::CommonLogger::SetLogger(jaf::comm::LOG_NAME, std::make_shared<jaf::log::Logger>(appender));
    LOG_INFO() << "日志初始化完成";

    std::shared_ptr<jaf::time::Timer> timer = std::make_shared<jaf::time::Timer>();
    timer->Start();
    jaf::time::CommonTimer::SetTimer(timer);

    WSAData version;
    WSAStartup(WINSOCK_VERSION, &version);

    Main main;
    main.Run();

    getchar();

    main.Stop();
    main.WaitFinish();

    WSACleanup();

    timer->Stop();

    LOG_INFO() << "程序结束";
    return 0;
}