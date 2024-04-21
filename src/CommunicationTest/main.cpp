#include <iostream>
#include <thread>
#include "log_head.h"
#include "define_constant.h"
#include "main_class.h"

int main()
{
	std::shared_ptr<jaf::log::ConsoleAppender> appender = std::make_shared<jaf::log::ConsoleAppender>();
	jaf::log::CommonLogger<"">::SetLogger(std::make_shared<jaf::log::Logger>(appender));
	jaf::log::CommonLogger<jaf::comm::LOG_NAME>::SetLogger(std::make_shared<jaf::log::Logger>(appender));
	LOG_INFO() << "日志初始化完成";

	WSAData version;
	WSAStartup(WINSOCK_VERSION, &version);

	Main main;
	main.Start();

	getchar();

	main.Stop();

	WSACleanup();
	LOG_INFO() << "程序结束";

	return 0;
}