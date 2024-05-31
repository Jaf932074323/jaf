#include "udp.h"
#include "udp_channel.h"
#include <WS2tcpip.h>
#include <format>

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

Udp::Udp(std::string local_ip, uint16_t local_port, std::string remote_ip, uint16_t remote_port, std::shared_ptr<jaf::time::ITimer> timer)
    : local_ip_(local_ip)
    , local_port_(local_port)
    , remote_ip_(remote_ip)
    , remote_port_(remote_port)
    , timer_(timer)
{
}

Udp::~Udp()
{
}

void Udp::SetChannelUser(std::shared_ptr<IChannelUser> user)
{
    user_ = user;
}

jaf::Coroutine<void> Udp::Run(HANDLE completion_handle)
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    completion_handle_ = completion_handle;
    Init();

    std::shared_ptr<UdpChannel> channel = std::make_shared<UdpChannel>(completion_handle_, socket_, remote_ip_, remote_port_, local_ip_, local_port_, timer_);

    {
        std::unique_lock lock(channel_mutex_);
        channel_ = channel;
    }

    if (co_await channel->Start())
    {
        co_await user_->Access(channel);
        channel->Stop();
    }

    run_flag_ = false;

    CancelIo((HANDLE) socket_);
    closesocket(socket_);
    socket_ = INVALID_SOCKET;

    co_return;
}

void Udp::Stop()
{
    run_flag_ = false;
    std::unique_lock lock(channel_mutex_);
    if (channel_ != nullptr)
    {
        channel_->Stop();
        channel_ = nullptr;
    }
}

void Udp::Init(void)
{
    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == socket_)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("Udp code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        OutputDebugString(str.c_str());
        return;
    }

    sockaddr_in addrSrv = {}; //定义sockSrv发送和接收数据包的地址
    inet_pton(AF_INET, local_ip_.c_str(), (void*) &addrSrv.sin_addr);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port   = htons(local_port_);

    //绑定套接字, 绑定到端口
    ::bind(socket_, (SOCKADDR*) &addrSrv, sizeof(addrSrv)); //会返回一个SOCKET_ERROR
}

} // namespace comm
} // namespace jaf