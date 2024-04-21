#pragma once
#include <memory>
#include "util/co_coroutine.h"
#include "Impl/iocp.h"
#include "Impl/tcp_server.h"
#include "Impl/tcp_client.h"
#include "Impl/udp.h"
#include "Impl/serial_port.h"
#include "Impl/channel_user.h"

// 处理通信通道通信通道 负责从通道读写数据
class Main
{
public:
	Main(){}
	virtual ~Main() {};

	void Start();
	void Stop();

private:
	void Init();

private:
	std::shared_ptr<jaf::comm::Iocp> iocp_ = std::make_shared<jaf::comm::Iocp>();
	std::shared_ptr<jaf::comm::TcpServer> server_ = nullptr;
	std::shared_ptr<jaf::comm::TcpClient> client_ = nullptr;
	std::shared_ptr<jaf::comm::ChannelUser> channel_user_ = nullptr;
	std::shared_ptr<jaf::comm::Udp> udp_ = nullptr;
	std::shared_ptr<jaf::comm::SerialPort> serial_port_ = nullptr;

};
