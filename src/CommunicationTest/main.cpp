#include <iostream>
#include <thread>
#include <winsock2.h>
#include "log_head.h"
#include "define_constant.h"
#include "main_class.h"
#include "time_head.h"

int main()
{
	std::shared_ptr<jaf::log::ConsoleAppender> appender = std::make_shared<jaf::log::ConsoleAppender>();
	jaf::log::CommonLogger::SetDefaultLogger(std::make_shared<jaf::log::Logger>(appender));
	jaf::log::CommonLogger::SetLogger(jaf::comm::LOG_NAME, std::make_shared<jaf::log::Logger>(appender));
	LOG_INFO() << "��־��ʼ�����";

	std::shared_ptr<jaf::time::Timer> timer = std::make_shared<jaf::time::Timer>();
	timer->Start();
	jaf::time::CommonTimer::SetTimer(timer);

	WSAData version;
	WSAStartup(WINSOCK_VERSION, &version);

	Main main;
	main.Start();

	getchar();

	main.Stop();

	WSACleanup();
	LOG_INFO() << "�������";

	return 0;
}