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

#include "udp_channel.h"
#include "Impl/tool/run_with_timeout.h"
#include "Log/log_head.h"
#include "head.h"
#include "util/co_wait_util_controlled_stop.h"
#include "util/finally.h"
#include <assert.h>
#include <errno.h>
#include <format>
#include <memory.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace jaf
{
namespace comm
{

UdpChannel::UdpChannel(int socket, int epoll_fd, std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port, std::shared_ptr<jaf::time::ITimer> timer)
    : socket_(socket)
    , epoll_fd_(epoll_fd)
    , remote_ip_(remote_ip)
    , remote_port_(remote_port)
    , local_ip_(local_ip)
    , local_port_(local_port)
    , remote_addr_(remote_ip, remote_port)
    , timer_(timer)
{
    close_flag_ = false;
}

UdpChannel::~UdpChannel() {}

Coroutine<void> UdpChannel::Run()
{
    stop_flag_ = false;

    epoll_data_.call_ = [this](EpollData* pData) { OnEpoll(pData); };

    struct epoll_event ev;
    ev.events   = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;
    ev.data.ptr = &epoll_data_;
    int ret     = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_, &ev);
    if (ret == -1)
    {
        close(socket_);
        std::string str = std::format("epoll_ctl(): error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
        stop_flag_      = true;
        co_return;
    }

    read_helper_.Start(socket_);
    write_helper_.Start(socket_);

    control_start_stop_.Start();
    co_await jaf::CoWaitUtilControlledStop(control_start_stop_);
    co_await wait_all_tasks_done_;
}

void UdpChannel::Stop()
{
    stop_flag_ = true;
    close(socket_);
    read_helper_.Stop();
    write_helper_.Stop();
    control_start_stop_.Stop();
}

Coroutine<SChannelResult> UdpChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    struct ReadCommunData : public CommunData
    {
        virtual void DoOperate(int file)
        {
            result.len = ::read(file, operate_buf_, need_len_);
        }
    };
    std::shared_ptr<ReadCommunData> read_data = std::make_shared<ReadCommunData>();
    read_data->need_len_                      = buff_size;
    read_data->operate_buf_                   = buff;

    RWAwaitable read_awaitable(read_helper_, timer_, read_data, timeout);
    co_await read_awaitable;

    if (read_data->result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        co_return read_data->result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    co_return read_data->result;
}

Coroutine<SChannelResult> UdpChannel::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    struct WriteCommunData : public CommunData
    {
        WriteCommunData(Addr& remote_addr)
            : remote_addr_(remote_addr)
        {
        }
        // 执行通讯功能
        virtual void DoOperate(int file)
        {
            result.len = ::sendto(file, operate_buf_, need_len_, 0, (sockaddr*) &remote_addr_.client_addr_, addr_len_);
        }

        Addr& remote_addr_;        
        socklen_t addr_len_ = sizeof(remote_addr_);
    };
    std::shared_ptr<WriteCommunData> write_data = std::make_shared<WriteCommunData>(remote_addr_);
    write_data->need_len_                       = buff_size;
    write_data->operate_buf_                    = (unsigned char*) buff;

    RWAwaitable write_awaitable(write_helper_, timer_, write_data, timeout);
    co_await write_awaitable;

    if (write_data->result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        co_return write_data->result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    co_return write_data->result;
}

Coroutine<SChannelResult> UdpChannel::ReadFrom(unsigned char* buff, size_t buff_size, Addr* addr, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    struct ReadCommunData : public CommunData
    {
        virtual void DoOperate(int file)
        {
            result.len = ::recvfrom(file, operate_buf_, need_len_, MSG_DONTWAIT, (sockaddr*) &client_addr_, &addr_len_);
        }

        struct sockaddr_in client_addr_;
        socklen_t addr_len_ = sizeof(client_addr_);
    };
    std::shared_ptr<ReadCommunData> read_data = std::make_shared<ReadCommunData>();
    read_data->need_len_                      = buff_size;
    read_data->operate_buf_                   = buff;

    RWAwaitable read_awaitable(read_helper_, timer_, read_data, timeout);
    co_await read_awaitable;

    addr->client_addr_ = read_data->client_addr_;

    if (read_data->result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        co_return read_data->result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    co_return read_data->result;
}

Coroutine<SChannelResult> UdpChannel::WriteTo(const unsigned char* buff, size_t buff_size, Addr* addr, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    struct WriteCommunData : public CommunData
    {
        // 执行通讯功能
        virtual void DoOperate(int file)
        {
            result.len = ::sendto(file, operate_buf_, need_len_, 0, (sockaddr*) &client_addr_, addr_len_);
        }

        struct sockaddr_in client_addr_;
        socklen_t addr_len_ = sizeof(client_addr_);
    };
    std::shared_ptr<WriteCommunData> write_data = std::make_shared<WriteCommunData>();
    write_data->client_addr_                    = addr->client_addr_;
    write_data->need_len_                       = buff_size;
    write_data->operate_buf_                    = (unsigned char*) buff;

    RWAwaitable write_awaitable(write_helper_, timer_, write_data, timeout);
    co_await write_awaitable;

    if (write_data->result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        co_return write_data->result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    co_return write_data->result;
}

void UdpChannel::OnEpoll(EpollData* data)
{
    if (data->events_ & EPOLLIN)
    {
        char buff[1];
        if (0 == ::recv(socket_, buff, 1, MSG_PEEK))
        {
            finish_reason_ = std::format("Connection closed");
            Stop();
            return;
        }
        read_helper_.OnOperate(data);
    }

    if (data->events_ & EPOLLOUT)
    {
        write_helper_.OnOperate(data);
    }
}

} // namespace comm
} // namespace jaf

#endif