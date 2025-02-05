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
#include "Communication/communication.h"
#include "global_timer/co_sleep.h"
#include "unpack.h"
#include "util/co_coroutine.h"
#include "util/co_coroutine_with_wait.h"
#include "util/co_wait_util_stop.h"
#include "util/simple_thread_pool.h"
#include "gtest/gtest.h"
#include <format>
#include <list>

TEST(udp, usual)
{
    std::shared_ptr<jaf::IThreadPool> thread_pool = std::make_shared<jaf::SimpleThreadPool>(); // TODO:这里需要调整

    auto co_fun = [thread_pool]() -> jaf::CoroutineWithWait<void> {
        std::shared_ptr<jaf::comm::ICommunication> communication = jaf::comm::CreateCommunication(thread_pool);
        jaf::Coroutine<jaf::comm::RunResult> communication_run   = communication->Run();

        std::string str = "hello world!";
        jaf::CoWaitUtilStop wait_recv; // 等待接收通知

        auto fun_deal = [&](std::shared_ptr<jaf::comm::IPack> pack) {
            auto [buff, len] = pack->GetData();
            std::string recv_str((const char*) buff, len);
            EXPECT_TRUE(recv_str == str);
            wait_recv.Stop();
        };


        std::string str_ip = "127.0.0.1";

        std::shared_ptr<Unpack> unpack = std::make_shared<Unpack>(fun_deal);

        std::shared_ptr<jaf::comm::IUdp> udp_1 = communication->CreateUdp();
        udp_1->SetAddr(jaf::comm::Endpoint(str_ip, 8081), jaf::comm::Endpoint(str_ip, 8082));
        udp_1->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));

        std::shared_ptr<jaf::comm::IUdp> udp_2 = communication->CreateUdp();
        udp_2->SetAddr(jaf::comm::Endpoint(str_ip, 8082), jaf::comm::Endpoint(str_ip, 8081));
        udp_2->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));

        wait_recv.Start();
        jaf::Coroutine<jaf::comm::RunResult> udp_1_run = udp_1->Run();
        jaf::Coroutine<jaf::comm::RunResult> udp_2_run = udp_2->Run();

        auto result = co_await udp_1->Write((const unsigned char*) str.data(), str.length(), 1000);
        co_await wait_recv.Wait();

        udp_1->Stop();
        udp_2->Stop();

        auto udp_1_run_result = co_await udp_1_run;
        auto udp_2_run_result = co_await udp_2_run;

        communication->Stop();
        auto communication_run_result = co_await communication_run;

        EXPECT_TRUE(udp_1_run_result);
        EXPECT_TRUE(udp_2_run_result);
        EXPECT_TRUE(communication_run_result);
    };

    auto co_test_co_await_time = co_fun();
    co_test_co_await_time.Wait();
}
