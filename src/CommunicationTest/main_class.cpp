#include "main_class.h"
#include "unpack.h"
#include "deal_pack.h"



void Main::Start()
{
	Init();
	iocp_->Start();	
	//server_->Run(iocp_->GetCompletionPort());
	//client_->Run(iocp_->GetCompletionPort());
	//udp_->Run(iocp_->GetCompletionPort());
	serial_port_->Run(iocp_->GetCompletionPort());
}

void Main::Stop()
{
	udp_->Stop();
	server_->Stop();
	client_->Stop();
	serial_port_->Stop();
	iocp_->Stop();
	iocp_->WaitEnd();
}

void Main::Init()
{
	iocp_->Init();


	std::shared_ptr<Unpack> unpack = std::make_shared<Unpack>();
	std::shared_ptr<DealPack> deal_pack = std::make_shared<DealPack>();
	channel_user_ = std::make_shared<jaf::comm::ChannelUser>(unpack, deal_pack);

	server_ = std::make_shared<jaf::comm::TcpServer>("10.10.10.231", 8181);
	server_->SetChannelUser(channel_user_);

	client_ = std::make_shared<jaf::comm::TcpClient>("10.10.10.231", 8183, "10.10.10.231", 8182);
	client_->SetChannelUser(channel_user_);

	udp_ = std::make_shared<jaf::comm::Udp>("10.10.10.231", 8081, "10.10.10.231", 8082);
	udp_->SetChannelUser(channel_user_);

	serial_port_ = std::make_shared<jaf::comm::SerialPort>(15, 9600, 8, 0, 0);
	serial_port_->SetChannelUser(channel_user_);
}
