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
#include "Interface/communication/i_udp.h"
#include "global_thread_pool/global_thread_pool.h"
#include "global_timer/co_sleep.h"
#include "global_timer/global_timer.h"
#include "util/co_coroutine.h"
#include "util/co_coroutine_with_wait.h"
#include "util/co_wait_notices.h"
#include <format>
#include <functional>
#include <iostream>
#include <list>
#include <string>

namespace test_udp
{

jaf::CoroutineWithWait<void> Test()
{
    jaf::comm::Communication communication(jaf::GlobalThreadPool::ThreadPool(), jaf::time::GlobalTimer::Timer());
    jaf::Coroutine<jaf::comm::RunResult> communication_run = communication.Run();

    std::string str = "hello world!";

    auto fun_deal_client_channel = [&](std::shared_ptr<jaf::comm::IChannel> channel) -> jaf::Coroutine<void> {
        unsigned char buff[1024] = "";
        while (true)
        {
            auto read_result = co_await channel->Read(buff, 1024, 5000);
            std::cout << std::format("read {}, state {}, error {}", std::string((char*) buff), (int) read_result.state, read_result.error) << std::endl;
            if (read_result.state == jaf::comm::SChannelResult::EState::CRS_CHANNEL_END)
            {
                break;
            }
            if (read_result.state != jaf::comm::SChannelResult::EState::CRS_SUCCESS)
            {
                continue;
            }

            auto write_result = co_await channel->Write(buff, 1024, 5000);
            std::cout << std::format("write {}, state {}, error {}", std::string((char*) buff), (int) write_result.state, write_result.error) << std::endl;
            if (write_result.state == jaf::comm::SChannelResult::EState::CRS_CHANNEL_END)
            {
                break;
            }
            buff[0] = '\0';
        }
    };

    jaf::comm::Endpoint local_enpoint("127.0.0.1", 8182);
    jaf::comm::Endpoint remote_enpoint("127.0.0.1", 8181);

    std::shared_ptr<jaf::comm::IUdp> udp = communication.CreateUdp();
    udp->SetAddr(local_enpoint, remote_enpoint);
    udp->SetHandleChannel(fun_deal_client_channel);

    jaf::Coroutine<jaf::comm::RunResult> udp_run = udp->Run();

    getchar();

    udp->Stop();

    co_await udp_run;

    communication.Stop();
    co_await communication_run;
}

} // namespace test_udp

void TestUdp()
{
    auto run = test_udp::Test();
    run.Wait();
}
