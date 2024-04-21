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
	LOG_INFO() << "��־��ʼ�����";

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