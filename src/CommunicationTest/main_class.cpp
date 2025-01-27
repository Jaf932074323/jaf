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
#include "main_class.h"
#include "log_head.h"
#include "unpack.h"
#include "util/simple_thread_pool.h"

Main::Main()
    : communication_(std::make_shared<jaf::comm::Communication>(std::make_shared<jaf::SimpleThreadPool>(2)))
{
}

Main::~Main()
{
}

jaf::Coroutine<void> Main::Run()
{
    wait_finish_latch_.Reset();
    std::list<jaf::Coroutine<void>> coroutines;

    Init();

    wait_stop_.Start();

    jaf::Coroutine<void> communication_run = communication_->Run();
    //coroutines.push_back(server_->Run());
    //coroutines.push_back(client_->Run());
    coroutines.push_back(udp_->Run());
    //coroutines.push_back(serial_port_->Run());

    co_await wait_stop_.Wait();

    //server_->Stop();
    //client_->Stop();
    udp_->Stop();
    //serial_port_->Stop();
    for (auto& coroutine : coroutines)
    {
        co_await coroutine;
    }

    communication_->Stop();
    co_await communication_run;
    wait_finish_latch_.CountDown();
}

void Main::Stop()
{
    wait_stop_.Stop();
}

void Main::WaitFinish()
{
    wait_finish_latch_.Wait();
}

void Main::Init()
{
    communication_->Init();

    std::string str_ip = "127.0.0.1";

    std::shared_ptr<Unpack> unpack = std::make_shared<Unpack>(std::bind(&Main::Deal, this, std::placeholders::_1));

    server_ = communication_->CreateTcpServer();
    server_->SetAddr(str_ip, 8181);
    server_->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));

    client_ = communication_->CreateTcpClient();
    client_->SetAddr(str_ip, 8182, str_ip, 0);
    client_->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));

    udp_ = communication_->CreateUdp();
    udp_->SetAddr(str_ip, 8081, str_ip, 8082);
    udp_->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));

    serial_port_ = communication_->CreateSerialPort();
    // serial_port_->SetAddr(11, 9600, 8, 0, 0);
    serial_port_->SetHandleChannel(std::bind(&Unpack::Run, unpack, std::placeholders::_1));
}

void Main::Deal(std::shared_ptr<jaf::comm::IPack> pack)
{
    auto [buff, len] = pack->GetData();
    std::string str((const char*) buff, len);
    LOG_INFO() << "接收:" << str;
    pack->GetChannel()->Write(buff, len, 1000);
}
