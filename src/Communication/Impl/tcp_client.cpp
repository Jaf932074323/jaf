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
#include "tcp_client.h"
#include "Log/log_head.h"
#include "impl/tool/run_with_timeout.h"
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

struct TcpClient::ConnectResult
{
    bool success = false;
    std::string error_info_;
};

class TcpClient::ConnectAwaitable
{
public:
    ConnectAwaitable(TcpClient* client, SOCKET connect_socket);
    ~ConnectAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    ConnectResult await_resume();
    void IoCallback(IOCP_DATA* pData);
    void Stop();

private:
    TcpClient* client_ = nullptr;
    std::string error_info_;
    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle_;
    SOCKET connect_socket_;

    bool success_ = false;

    char buf_[1]   = {0};
    DWORD dwBytes_ = 0;

    std::mutex mutex_;           // 超时使用的同步
    bool callback_flag_ = false; // 已经回调标记
};

TcpClient::TcpClient(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer)
    : get_completion_port_(get_completion_port)
    , timer_(timer)
    , await_time_(timer_)
{
}

TcpClient::~TcpClient()
{
}

void TcpClient::SetAddr(const std::string& remote_ip, uint16_t remote_port, const std::string& local_ip, uint16_t local_port)
{
    local_ip_    = local_ip;
    local_port_  = local_port;
    remote_ip_   = remote_ip;
    remote_port_ = remote_port;
}

void TcpClient::SetConnectTime(uint64_t connect_timeout, uint64_t reconnect_wait_time)
{
    connect_timeout_     = connect_timeout;
    reconnect_wait_time_ = reconnect_wait_time;
}

void TcpClient::SetChannelUser(std::shared_ptr<IChannelUser> user)
{
    user_ = user;
}

jaf::Coroutine<void> TcpClient::Run()
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    wait_stop_.Start();
    completion_handle_ = get_completion_port_->Get();
    await_time_.Start();

    Init();

    jaf::Coroutine<void> execute = Execute();

    co_await wait_stop_.Wait();

    run_flag_ = false;

    {
        std::unique_lock<std::mutex> lock(channel_mutex_);
        if (channel_)
        {
            channel_->Stop();
        }
    }

    await_time_.Stop();

    co_await execute;

    co_return;
}

void TcpClient::Stop()
{
    wait_stop_.Stop();
}

void TcpClient::Init(void)
{
}

jaf::Coroutine<void> TcpClient::Execute()
{
    SOCKET connect_socket = INVALID_SOCKET; // 连接套接字

    while (run_flag_)
    {
        connect_socket = CreationSocket();
        if (connect_socket == INVALID_SOCKET)
        {
            LOG_ERROR() << error_info_;
            co_return;
        }
        FINALLY(closesocket(connect_socket); connect_socket = INVALID_SOCKET;);

        ConnectAwaitable connect_awaitable(this, connect_socket);
        RunWithTimeout run_with_timeout(await_time_, connect_awaitable, connect_timeout_);
        co_await run_with_timeout.Run();

        const auto& result = run_with_timeout.Result();
        if (!run_flag_)
        {
            co_return;
        }
        if (!result.success)
        {
            LOG_WARNING() << std::format("TCP连接失败,本地{}:{},远程{}:{},{}",
                local_ip_, local_port_,
                remote_ip_, remote_port_,
                result.error_info_);

            jaf::time::CoAwaitTime::WaitHandle wait_handle;
            co_await await_time_.Wait(wait_handle, reconnect_wait_time_);
            continue;
        }


        std::shared_ptr<TcpChannel> channel = std::make_shared<TcpChannel>(connect_socket, remote_ip_, remote_port_, local_ip_, local_port_, timer_);

        {
            std::unique_lock lock(channel_mutex_);
            channel_ = channel;
        }

        if (co_await channel->Start())
        {
            co_await user_->Access(channel);
            channel->Stop();
        }
    }

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

TcpClient::ConnectAwaitable::ConnectAwaitable(TcpClient* client, SOCKET connect_socket)
    : client_(client)
    , connect_socket_(connect_socket)
{
}

TcpClient::ConnectAwaitable::~ConnectAwaitable()
{
}

bool TcpClient::ConnectAwaitable::await_ready()
{
    return false;
}

bool TcpClient::ConnectAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    static LPFN_CONNECTEX func_connect = GetConnectEx(connect_socket_);

    handle_          = co_handle;
    iocp_data_.call_ = std::bind(&ConnectAwaitable::IoCallback, this, std::placeholders::_1);

    sockaddr_in connect_addr = {0};
    inet_pton(AF_INET, client_->remote_ip_.c_str(), (void*) &connect_addr.sin_addr);
    connect_addr.sin_family = AF_INET;
    connect_addr.sin_port   = htons(client_->remote_port_);
    DWORD bytes;
    bool connect_result = func_connect(connect_socket_, (const sockaddr*) &connect_addr, sizeof(connect_addr), nullptr, 0, &bytes, &iocp_data_.overlapped);

    if (!connect_result && WSAGetLastError() != ERROR_IO_PENDING)
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("code:{},:{}", dw, GetFormatMessage(dw));
        return false;
    }

    return true;
}

TcpClient::ConnectResult TcpClient::ConnectAwaitable::await_resume()
{
    return ConnectResult{.success = success_, .error_info_ = error_info_};
}

void TcpClient::ConnectAwaitable::IoCallback(IOCP_DATA* pData)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        callback_flag_ = true;
    }

    if (pData->success_ == 0)
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("code:{},:{}", dw, GetFormatMessage(dw));
        success_    = false;
    }
    else
    {
        success_ = true;
    }

    handle_.resume();
}

void TcpClient::ConnectAwaitable::Stop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback_flag_)
    {
        return;
    }
    callback_flag_ = true;
    CancelIoEx((HANDLE) connect_socket_, &iocp_data_.overlapped);
}

} // namespace comm
} // namespace jaf