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
// 2024-10-19 姜安富
#include "Communication/Impl/iocp.h"
#include "Interface/communication/i_serial_port.h"
#include "Interface/communication/i_tcp_client.h"
#include "Interface/communication/i_tcp_server.h"
#include "Interface/communication/i_udp.h"
#include "global_thread_pool/global_thread_pool.h"
#include "global_timer/co_sleep.h"
#include "global_timer/global_timer.h"
#include "unpack.h"
#include "util/co_coroutine.h"
#include "util/co_coroutine_with_wait.h"
#include "util/co_wait_notices.h"
#include "gtest/gtest.h"
#include <format>
#include <list>

TEST(serial, usual)
{
    auto co_fun = []() -> jaf::CoroutineWithWait<void> {
        jaf::comm::Iocp iocp(jaf::GlobalThreadPool::ThreadPool(), jaf::time::GlobalTimer::Timer());
        jaf::Coroutine<void> iocp_run = iocp.Run();

        std::string str = "hello world!";
        jaf::CoWaitNotices wait_recv; // 等待接收通知

        auto fun_deal = [&](std::shared_ptr<jaf::comm::IPack> pack) -> jaf::CoroutineWithWait<void> {
            auto [buff, len] = pack->GetData();
            std::string recv_str((const char*) buff, len);
            EXPECT_TRUE(recv_str == str);
            wait_recv.Notify();

            auto result = co_await pack->GetChannel()->Write((const unsigned char*) str.data(), str.length(), 1000);
            co_return;
        };
        std::shared_ptr<Unpack> unpack = std::make_shared<Unpack>(fun_deal);

        auto fun_deal_client_channel = [&](std::shared_ptr<jaf::comm::IChannel> channel) -> jaf::Coroutine<void> {
            auto unpack_run = unpack->Run(channel);
            auto result = co_await channel->Write((const unsigned char*) str.data(), str.length(), 1000);
            co_await unpack_run;
        };

        std::shared_ptr<jaf::comm::ISerialPort> serial_port_1 = iocp.CreateSerialPort();
        serial_port_1->SetAddr(11, 9600, 8, 0, 0);
        serial_port_1->SetHandleChannel(fun_deal_client_channel);

        std::shared_ptr<jaf::comm::ISerialPort> serial_port_2 = iocp.CreateSerialPort();
        serial_port_2->SetAddr(21, 9600, 8, 0, 0);
        serial_port_2->SetHandleChannel(fun_deal_client_channel);
        //serial_port_2->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));

        wait_recv.Start(10);

        jaf::Coroutine<void> run_1 = serial_port_1->Run();
        jaf::Coroutine<void> run_2 = serial_port_2->Run();

        co_await wait_recv;

        serial_port_1->Stop();
        serial_port_2->Stop();

        co_await run_1;
        co_await run_2;

        iocp.Stop();
        co_await iocp_run;
    };

    auto co_test_co_await_time = co_fun();
    co_test_co_await_time.Wait();
}
