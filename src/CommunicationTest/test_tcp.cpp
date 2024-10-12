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
// 2024-6-20 ������
#include "Communication/Impl/iocp.h"
#include "Interface/communication/i_serial_port.h"
#include "Interface/communication/i_tcp_client.h"
#include "Interface/communication/i_tcp_server.h"
#include "Interface/communication/i_udp.h"
#include "global_timer/co_sleep.h"
#include "unpack.h"
#include "util/co_coroutine.h"
#include "util/co_coroutine_with_wait.h"
#include "util/co_wait_notice.h"
#include "gtest/gtest.h"
#include <format>
#include <list>

TEST(tcp, usual)
{
    jaf::comm::Iocp iocp;
    jaf::Coroutine<void> iocp_run = iocp.Run();

    auto co_fun = [&iocp]() -> jaf::CoroutineWithWait<void> {
        std::string str = "hello world!";
        jaf::CoWaitNotice wait_recv; // �ȴ�����֪ͨ

        auto fun_deal = [&](std::shared_ptr<jaf::comm::IPack> pack) {
            auto [buff, len] = pack->GetData();
            std::string recv_str((const char*) buff, len);
            EXPECT_TRUE(recv_str == str);
            wait_recv.Stop();
        };
        std::shared_ptr<Unpack> unpack = std::make_shared<Unpack>(fun_deal);

        auto fun_deal_client_channel   = [&](std::shared_ptr<jaf::comm::IChannel> channel) -> jaf::Coroutine<void> {
            auto result = co_await channel->Write((const unsigned char*) str.data(), str.length(), 1000);
            co_await unpack->Run(channel);
        };

        std::string str_ip   = "127.0.0.1";
        uint16_t server_port = 8181;
        uint16_t client_port = 0;

        std::shared_ptr<jaf::comm::ITcpServer> server = iocp.CreateTcpServer();
        server->SetAddr(str_ip, server_port);
        server->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));
        server->SetAcceptCount(1);

        std::shared_ptr<jaf::comm::ITcpClient> client = iocp.CreateTcpClient();
        client->SetAddr(str_ip, server_port, str_ip, client_port);
        client->SetHandleChannel(fun_deal_client_channel);

        wait_recv.Start();

        jaf::Coroutine<void> server_run = server->Run();
        jaf::Coroutine<void> client_run = client->Run();

        co_await wait_recv.Wait();

        client->Stop();
        server->Stop();

        co_await server_run;
        co_await client_run;
    };

    auto co_test_co_await_time = co_fun();
    co_test_co_await_time.Wait();

    iocp.Stop();
}
