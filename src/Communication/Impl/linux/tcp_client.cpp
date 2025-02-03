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
// 2024-12-28 姜安富
#ifdef _WIN32
#elif defined(__linux__)

#include "tcp_client.h"
#include "Impl/tool/run_with_timeout.h"
#include "Log/log_head.h"
#include "tcp_channel.h"
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
struct TcpClient::ConnectResult
{
    bool success = false;
    std::string error_info_;
};

class TcpClient::ConnectAwaitable
{
    struct ConnectData
    {
        std::mutex mutex_;
        bool timeout_flag_  = false;
        bool callback_flag_ = false;

        jaf::time::STimerTask timeout_task_;
    };

public:
    ConnectAwaitable(TcpClient* tcp_client, int connect_socket)
        : tcp_client_(tcp_client)
        , connect_socket_(connect_socket)
    {
        connect_epoll_data_.call_ = [this](EpollData* data) { OnConnect(data); };
    }
    ~ConnectAwaitable() {}
    bool await_ready()
    {
        return false;
    }
    bool await_suspend(std::coroutine_handle<> co_handle)
    {
        handle_ = co_handle;

        epoll_fd_ = tcp_client_->get_epoll_fd_->Get();
        return Connect();
    }
    void await_resume()
    {
        return;
    }

private:
    bool Connect()
    {
        sockaddr_in& connect_addr = tcp_client_->remote_endpoint_.GetSockAddr();
        if (::connect(connect_socket_, (const sockaddr*) &connect_addr, sizeof(connect_addr)) < 0)
        {
            if (errno != EINPROGRESS)
            {
                LOG_WARNING() << std::format("Failed to connect ,local {}:{},remote {}:{},code:{},:{}",
                    tcp_client_->local_ip_, tcp_client_->local_port_,
                    tcp_client_->remote_ip_, tcp_client_->remote_port_,
                    errno, strerror(errno));
                return false;
            }
        }

        std::shared_ptr<ConnectData> connect_data = std::make_shared<ConnectData>();
        connect_data_                             = connect_data;
        ConnectData* p_connect_data               = connect_data.get();

        p_connect_data->timeout_task_.interval = tcp_client_->connect_timeout_;
        p_connect_data->timeout_task_.fun      = [this, connect_data](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {
            OnConnectTimeout(connect_data.get());
            connect_data->timeout_task_.fun = std::function<void(jaf::time::ETimerResultType result_type, jaf::time::STimerTask * task)>();
        };

        struct epoll_event connect_event;
        connect_event.events   = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;
        connect_event.data.ptr = &connect_epoll_data_;
        int ret                = epoll_ctl(tcp_client_->epoll_fd_, EPOLL_CTL_ADD, connect_socket_, &connect_event);
        if (ret == -1)
        {
            std::string str = std::format("epoll_ctl(): error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
            return false;
        }

        tcp_client_->timer_->StartTask(&p_connect_data->timeout_task_);
        return true;
    }

    void OnConnect(EpollData* data)
    {
        assert(connect_data_ != nullptr);
        int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, connect_socket_, nullptr);

        {
            std::unique_lock<std::mutex> lock(connect_data_->mutex_);
            if (connect_data_->timeout_flag_)
            {
                return;
            }
            connect_data_->callback_flag_ = true;
        }

        tcp_client_->timer_->StopTask(&connect_data_->timeout_task_);

        //看看 socket 是不是链接成功了
        int result;
        socklen_t result_len = sizeof(result);
        if (getsockopt(connect_socket_, SOL_SOCKET, SO_ERROR, &result, &result_len) < 0 || result != 0)
        {
            connect_result_.success     = false;
            connect_result_.error_info_ = std::format("OnConnect(): connect error: {} \t  error-msg: {}\r\n", errno, strerror(errno));

            handle_.resume();
            return;
        }

        connect_result_.success = true;
        handle_.resume();
    }

    void OnConnectTimeout(ConnectData* connect_data)
    {
        {
            std::unique_lock<std::mutex> lock(connect_data->mutex_);
            if (connect_data->callback_flag_)
            {
                return;
            }
            connect_data->timeout_flag_ = true;
        }

        connect_result_.success     = false;
        connect_result_.error_info_ = std::format("Connect timeout");

        shutdown(connect_socket_, SHUT_RDWR);
        handle_.resume();
    }

public:
    TcpClient* tcp_client_;

    std::shared_ptr<ConnectData> connect_data_;
    ConnectResult connect_result_;

    int connect_socket_ = -1;
    int epoll_fd_       = -1;

    std::string error_info_;

    EpollData connect_epoll_data_; // 连接时用的通讯数据

private:
    std::coroutine_handle<> handle_;
};

TcpClient::TcpClient(IGetEpollFd* get_epoll_fd, std::shared_ptr<jaf::time::ITimer> timer)
    : get_epoll_fd_(get_epoll_fd)
    , timer_(timer)
{
}

TcpClient::~TcpClient()
{
}

void TcpClient::SetAddr(const Endpoint& remote_endpoint, const Endpoint& local_endpoint)
{
    remote_endpoint_ = remote_endpoint;
    local_endpoint_  = local_endpoint;
    local_ip_        = local_endpoint.Ip();
    local_port_      = local_endpoint.Port();
    remote_ip_       = remote_endpoint.Ip();
    remote_port_     = remote_endpoint.Port();
}

void TcpClient::SetConnectTime(uint64_t connect_timeout, uint64_t reconnect_wait_time)
{
    connect_timeout_     = connect_timeout;
    reconnect_wait_time_ = reconnect_wait_time;
}

void TcpClient::SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel)
{
    handle_channel_ = handle_channel;
}

