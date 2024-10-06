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

void Udp::SetAddr(const std::string& local_ip, uint16_t local_port, const std::string& remote_ip, uint16_t remote_port)
{
    local_ip_    = local_ip;
    local_port_  = local_port;
    remote_ip_   = remote_ip;
    remote_port_ = remote_port;
}

void Udp::SetUnpack(std::shared_ptr<IUnpack> unpack)
{
    unpack_ = unpack;
}

jaf::Coroutine<void> Udp::Run()
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    completion_handle_ = get_completion_port_->Get();
    Init();

    std::shared_ptr<UdpChannel> channel = std::make_shared<UdpChannel>(completion_handle_, socket_, remote_ip_, remote_port_, local_ip_, local_port_, timer_);

    {
        std::unique_lock lock(channel_mutex_);
        channel_ = channel;
    }

    jaf::Coroutine<void> channel_run = channel->Run();
    co_await unpack_->Run(channel);
    channel->Stop();
    co_await channel_run;

    run_flag_ = false;

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
        channel_ = std::make_shared<EmptyChannel>();
    }
}

std::shared_ptr<IChannel> Udp::GetChannel()
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