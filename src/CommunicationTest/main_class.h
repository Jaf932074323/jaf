#pragma once
#include "Impl/channel_user.h"
#include "Impl/iocp.h"
#include "Impl/serial_port.h"
#include "Impl/tcp_client.h"
#include "Impl/tcp_server.h"
#include "Impl/udp.h"
#include "util/co_coroutine.h"
#include <memory>

// 处理通信通道通信通道 负责从通道读写数据
class Main
{
public:
    Main();
    virtual ~Main(){};

    jaf::Coroutine<void> Run();
    void Stop();
    void WaitFinish(); // 阻塞等待Run结束

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
};
