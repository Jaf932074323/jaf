#include "tcp_client.h"
#include "Impl/co_notify.h"
#include "Log/log_head.h"
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
    bool success   = false;
    SOCKET socket_ = INVALID_SOCKET; // 连接套接字
    std::string error_info_;
};

class TcpClient::ConnectAwaitable
{
public:
    ConnectAwaitable(TcpClient* client);
    ~ConnectAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    ConnectResult await_resume();
    void IoCallback(IOCP_DATA* pData);

private:
    TcpClient* client_ = nullptr;
    SOCKET socket_     = INVALID_SOCKET; // 连接套接字
    std::string error_info_;
    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle_;

    char buf_[1]   = {0};
    DWORD dwBytes_ = 0;
};

TcpClient::TcpClient(std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port, std::shared_ptr<jaf::time::Timer> timer)
    : local_ip_(local_ip)
    , local_port_(local_port)
    , remote_ip_(remote_ip)
    , remote_port_(remote_port)
    , timer_(timer == nullptr ? jaf::time::CommonTimer::Timer() : timer)
{
}

TcpClient::~TcpClient()
{
}

void TcpClient::SetReconnectWaitTime(uint64_t reconnect_wait_time)
{
    reconnect_wait_time_ = reconnect_wait_time;
}

void TcpClient::SetChannelUser(std::shared_ptr<IChannelUser> user)
{
    user_ = user;
}

jaf::Coroutine<void> TcpClient::Run(HANDLE completion_handle)
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    completion_handle_ = completion_handle;
    Init();

    co_await Run();

    co_return;
}

void TcpClient::Stop()
{
}

void TcpClient::Init(void)
{
}

jaf::Coroutine<void> TcpClient::Run()
{
    jaf::time::CoNotify notify(timer_);

    while (run_flag_)
    {
        auto result = co_await ConnectAwaitable(this);
        if (!result.success)
        {
            LOG_WARNING() << std::format("TCP连接失败,本地{}:{},远程{}:{},{}",
                local_ip_, local_port_,
                remote_ip_, remote_port_,
                result.error_info_);

            co_await notify.Wait(reconnect_wait_time_);
            continue;
        }
        FINALLY(closesocket(result.socket_););

        if (CreateIoCompletionPort((HANDLE) result.socket_, completion_handle_, 0, 0) == 0)
        {
            DWORD dw    = GetLastError();
            std::string str_err = std::format("TCP绑定完成端口失败,本地{}:{},远程{}:{},code:{},{}",
                local_ip_, local_port_,
                remote_ip_, remote_port_,
                dw, GetFormatMessage(dw));
            LOG_ERROR() << str_err;

            co_await notify.Wait(reconnect_wait_time_);
            continue;
        }

        std::shared_ptr<TcpChannel> channel = std::make_shared<TcpChannel>(result.socket_, remote_ip_, remote_port_, local_ip_, local_port_);
        if (co_await channel->Start())
        {
            co_await user_->Access(channel);
            channel->Stop();
        }
    }

    co_return;
}

TcpClient::ConnectAwaitable::ConnectAwaitable(TcpClient* client)
    : client_(client)
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
    handle_ = co_handle;

    iocp_data_.call_ = std::bind(&ConnectAwaitable::IoCallback, this, std::placeholders::_1);

    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == socket_)
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("code:{},:{}", dw, GetFormatMessage(dw));
        return false;
    }

    sockaddr_in bind_addr = {};
    inet_pton(AF_INET, client_->local_ip_.c_str(), (void*) &bind_addr.sin_addr);
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port   = htons(client_->local_port_);

    //绑定套接字, 绑定到端口
    ::bind(socket_, (SOCKADDR*) &bind_addr, sizeof(bind_addr)); //会返回一个SOCKET_ERROR

    if (CreateIoCompletionPort((HANDLE) socket_, client_->completion_handle_, 0, 0) == 0)
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("code:{},:{}", dw, GetFormatMessage(dw));
        return false;
    }

    static LPFN_CONNECTEX func_connect = GetConnectEx(socket_);

    sockaddr_in connect_addr = {0};
    inet_pton(AF_INET, client_->remote_ip_.c_str(), (void*) &connect_addr.sin_addr);
    connect_addr.sin_family = AF_INET;
    connect_addr.sin_port   = htons(client_->remote_port_);
    DWORD bytes;
    bool connect_result = func_connect(socket_, (const sockaddr*) &connect_addr, sizeof(connect_addr), nullptr, 0, &bytes, &iocp_data_.overlapped);

    if (!connect_result && WSAGetLastError() != ERROR_IO_PENDING)
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("code:{},:{}", dw, GetFormatMessage(dw));
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
        return false;
    }

    return true;
}

TcpClient::ConnectResult TcpClient::ConnectAwaitable::await_resume()
{
    return ConnectResult{.success = socket_ != INVALID_SOCKET, .socket_ = socket_, .error_info_ = error_info_};
}

void TcpClient::ConnectAwaitable::IoCallback(IOCP_DATA* pData)
{
    if (pData->success_ == 0)
    {
        DWORD dw    = GetLastError();
        error_info_ = std::format("code:{},:{}", dw, GetFormatMessage(dw));
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }

    handle_.resume();
}

} // namespace comm
} // namespace jaf