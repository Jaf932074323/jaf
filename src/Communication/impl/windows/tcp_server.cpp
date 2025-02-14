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

#include "tcp_server.h"
#include "Log/log_head.h"
#include "tcp_channel.h"
#include "util/finally.h"
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
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (LPTSTR) &lpMsgBuf,
        0, NULL);
    std::string strText = lpMsgBuf;
    LocalFree(lpMsgBuf);

    return strText;
}

static LPFN_ACCEPTEX GetAcceptEx(SOCKET a_socket)
{
    LPFN_ACCEPTEX func = nullptr;
    DWORD bytes;
    GUID guid = WSAID_ACCEPTEX;
    if (WSAIoctl(a_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &func, sizeof(func), &bytes, nullptr, nullptr) == SOCKET_ERROR)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("GetAcceptEx code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        LOG_ERROR() << str;
        return nullptr;
    }

    return func;
}

struct TcpServer::AcceptAwaitableResult
{
    SOCKET sock_{INVALID_SOCKET};
    int err_ = 0; // 错误代码
};

class TcpServer::AcceptAwaitable
{
public:
    AcceptAwaitable(SOCKET listen_socket);
    ~AcceptAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    AcceptAwaitableResult await_resume();
    void IoCallback(IOCP_DATA* pData);

private:
    SOCKET listen_socket_ = 0; // 侦听套接字
    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle_;

    AcceptAwaitableResult reslult_;
    char buf_[((sizeof (sockaddr_in) + 16) * 2)]   = {0};
    DWORD dwBytes_ = 0;
};

TcpServer::TcpServer(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer)
    : get_completion_port_(get_completion_port)
    , timer_(timer)
{
    assert(get_completion_port_ != nullptr);
}

TcpServer::~TcpServer()
{
}

void TcpServer::SetAddr(const Endpoint& endpoint)
{
    endpoint_ = endpoint;
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

jaf::Coroutine<RunResult> TcpServer::Run()
{
    assert(!run_flag_);
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
    closesocket(listen_socket_);

    {
        std::unique_lock<std::mutex> ul(channels_mutex_);
        for (auto& [key, channel] : channels_)
        {
            channel->Stop();
        }
    }

    co_await wait_all_tasks_done_;
    assert(channels_.empty());

    listen_socket_ = INVALID_SOCKET;

    co_return true;
}

void TcpServer::Stop()
{
    wait_stop_.Stop();
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
        DWORD dw        = GetLastError();
        std::string str = std::format("TcpServer code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        OutputDebugString(str.c_str());
        closesocket(listen_socket_);
        return;
    }

    if (SOCKET_ERROR == ::bind(listen_socket_, (LPSOCKADDR) &endpoint_.GetSockAddr(), sizeof(endpoint_.GetSockAddr())))
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("TcpServer code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        OutputDebugString(str.c_str());
        closesocket(listen_socket_);
        return;
    }

    CreateIoCompletionPort((HANDLE) listen_socket_, completion_handle_, 0, 0);

    if (SOCKET_ERROR == listen(listen_socket_, max_client_count_))
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("TcpServer code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        OutputDebugString(str.c_str());
        closesocket(listen_socket_);
        return;
    }
}

jaf::Coroutine<void> TcpServer::Accept()
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    AcceptAwaitable accept_awaitable(listen_socket_);

    while (run_flag_)
    {
        AcceptAwaitableResult accept_result = co_await accept_awaitable;

        if (accept_result.sock_ == INVALID_SOCKET)
        {
            if (!run_flag_)
            {
                break;
            }
            if (WAIT_TIMEOUT != accept_result.err_)
            {
                LOG_ERROR() << std::format("TcpServer Accept error,code error: {} error-msg: {}", accept_result.err_, GetFormatMessage(accept_result.err_));
            }
            continue;
        }

        RunSocket(accept_result.sock_);
    }

    co_return;
}

jaf::Coroutine<void> TcpServer::RunSocket(SOCKET socket)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    // 获取到远端和本地的端口地址
    if (setsockopt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*) &listen_socket_, sizeof(listen_socket_)) == SOCKET_ERROR)
    {
        errno = WSAGetLastError();
        shutdown(socket, SD_BOTH);
        closesocket(socket);
        socket = INVALID_SOCKET;
        co_return;
    }

    Endpoint remote_enpoint;
    Endpoint locade_enpoint;
    sockaddr_in& remote_addr = remote_enpoint.GetSockAddr();
    sockaddr_in& locade_addr = locade_enpoint.GetSockAddr();
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

    std::shared_ptr<TcpChannel> channel = std::make_shared<TcpChannel>(socket, remote_enpoint, locade_enpoint, timer_);

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
        jaf::Coroutine<RunResult> channel_run = channel->Run();
        co_await handle_channel_(channel);
        channel->Stop();
        co_await channel_run;

        {
            std::unique_lock<std::mutex> ul(channels_mutex_);
            channels_.erase(channel_key);
        }
    }
}

TcpServer::AcceptAwaitable::AcceptAwaitable(SOCKET listen_socket)
    : listen_socket_{listen_socket}
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
    static LPFN_ACCEPTEX  func_accept = GetAcceptEx(listen_socket_);
    assert(func_accept != nullptr);

    handle_ = co_handle;

    iocp_data_.call_ = std::bind(&AcceptAwaitable::IoCallback, this, std::placeholders::_1);

    reslult_.sock_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (reslult_.sock_ == INVALID_SOCKET)
    {
        errno = WSAGetLastError();
        return false;
    }

    if (!func_accept(listen_socket_, reslult_.sock_, buf_, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes_, &iocp_data_.overlapped))
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            reslult_.err_ = error;
            closesocket(reslult_.sock_);
            reslult_.sock_ = INVALID_SOCKET;
            return false;
        }
    }

    return true;
}

TcpServer::AcceptAwaitableResult TcpServer::AcceptAwaitable::await_resume()
{
    return reslult_;
}

void TcpServer::AcceptAwaitable::IoCallback(IOCP_DATA* pData)
{
    if (pData->success_ == 0)
    {
        reslult_.err_ = WSAGetLastError();

        closesocket(reslult_.sock_);
        reslult_.sock_ = INVALID_SOCKET;
    }

    handle_.resume();
}

} // namespace comm
} // namespace jaf

#elif defined(__linux__)
#endif