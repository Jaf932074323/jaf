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
#include "Impl/communication_include.h"
#include "Interface/communication/i_pack.h"
#include "Interface/communication/i_serial_port.h"
#include "Interface/communication/i_tcp_client.h"
#include "Interface/communication/i_tcp_server.h"
#include "Interface/communication/i_udp.h"
#include "util/co_coroutine.h"
#include "util/co_wait_util_stop.h"
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
    void Deal(std::shared_ptr<jaf::comm::IPack> pack);

private:
    std::shared_ptr<jaf::comm::Communication> communication_ = std::make_shared<jaf::comm::Communication>();
    std::shared_ptr<jaf::comm::ITcpServer> server_           = nullptr;
    std::shared_ptr<jaf::comm::ITcpClient> client_           = nullptr;
    std::shared_ptr<jaf::comm::IUdp> udp_                    = nullptr;
    std::shared_ptr<jaf::comm::ISerialPort> serial_port_     = nullptr;

    jaf::Latch wait_finish_latch_{1};

    jaf::CoWaitUtilStop wait_stop_;
};