jaf::Coroutine<RunResult> TcpClient::Run()
{
    assert(!run_flag_);
    run_flag_ = true;
    wait_stop_.Start();

    epoll_fd_ = get_epoll_fd_->Get();

    int connect_socket = CreationSocket();
    if (connect_socket < 0)
    {
        run_flag_ = false;
        co_return error_info_;
    }

    jaf::Coroutine<void> execute = Execute(connect_socket);

    co_await wait_stop_.Wait();
    run_flag_ = false;

    GetChannel()->Stop();
    close(connect_socket);
    co_await execute;

    co_return true;
}

void TcpClient::Stop()
{
    wait_stop_.Stop();
}

std::shared_ptr<IChannel> TcpClient::GetChannel()
{
    std::unique_lock lock(channel_mutex_);
    assert(channel_ != nullptr);
    return channel_;
}

Coroutine<SChannelResult> TcpClient::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    std::shared_ptr<IChannel> channel = nullptr;
    {
        std::unique_lock<std::mutex> lock(channel_mutex_);
        channel = channel_;
    }

    co_return co_await channel->Write(buff, buff_size, timeout);
}

jaf::Coroutine<void> TcpClient::Execute(int connect_socket)
{
    ConnectAwaitable connect_awaitable(this, connect_socket);
    co_await connect_awaitable;

    if (!run_flag_)
    {
        co_return;
    }

    const ConnectResult& connect_result = connect_awaitable.connect_result_;
    if (!connect_result.success)
    {
        LOG_WARNING() << std::format("TCP连接失败,本地{}:{},远程{}:{},{}",
            local_ip_, local_port_,
            remote_ip_, remote_port_,
            connect_result.error_info_);
        co_return;
    }

    std::shared_ptr<TcpChannel> channel = std::make_shared<TcpChannel>(connect_socket, epoll_fd_, remote_endpoint_, local_endpoint_, timer_);

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
        channel_ = empty_channel_;
    }

    Stop();
    co_return;
}

int TcpClient::CreationSocket()
{
    int connect_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_socket < 0)
    {
        int error   = errno;
        error_info_ = std::format("Failed to create a connection socket,local {}:{},remote {}:{},code:{},:{}",
            local_ip_, local_port_,
            remote_ip_, remote_port_,
            error, strerror(error));
        return -1;
    }

    sockaddr_in bind_addr     = {};
    bind_addr.sin_family      = AF_INET;
    bind_addr.sin_addr.s_addr = inet_addr(local_ip_.c_str());
    bind_addr.sin_port        = htons(local_port_);

    //绑定套接字, 绑定到端口
    if (::bind(connect_socket, (sockaddr*) &bind_addr, sizeof(bind_addr)) < 0)
    {
        int error   = errno;
        error_info_ = std::format("Failed to bind to the local,local {}:{},remote {}:{},code:{},:{}",
            local_ip_, local_port_,
            remote_ip_, remote_port_,
            error, strerror(error));
        close(connect_socket);
        return -1;
    }

    //设置 connect 的 socket 为非阻塞
    int flags = fcntl(connect_socket, F_GETFL);
    fcntl(connect_socket, F_SETFL, flags | O_NONBLOCK);

    return connect_socket;
}

} // namespace comm
} // namespace jaf

#endif