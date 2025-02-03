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
#ifdef _WIN32

#include "udp.h"
#include "udp_channel.h"
#include <WS2tcpip.h>
#include <format>

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

Udp::Udp(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer)
    : get_completion_port_(get_completion_port)
    , timer_(timer)
{
    assert(get_completion_port_ != nullptr);
}

Udp::~Udp()
{
}

void Udp::SetAddr(const Endpoint& remote_endpoint, const Endpoint& local_endpoint)
{
    remote_endpoint_ = remote_endpoint;
    local_endpoint_  = local_endpoint;
    local_ip_        = local_endpoint_.Ip();
    local_port_      = local_endpoint_.Port();
    remote_ip_       = remote_endpoint_.Ip();
    remote_port_     = remote_endpoint_.Port();
}

void Udp::SetHandleChannel(std::function<jaf::Coroutine<void>(std::shared_ptr<jaf::comm::IUdpChannel> channel)> handle_channel)
{
    handle_channel_ = handle_channel;
}

jaf::Coroutine<RunResult> Udp::Run()
{
    assert(!run_flag_);
    run_flag_ = true;

    completion_handle_ = get_completion_port_->Get();

    SOCKET the_socket = CreateSocket();
    if (the_socket == INVALID_SOCKET)
    {
        run_flag_ = false;
        co_return error_info_;
    }

    wait_stop_.Start();
    auto run = RunSocket(the_socket);

    co_await wait_stop_.Wait();
    run_flag_ = false;

    GetChannel()->Stop();
    closesocket(the_socket);
    co_await run;

    co_return true;
}

void Udp::Stop()
{
    wait_stop_.Stop();
}

std::shared_ptr<IUdpChannel> Udp::GetChannel()
{
    std::unique_lock lock(channel_mutex_);
    assert(channel_ != nullptr);
    return channel_;
}

Coroutine<SChannelResult> Udp::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    std::shared_ptr<IChannel> channel = GetChannel();
    co_return co_await channel->Write(buff, buff_size, timeout);
}

Coroutine<SChannelResult> Udp::WriteTo(const unsigned char* buff, size_t buff_size, const Endpoint* endpoint, uint64_t timeout)
{
    std::shared_ptr<IUdpChannel> channel = GetChannel();
    co_return co_await channel->WriteTo(buff, buff_size, endpoint, timeout);
}

SOCKET Udp::CreateSocket(void)
{
    SOCKET the_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == the_socket)
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("Udp code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        return INVALID_SOCKET;
    }

    sockaddr_in& addrSrv = local_endpoint_.GetSockAddr(); //定义sockSrv发送和接收数据包的地址

    //绑定套接字, 绑定到端口
    if (SOCKET_ERROR == ::bind(the_socket, (SOCKADDR*) &addrSrv, sizeof(addrSrv)))
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("绑定UDP套接字失败,本地{}:{},code:{}",
            local_ip_, local_port_,
            dw, GetFormatMessage(dw));
        closesocket(the_socket);
        return INVALID_SOCKET;
    }

    return the_socket;
}

Coroutine<void> Udp::RunSocket(SOCKET the_socket)
{
    std::shared_ptr<UdpChannel> channel = std::make_shared<UdpChannel>(completion_handle_, the_socket, remote_endpoint_, local_endpoint_, timer_);

    {
        std::unique_lock lock(channel_mutex_);
        if (!run_flag_)
        {
            co_return;
        }
        channel_ = channel;
    }

    jaf::Coroutine<RunResult> channel_run = channel->Run();
    co_await handle_channel_(channel);
    channel->Stop();
    co_await channel_run;

    {
        std::unique_lock lock(channel_mutex_);
        channel_ = std::make_shared<EmptyUdpChannel>();
    }

    Stop();
}

} // namespace comm
} // namespace jaf

#elif defined(__linux__)
#endif