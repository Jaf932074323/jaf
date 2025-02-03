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

#include "udp_channel.h"
#include "Impl/tool/run_with_timeout.h"
#include "Log/log_head.h"
#include "channel_read_write_helper.h"
#include "util/co_wait_util_controlled_stop.h"
#include <WS2tcpip.h>
#include <assert.h>
#include <format>
#include <mswsock.h>
#include <mutex>

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

UdpChannel::UdpChannel(HANDLE completion_handle, SOCKET socket, const Endpoint& remote_endpoint, const Endpoint& local_endpoint, std::shared_ptr<jaf::time::ITimer> timer)
    : completion_handle_(completion_handle)
    , socket_(socket)
    , remote_endpoint_(remote_endpoint)
    , local_endpoint_(local_endpoint)
    , timer_(timer)
{
}

UdpChannel::~UdpChannel()
{
}

Coroutine<void> UdpChannel::Run()
{
    wait_stop_.Start();
    stop_flag_ = false;
    if (CreateIoCompletionPort((HANDLE) socket_, completion_handle_, 0, 0) == 0)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("Communication code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        LOG_ERROR() << str;
        co_return;
    }

    co_await wait_stop_.Wait();
    closesocket(socket_);
    co_await wait_all_tasks_done_;

    co_return;
}

void UdpChannel::Stop()
{
    stop_flag_ = true;
    wait_stop_.Stop();
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
        SOCKET socket_ = 0; // 收发数据的套接字
        WSABUF wsbuffer_{};
        // 执行通讯功能
        bool DoOperate()
        {
            DWORD flags = 0;
            if (SOCKET_ERROR == WSARecv(socket_, &wsbuffer_, 1, nullptr, &flags, &iocp_data_.overlapped, NULL))
            {
                result.code_ = WSAGetLastError();
                if (result.code_ != ERROR_IO_PENDING)
                {
                    result.error = GetFormatMessage(result.code_);
                    return false;
                }
            }
            return true;
        }

        // 停止通讯功能
        void StopOperate()
        {
            CancelIoEx((HANDLE) socket_, &iocp_data_.overlapped);
        }
    };

    std::shared_ptr<ReadCommunData> commun_data = std::make_shared<ReadCommunData>();
    commun_data->socket_                        = socket_;
    commun_data->wsbuffer_.len                  = (ULONG) buff_size;
    commun_data->wsbuffer_.buf                  = (char*) buff;
    RWAwaitable read_awaitable(timer_, commun_data, timeout);
    co_await read_awaitable;

    SChannelResult& channel_result = commun_data->result;

    if (channel_result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        if (channel_result.len == 0)
        {
            channel_result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            channel_result.error = "The connection has been disconnected";
        }
        co_return channel_result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    if (channel_result.state == SChannelResult::EState::CRS_TIMEOUT)
    {
        co_return channel_result;
    }

    Stop();
    co_return channel_result;
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
        SOCKET socket_ = 0; // 收发数据的套接字
        sockaddr_in send_addr_;
        WSABUF wsbuffer_{};
        // 执行通讯功能
        bool DoOperate()
        {
            DWORD flags = 0;

            if (SOCKET_ERROR == WSASendTo(socket_, &wsbuffer_, 1, nullptr, flags, (SOCKADDR*) &send_addr_, sizeof(send_addr_), &iocp_data_.overlapped, NULL))
            {
                int error = WSAGetLastError();
                if (error != ERROR_IO_PENDING)
                {
                    return false;
                }
            }

            return true;
        }

        // 停止通讯功能
        void StopOperate()
        {
            CancelIoEx((HANDLE) socket_, &iocp_data_.overlapped);
        }
    };

    std::shared_ptr<WriteCommunData> commun_data = std::make_shared<WriteCommunData>();
    commun_data->send_addr_                      = remote_endpoint_.GetSockAddr();
    commun_data->socket_                         = socket_;
    commun_data->wsbuffer_.len                   = (ULONG) buff_size;
    commun_data->wsbuffer_.buf                   = (char*) buff;
    RWAwaitable write_awaitable(timer_, commun_data, timeout);
    co_await write_awaitable;

    SChannelResult& channel_result = commun_data->result;

    if (channel_result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        if (channel_result.len == 0)
        {
            channel_result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            channel_result.error = "The connection has been disconnected";
        }
        co_return channel_result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    if (channel_result.state == SChannelResult::EState::CRS_TIMEOUT)
    {
        co_return channel_result;
    }

    Stop();
    co_return channel_result;
}

