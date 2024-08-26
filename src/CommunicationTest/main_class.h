#pragma once
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
#include "impl/channel_user.h"
#include "impl/iocp.h"
#include "Interface/communication/i_tcp_server.h"
#include "Interface/communication/i_tcp_client.h"
#include "Interface/communication/i_udp.h"
#include "Interface/communication/i_serial_port.h"
#include "util/co_await.h"
#include "util/co_coroutine.h"
#include "util/latch.h"
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
