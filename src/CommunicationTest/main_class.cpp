#include "main_class.h"
#include "deal_pack.h"
#include "unpack.h"
#include "util/simple_thread_pool.h"

Main::Main()
    : iocp_(std::make_shared<jaf::comm::Iocp>(std::make_shared<jaf::SimpleThreadPool>(2)))
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

    await_stop_.Start();

    jaf::Coroutine<void> iocp_run = iocp_->Run();
    coroutines.push_back(server_->Run());
    coroutines.push_back(client_->Run());
    coroutines.push_back(udp_->Run());
    coroutines.push_back(serial_port_->Run());

    co_await await_stop_.Wait();

    server_->Stop();
    client_->Stop();
    udp_->Stop();
    serial_port_->Stop();
    for (auto& coroutine : coroutines)
    {
        co_await coroutine;
    }

    iocp_->Stop();
    co_await iocp_run;
    wait_finish_latch_.CountDown();
}

void Main::Stop()
{
    await_stop_.Stop();
}

void Main::WaitFinish()
{
    wait_finish_latch_.Wait();
}

void Main::Init()
{
    iocp_->Init();

    std::string str_ip = "127.0.0.1";

    std::shared_ptr<Unpack> unpack      = std::make_shared<Unpack>();
    std::shared_ptr<DealPack> deal_pack = std::make_shared<DealPack>();
    channel_user_                       = std::make_shared<jaf::comm::ChannelUser>(unpack, deal_pack);

    server_ = iocp_->CreateTcpServer();
    server_->SetAddr(str_ip, 8181);
    server_->SetChannelUser(channel_user_);

    client_ = iocp_->CreateTcpClient();
    client_->SetAddr(str_ip, 8182, str_ip, 0);
    client_->SetChannelUser(channel_user_);

    udp_ = iocp_->CreateUdp();
    udp_->SetAddr(str_ip, 8081, str_ip, 8082);
    udp_->SetChannelUser(channel_user_);

    serial_port_ = iocp_->CreateSerialPort();
    serial_port_->SetAddr(11, 9600, 8, 0, 0);
    serial_port_->SetChannelUser(channel_user_);
}
