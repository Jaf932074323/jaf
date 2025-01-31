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
// 2024-6-20 姜安富
#include "Communication/Impl/communication_include.h"
#include "Interface/communication/i_serial_port.h"
#include "Interface/communication/i_tcp_client.h"
#include "Interface/communication/i_tcp_server.h"
#include "Interface/communication/i_udp.h"
#include "global_timer/co_sleep.h"
#include "unpack.h"
#include "util/co_coroutine.h"
#include "util/co_coroutine_with_wait.h"
#include "gtest/gtest.h"
#include <format>
#include <list>

TEST(udp, usual)
{
    jaf::comm::Communication communication;
    jaf::Coroutine<void> communication_run = communication.Run();

    auto co_fun = [&communication]() -> jaf::CoroutineWithWait<void> {
        std::string str = "hello world!";
        jaf::CoWaitUtilStop wait_recv; // 等待接收通知

        auto fun_deal   = [&](std::shared_ptr<jaf::comm::IPack> pack) {
            auto [buff, len] = pack->GetData();
            std::string recv_str((const char*) buff, len);
            EXPECT_TRUE(recv_str == str);
            wait_recv.Stop();
        };


        std::string str_ip = "127.0.0.1";

        std::shared_ptr<Unpack> unpack = std::make_shared<Unpack>(fun_deal);

        std::shared_ptr<jaf::comm::IUdp> udp_1 = communication.CreateUdp();
        udp_1->SetAddr(jaf::comm::Endpoint(str_ip, 8081), jaf::comm::Endpoint(str_ip, 8082));
        udp_1->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));

        std::shared_ptr<jaf::comm::IUdp> udp_2 = communication.CreateUdp();
        udp_2->SetAddr(jaf::comm::Endpoint(str_ip, 8082), jaf::comm::Endpoint(str_ip, 8081));
        udp_2->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));

        wait_recv.Start();
        jaf::Coroutine<void> udp_1_run = udp_1->Run();
        jaf::Coroutine<void> udp_2_run = udp_2->Run();

        auto result = co_await udp_1->Write((const unsigned char*) str.data(), str.length(), 1000);
        co_await wait_recv.Wait();

        udp_1->Stop();
        udp_2->Stop();

        co_await udp_1_run;
        co_await udp_2_run;
    };

    auto co_test_co_await_time = co_fun();
    co_test_co_await_time.Wait();

    communication.Stop();
}