Coroutine<SChannelResult> UdpChannel::ReadFrom(unsigned char* buff, size_t buff_size, Endpoint* endpoint, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    struct ReadCommunData : public CommunData
    {
        SOCKET socket_ = 0; // 收发数据的套接字
        sockaddr_in remote_addr_;
        int remote_addr_size_ = sizeof(remote_addr_);
        WSABUF wsbuffer_{};
        // 执行通讯功能
        bool DoOperate()
        {
            DWORD flags = 0;
            if (SOCKET_ERROR == WSARecvFrom(socket_, &wsbuffer_, 1, nullptr, &flags, (SOCKADDR*) &remote_addr_, &remote_addr_size_, &iocp_data_.overlapped, NULL))
            {
                result.code_ = WSAGetLastError();
                if (result.code_ != ERROR_IO_PENDING)
                {
                    result.error = GetFormatMessage(result.code_);
                    return false;
                }
            }
            return true;
        }

        // 停止通讯功能
        void StopOperate()
        {
            CancelIoEx((HANDLE) socket_, &iocp_data_.overlapped);
        }
    };

    std::shared_ptr<ReadCommunData> commun_data = std::make_shared<ReadCommunData>();
    commun_data->socket_                        = socket_;
    commun_data->wsbuffer_.len                  = (ULONG) buff_size;
    commun_data->wsbuffer_.buf                  = (char*) buff;
    RWAwaitable read_awaitable(timer_, commun_data, timeout);
    co_await read_awaitable;

    SChannelResult& channel_result = commun_data->result;

    if (channel_result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        if (channel_result.len == 0)
        {
            channel_result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            channel_result.error = "The connection has been disconnected";
        }

        endpoint->Set(commun_data->remote_addr_);

        co_return channel_result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    if (channel_result.state == SChannelResult::EState::CRS_TIMEOUT)
    {
        co_return channel_result;
    }

    Stop();
    co_return channel_result;
}

Coroutine<SChannelResult> UdpChannel::WriteTo(const unsigned char* buff, size_t buff_size, const Endpoint* endpoint, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    struct WriteCommunData : public CommunData
    {
        SOCKET socket_ = 0; // 收发数据的套接字
        sockaddr_in remote_addr_;
        WSABUF wsbuffer_{};
        // 执行通讯功能
        bool DoOperate()
        {
            DWORD flags = 0;

            if (SOCKET_ERROR == WSASendTo(socket_, &wsbuffer_, 1, nullptr, flags, (SOCKADDR*) &remote_addr_, sizeof(remote_addr_), &iocp_data_.overlapped, NULL))
            {
                int error = WSAGetLastError();
                if (error != ERROR_IO_PENDING)
                {
                    return false;
                }
            }

            return true;
        }

        // 停止通讯功能
        void StopOperate()
        {
            CancelIoEx((HANDLE) socket_, &iocp_data_.overlapped);
        }
    };

    std::shared_ptr<WriteCommunData> commun_data = std::make_shared<WriteCommunData>();
    commun_data->remote_addr_                    = endpoint->GetSockAddr();
    commun_data->socket_                         = socket_;
    commun_data->wsbuffer_.len                   = (ULONG) buff_size;
    commun_data->wsbuffer_.buf                   = (char*) buff;
    RWAwaitable write_awaitable(timer_, commun_data, timeout);
    co_await write_awaitable;

    SChannelResult& channel_result = commun_data->result;

    if (channel_result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        if (channel_result.len == 0)
        {
            channel_result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            channel_result.error = "The connection has been disconnected";
        }
        co_return channel_result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    if (channel_result.state == SChannelResult::EState::CRS_TIMEOUT)
    {
        co_return channel_result;
    }

    Stop();
    co_return channel_result;
}

} // namespace comm
} // namespace jaf

#elif defined(__linux__)
#endif