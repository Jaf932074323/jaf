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

#include "tcp_server.h"
#include "Impl/tool/run_with_timeout.h"
#include "Impl/tool/stoppable_run.h"
#include "Log/log_head.h"
#include "tcp_channel.h"
#include "util/finally.h"
#include <arpa/inet.h>
#include <errno.h>
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

TcpServer::TcpServer(IGetEpollFd* get_epoll_fd, std::shared_ptr<jaf::time::ITimer> timer)
    : get_epoll_fd_(get_epoll_fd)
    , timer_(timer)
{
    assert(get_epoll_fd != nullptr);

    listen_epoll_data_.call_ = [this](EpollData* data) { OnListen(data); };
}

TcpServer::~TcpServer()
{
}

void TcpServer::SetAddr(const std::string& ip, uint16_t port)
{
    ip_   = ip;
    port_ = port;
}

void TcpServer::SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel)
{
    handle_channel_ = handle_channel;
}

void TcpServer::SetAcceptCount(size_t accept_count)
{
    assert(!run_flag_);
    accept_count_ = accept_count;
}

void TcpServer::SetMaxClientCount(size_t max_client_count)
{
    assert(!run_flag_);
    max_client_count_ = max_client_count;
}

jaf::Coroutine<void> TcpServer::Run()
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;
    control_start_stop_.Start();

    epoll_fd_ = get_epoll_fd_->Get();
    Init();

    co_await jaf::CoWaitUtilControlledStop(control_start_stop_);

    {
        std::unique_lock<std::mutex> ul(channels_mutex_);
        for (auto& [key, channel] : channels_)
        {
            channel->Stop();
        }
    }

    co_await wait_all_tasks_done_;
    assert(channels_.empty());

    close(listen_socket_);
    listen_socket_ = -1;

    co_return;
}

void TcpServer::Stop()
{
    run_flag_ = false;
    control_start_stop_.Stop();
}

Coroutine<void> TcpServer::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    std::map<std::string, std::shared_ptr<IChannel>> channels; // 当前连接的所有通道 key由IP和端口
    {
        std::unique_lock<std::mutex> ul(channels_mutex_);
        channels = channels_;
    }

    std::list<Coroutine<SChannelResult>> list_wait;

    for (auto& [key, channel] : channels)
    {
        list_wait.push_back(channel->Write(buff, buff_size, timeout));
    }
    for (Coroutine<SChannelResult>& wait : list_wait)
    {
        co_await wait;
    }
}

bool TcpServer::Init(void)
{
    listen_socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket_ < 1)
    {
        int error   = errno;
        error_info_ = std::format("Failed to create a listen socket, addr {}:{},code:{},:{}", ip_, port_, error, strerror(error));
        return false;
    }

    //设置 listen 的 socket 为非阻塞
    long on = 1L;
    if (ioctl(listen_socket_, (int) FIONBIO, (char*) &on))
    {
        perror("ioctl FIONBIO call failed\n");
        return false;
    }

    sockaddr_in server{};
    server.sin_port   = htons(port_);
    server.sin_family = AF_INET;
    inet_pton(AF_INET, ip_.c_str(), (void*) &server.sin_addr);

    if (::bind(listen_socket_, (sockaddr*) &server, sizeof(server)) < 0)
    {
        int error   = errno;
        error_info_ = std::format("Failed to bind listen socket, addr {}:{},code:{},:{}", ip_, port_, error, strerror(error));
        close(listen_socket_);
        return false;
    }

    if (listen(listen_socket_, max_client_count_) < 0)
    {
        int error   = errno;
        error_info_ = std::format("Failed to listen socket, addr {}:{},code:{},:{}", ip_, port_, error, strerror(error));
        close(listen_socket_);
        return false;
    }

    epoll_event listen_ev{};
    listen_ev.events   = EPOLLIN | EPOLLET;
    listen_ev.data.ptr = &listen_epoll_data_;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_socket_, &listen_ev) < 0)
    {
        std::string str = std::format("epoll_ctl(): error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
        close(listen_socket_);
        return false;
    }

    return true;
}

void TcpServer::OnListen(EpollData* data)
{
    if (!(data->events_ & EPOLLIN))
    {
        return;
    }

    sockaddr_in client_addr{};
    socklen_t client_addr_length = sizeof(client_addr);
    int client_socket = accept(listen_socket_, (sockaddr*) &client_addr, &client_addr_length);
    //接受来自socket连接
    if (client_socket < 0)
    {
        return;
    }

    RunSocket(client_socket);
}


jaf::Coroutine<void> TcpServer::RunSocket(int socket)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););
    
    // 获取到远端和本地的端口地址
    sockaddr_in remote_addr;
    sockaddr_in locade_addr;
    socklen_t remote_len = sizeof(sockaddr_in);
    socklen_t locade_len = sizeof(sockaddr_in);
    getsockname(listen_socket_, (struct sockaddr*) &remote_addr, &remote_len);
    getpeername(socket, (struct sockaddr*) &locade_addr, &locade_len);
    std::string remote_ip = inet_ntoa(remote_addr.sin_addr);
    uint16_t remote_port  = ntohs(remote_addr.sin_port);
    std::string local_ip  = inet_ntoa(locade_addr.sin_addr);
    uint16_t local_port   = ntohs(locade_addr.sin_port);

    std::shared_ptr<TcpChannel> channel = std::make_shared<TcpChannel>(socket, epoll_fd_, remote_ip, remote_port, local_ip, local_port, timer_);
    const std::string channel_key = std::format("{}:{}", local_ip, local_port);

    bool run_flag;
    {
        std::unique_lock<std::mutex> ul(channels_mutex_);
        run_flag = run_flag_;
        if (run_flag)
        {
            channels_.insert(std::make_pair(channel_key, channel));
        }
    }

    if (run_flag)
    {
        jaf::Coroutine<void> channel_run = channel->Run();
        co_await handle_channel_(channel);
        channel->Stop();
        co_await channel_run;

        {
            std::unique_lock<std::mutex> ul(channels_mutex_);
            channels_.erase(channel_key);
        }
    }

    close(socket);
}


} // namespace comm
} // namespace jaf

#endif