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

#include "tcp_client.h"
#include "Impl/tool/run_with_timeout.h"
#include "Log/log_head.h"
#include "channel_read_write_helper.h"
#include "tcp_channel.h"
#include "util/finally.h"
#include <WS2tcpip.h>
#include <format>
#include <mswsock.h>
#include <winsock2.h>

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

static LPFN_CONNECTEX GetConnectEx(SOCKET a_socket)
{
    LPFN_CONNECTEX func = nullptr;
    DWORD bytes;
    GUID guid = WSAID_CONNECTEX;
    if (WSAIoctl(a_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &func, sizeof(func), &bytes, nullptr, nullptr) == SOCKET_ERROR)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("GetConnectEx code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        LOG_ERROR() << str;
        return nullptr;
    }

    return func;
}

TcpClient::TcpClient(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer)
    : get_completion_port_(get_completion_port)
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

jaf::Coroutine<void> TcpClient::Run()
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    completion_handle_ = get_completion_port_->Get();

    SOCKET connect_socket = CreationSocket();
    if (connect_socket == INVALID_SOCKET)
    {
        LOG_ERROR() << error_info_;
        co_return;
    }

    wait_stop_.Start();
    jaf::Coroutine<void> execute = Execute(connect_socket);

    co_await wait_stop_.Wait();

    GetChannel()->Stop();
    closesocket(connect_socket);
    co_await execute;
}

void TcpClient::Stop()
{
    run_flag_ = false;
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

jaf::Coroutine<void> TcpClient::Execute(SOCKET connect_socket)
{
    struct ConnectCommunData : public CommunData
    {
        SOCKET connect_socket_; // 收发数据的套接字
        sockaddr_in connect_addr_;

        // 执行通讯功能
        bool DoOperate()
        {
            static LPFN_CONNECTEX func_connect = GetConnectEx(connect_socket_);

            DWORD bytes;
            bool connect_result = func_connect(connect_socket_, (const sockaddr*) &connect_addr_, sizeof(connect_addr_), nullptr, 0, &bytes, &iocp_data_.overlapped);

            if (!connect_result && WSAGetLastError() != ERROR_IO_PENDING)
            {
                result.code_ = GetLastError();
                result.error = std::format("code:{},:{}", result.code_, GetFormatMessage(result.code_));
                return false;
            }
            return true;
        }

        // 停止通讯功能
        void StopOperate()
        {
            CancelIoEx((HANDLE) connect_socket_, &iocp_data_.overlapped);
        }
    };

    std::shared_ptr<ConnectCommunData> commun_data = std::make_shared<ConnectCommunData>();
    commun_data->connect_addr_                     = remote_endpoint_.GetSockAddr();
    commun_data->connect_socket_                   = connect_socket;
    RWAwaitable connect_awaitable(timer_, commun_data, connect_timeout_);
    co_await connect_awaitable;

    if (!run_flag_)
    {
        co_return;
    }

    const SChannelResult& result     = commun_data->result;
    if (result.state != SChannelResult::EState::CRS_SUCCESS)
    {
        LOG_WARNING() << std::format("TCP连接失败,本地{}:{},远程{}:{},{}",
            local_ip_, local_port_,
            remote_ip_, remote_port_,
            result.error);
        co_return;
    }

    std::shared_ptr<TcpChannel> channel = std::make_shared<TcpChannel>(connect_socket, remote_endpoint_, local_endpoint_, timer_);

    {
        std::unique_lock lock(channel_mutex_);
        if (!run_flag_)
        {
            co_return;
        }
        channel_ = channel;
    }

    jaf::Coroutine<void> channel_run = channel->Run();
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

SOCKET TcpClient::CreationSocket()
{
    SOCKET connect_socket = INVALID_SOCKET; // 连接套接字
    connect_socket        = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == connect_socket)
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("TCP连接创建套接字失败,本地{}:{},远程{}:{},code:{},:{}",
            local_ip_, local_port_,
            remote_ip_, remote_port_,
            dw, GetFormatMessage(dw));
        return false;
    }

    sockaddr_in bind_addr = {};
    inet_pton(AF_INET, local_ip_.c_str(), (void*) &bind_addr.sin_addr);
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port   = htons(local_port_);

    //绑定套接字, 绑定到端口
    if (SOCKET_ERROR == ::bind(connect_socket, (SOCKADDR*) &bind_addr, sizeof(bind_addr)))
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("TCP连接创建套接字失败,本地{}:{},远程{}:{},code:{},:{}",
            local_ip_, local_port_,
            remote_ip_, remote_port_,
            dw, GetFormatMessage(dw));
        closesocket(connect_socket);
        connect_socket = INVALID_SOCKET;
        return false;
    }

    if (CreateIoCompletionPort((HANDLE) connect_socket, completion_handle_, 0, 0) == 0)
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("TCP连接创建套接字失败,本地{}:{},远程{}:{},code:{},:{}",
            local_ip_, local_port_,
            remote_ip_, remote_port_,
            dw, GetFormatMessage(dw));
        closesocket(connect_socket);
        connect_socket = INVALID_SOCKET;
        return false;
    }

    return connect_socket;
}

} // namespace comm
} // namespace jaf

#elif defined(__linux__)
#endif