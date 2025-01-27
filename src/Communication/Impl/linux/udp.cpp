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
#elif defined(__linux__)

#include "udp.h"
#include "Impl/tool/run_with_timeout.h"
#include "udp_channel.h"
#include "util/finally.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <format>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

namespace jaf
{
namespace comm
{

Udp::Udp(IGetEpollFd* get_epoll_fd, std::shared_ptr<jaf::time::ITimer> timer)
    : get_epoll_fd_(get_epoll_fd)
    , timer_(timer)
{
    assert(get_epoll_fd != nullptr);
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

void Udp::SetHandleChannel(std::function<jaf::Coroutine<void>(std::shared_ptr<jaf::comm::IUdpChannel> channel)> handle_channel)
{
    handle_channel_ = handle_channel;
}

jaf::Coroutine<void> Udp::Run()
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    epoll_fd_ = get_epoll_fd_->Get();
    Init();

    std::shared_ptr<UdpChannel> channel = std::make_shared<UdpChannel>(socket_, epoll_fd_, remote_ip_, remote_port_, local_ip_, local_port_, timer_);

    {
        std::unique_lock lock(channel_mutex_);
        channel_ = channel;
    }

    jaf::Coroutine<void> channel_run = channel->Run();
    co_await handle_channel_(channel);
    channel->Stop();
    co_await channel_run;

    run_flag_ = false;

    close(socket_);
    socket_ = -1;

    co_return;
}

void Udp::Stop()
{
    run_flag_ = false;
    std::unique_lock lock(channel_mutex_);
    if (channel_ != nullptr)
    {
        channel_->Stop();
        channel_ = std::make_shared<EmptyUdpChannel>();
    }
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

bool Udp::Init(void)
{
    socket_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_ < 0)
    {
        int error   = errno;
        error_info_ = std::format("Failed to create a listen socket,code:{},:{}", error, strerror(error));
        return false;
    }

    sockaddr_in bind_addr     = {};
    bind_addr.sin_family      = AF_INET;
    bind_addr.sin_addr.s_addr = inet_addr(local_ip_.c_str());
    bind_addr.sin_port        = htons(local_port_);

    //绑定套接字, 绑定到端口
    if (::bind(socket_, (sockaddr*) &bind_addr, sizeof(bind_addr)) < 0)
    {
        int error   = errno;
        error_info_ = std::format("Failed to bind to the local,local {}:{},code:{},:{}",
            local_ip_, local_port_,
            error, strerror(error));
        close(socket_);
        return false;
    }

    return true;
}

} // namespace comm
} // namespace jaf

#endif