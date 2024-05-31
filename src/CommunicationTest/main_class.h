#pragma once
#include "impl/channel_user.h"
#include "impl/iocp.h"
#include "impl/serial_port.h"
#include "impl/tcp_client.h"
#include "impl/tcp_server.h"
#include "impl/udp.h"
#include "util/co_coroutine.h"
#include "util/co_await.h"
#include <memory>

// ����ͨ��ͨ��ͨ��ͨ�� �����ͨ����д����
class Main
{
public:
    Main();
    virtual ~Main();
    ;

    jaf::Coroutine<void> Run();
    void Stop();
    void WaitFinish(); // �����ȴ�Run����

private:
    void Init();

private:
    std::shared_ptr<jaf::comm::Iocp> iocp_                = std::make_shared<jaf::comm::Iocp>();
    std::shared_ptr<jaf::comm::TcpServer> server_         = nullptr;
    std::shared_ptr<jaf::comm::TcpClient> client_         = nullptr;
    std::shared_ptr<jaf::comm::ChannelUser> channel_user_ = nullptr;
    std::shared_ptr<jaf::comm::Udp> udp_                  = nullptr;
    std::shared_ptr<jaf::comm::SerialPort> serial_port_   = nullptr;

    std::shared_ptr<std::latch> wait_finish_latch_ = std::make_shared<std::latch>(0);

    jaf::CoAwait await_stop_;
};
