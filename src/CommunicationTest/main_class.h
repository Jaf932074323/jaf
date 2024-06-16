#pragma once
#include "impl/channel_user.h"
#include "impl/iocp.h"
#include "Interface/communication/i_tcp_server.h"
#include "Interface/communication/i_tcp_client.h"
#include "Interface/communication/i_udp.h"
#include "Interface/communication/i_serial_port.h"
#include "util/co_await.h"
#include "util/co_coroutine.h"
#include <memory>

// 处理通信通道通信通道 负责从通道读写数据
class Main
{
public:
    Main();
    virtual ~Main();

    jaf::Coroutine<void> Run();
    void Stop();
    void WaitFinish(); // 阻塞等待Run结束

private:
    void Init();

private:
    std::shared_ptr<jaf::comm::Iocp> iocp_                = std::make_shared<jaf::comm::Iocp>();
    std::shared_ptr<jaf::comm::ChannelUser> channel_user_ = nullptr;
    std::shared_ptr<jaf::comm::ITcpServer> server_        = nullptr;
    std::shared_ptr<jaf::comm::ITcpClient> client_        = nullptr;
    std::shared_ptr<jaf::comm::IUdp> udp_                 = nullptr;
    std::shared_ptr<jaf::comm::ISerialPort> serial_port_  = nullptr;

    jaf::Latch wait_finish_latch_{1};

    jaf::CoAwait await_stop_;
};
