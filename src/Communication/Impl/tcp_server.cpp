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
#include "tcp_server.h"
#include "Log/log_head.h"
#include "tcp_channel.h"
#include <WS2tcpip.h>
#include <assert.h>
#include <format>
#include <mswsock.h>

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw)
{
    LPTSTR lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
        nullptr,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL);
    std::string strText = lpMsgBuf;
    LocalFree(lpMsgBuf);

    return strText;
}

TcpServer::TcpServer(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer)
    : get_completion_port_(get_completion_port)
    , timer_(timer)
{
    assert(get_completion_port_ != nullptr);
}

TcpServer::~TcpServer()
{
}

void TcpServer::SetAddr(const std::string& ip, uint16_t port)
{
    ip_   = ip;
    port_ = port;
}

void TcpServer::SetChannelUser(std::shared_ptr<IChannelUser> user)
{
    user_ = user;
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

    wait_stop_.Start();

    completion_handle_ = get_completion_port_->Get();
    Init();
    for (size_t i = 0; i < accept_count_; ++i)
    {
        Accept();
    }

    co_await wait_stop_.Wait();

    run_flag_ = false;

    {
        std::unique_lock<std::mutex> ul(channels_mutex_);
        for (auto& [key, channel] : channels_)
        {
            channel->Stop();
        }
        channels_.clear();
    }

    closesocket(listen_socket_);
    listen_socket_ = INVALID_SOCKET;

    co_return;
}

void TcpServer::Stop()
{
    wait_stop_.Stop();
}

void TcpServer::Init(void)
{
    listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == listen_socket_)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("TcpServer code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        OutputDebugString(str.c_str());
        return;
    }

    ULONG NBIO = TRUE;
    if (SOCKET_ERROR == ioctlsocket(listen_socket_, FIONBIO, &NBIO))
    {
        closesocket(listen_socket_);
        DWORD dw        = GetLastError();
        std::string str = std::format("TcpServer code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        OutputDebugString(str.c_str());
        return;
    }

    SOCKADDR_IN server = {0};
    inet_pton(AF_INET, ip_.c_str(), (void*) &server.sin_addr);
    server.sin_family = AF_INET;
    server.sin_port   = htons(port_);

    if (SOCKET_ERROR == ::bind(listen_socket_, (LPSOCKADDR) &server, sizeof(server)))
    {
        closesocket(listen_socket_);
        DWORD dw        = GetLastError();
        std::string str = std::format("TcpServer code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        OutputDebugString(str.c_str());
        return;
    }

    CreateIoCompletionPort((HANDLE) listen_socket_, completion_handle_, 0, 0);

    if (SOCKET_ERROR == listen(listen_socket_, max_client_count_))
    {
        closesocket(listen_socket_);
        DWORD dw        = GetLastError();
        std::string str = std::format("TcpServer code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        OutputDebugString(str.c_str());
        return;
    }
}

jaf::Coroutine<void> TcpServer::Accept()
{
    while (run_flag_)
    {
        SOCKET socket = co_await AcceptAwaitable(this, listen_socket_);
        if (socket == INVALID_SOCKET)
        {
            continue;
        }

        RunSocket(socket);
    }

    co_return;
}

jaf::Coroutine<void> TcpServer::RunSocket(SOCKET socket)
{
    // 获取到远端和本地的端口地址
    if (setsockopt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*) &listen_socket_, sizeof(listen_socket_)) == SOCKET_ERROR)
    {
        errno = WSAGetLastError();
        shutdown(socket, SD_BOTH);
        closesocket(socket);
        socket = INVALID_SOCKET;
        co_return;
    }
    sockaddr_in remote_addr;
    sockaddr_in locade_addr;
    int remote_len = sizeof(sockaddr_in);
    int locade_len = sizeof(sockaddr_in);
    getsockname(listen_socket_, (struct sockaddr*) &remote_addr, &remote_len);
    getpeername(socket, (struct sockaddr*) &locade_addr, &locade_len);
    std::string remote_ip = inet_ntoa(remote_addr.sin_addr);
    uint16_t remote_port  = ntohs(remote_addr.sin_port);
    std::string local_ip  = inet_ntoa(locade_addr.sin_addr);
    uint16_t local_port   = ntohs(locade_addr.sin_port);

    const std::string channel_key = std::format("{}:{}", local_ip, local_port);

    if (CreateIoCompletionPort((HANDLE) socket, completion_handle_, 0, 0) == 0)
    {
        DWORD dw            = GetLastError();
        std::string str_err = std::format("绑定完成端口失败,本地{}:{},远程{}:{},code:{},{}",
            local_ip, local_port,
            remote_ip, remote_port,
            dw, GetFormatMessage(dw));
        LOG_ERROR() << str_err;

        co_return;
    }

    std::shared_ptr<TcpChannel> channel = std::make_shared<TcpChannel>(socket, remote_ip, remote_port, local_ip, local_port, timer_);

    {
        std::unique_lock<std::mutex> ul(channels_mutex_);
        channels_.insert(std::make_pair(channel_key, channel));
    }

    if (co_await channel->Start())
    {
        co_await user_->Access(channel);
        channel->Stop();
    }

    {
        std::unique_lock<std::mutex> ul(channels_mutex_);
        channels_.erase(channel_key);
    }

    CancelIo((HANDLE) socket);
    closesocket(socket);
}

TcpServer::AcceptAwaitable::AcceptAwaitable(TcpServer* server, SOCKET listen_socket)
    : server{server}, listen_socket_{listen_socket}
{
}

TcpServer::AcceptAwaitable::~AcceptAwaitable()
{
}

bool TcpServer::AcceptAwaitable::await_ready()
{
    return false;
}

bool TcpServer::AcceptAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    handle_ = co_handle;

    iocp_data_.call_ = std::bind(&AcceptAwaitable::IoCallback, this, std::placeholders::_1);

    sock_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sock_ == INVALID_SOCKET)
    {
        errno = WSAGetLastError();
        return false;
    }

    bool accept_result = AcceptEx(listen_socket_, sock_, buf_, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes_, &iocp_data_.overlapped);

    if (!accept_result && WSAGetLastError() != ERROR_IO_PENDING)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("TcpServer code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        OutputDebugString(str.c_str());
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
        return false;
    }

    return true;
}

SOCKET TcpServer::AcceptAwaitable::await_resume()
{
    return sock_;
}

void TcpServer::AcceptAwaitable::IoCallback(IOCP_DATA* pData)
{
    if (pData->success_ == 0)
    {
        DWORD dw = GetLastError();
        if (WAIT_TIMEOUT != dw)
        {
            std::string str = std::format("TcpServer code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
            OutputDebugString(str.c_str());
        }

        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }

    handle_.resume();
}

} // namespace comm
} // namespace jaf