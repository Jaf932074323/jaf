#include "tcp_channel.h"
#include "Log/log_head.h"
#include <WS2tcpip.h>
#include <assert.h>
#include <format>
#include <mswsock.h>

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

class TcpChannel::ReadAwaitable
{
public:
    ReadAwaitable(TcpChannel* tcp_channel, SOCKET socket, unsigned char* buff, size_t size, uint64_t timeout);
    ~ReadAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    size_t await_resume() const;
    void IoCallback(IOCP_DATA* pData);
    void TimerCallback();

private:
    TcpChannel* tcp_channel_ = nullptr;
    SOCKET socket_           = 0; // 收发数据的套接字
    size_t recv_len_         = 0; // 接收数据长度

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;
    WSABUF wsbuffer_;

    std::mutex time_mutex_;      // 超时使用的同步
    bool callback_flag_ = false; // 已经回调标记
    bool timeout_flag_  = false; // 超时标记

    uint64_t timeout_  = 5000; // 超时时间
    uint64_t timer_id_ = 0;    // 定时ID
};

class TcpChannel::WriteAwaitable
{
public:
    WriteAwaitable(TcpChannel* tcp_channel, SOCKET socket, const unsigned char* buff, size_t size, uint64_t timeout);
    ~WriteAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    size_t await_resume() const;
    void IoCallback(IOCP_DATA* pData);

    void TimerCallback();

private:
    TcpChannel* tcp_channel_ = nullptr;
    SOCKET socket_           = 0; // 收发数据的套接字
    size_t write_len_        = 0; // 接收数据长度

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;

    WSABUF wsbuffer_;
    DWORD dwBytes = 0;

    std::mutex time_mutex_;      // 超时使用的同步
    bool callback_flag_ = false; // 已经回调标记
    bool timeout_flag_  = false; // 超时标记

    uint64_t timeout_  = 5000; // 超时时间
    uint64_t timer_id_ = 0;    // 定时ID
};

TcpChannel::TcpChannel(SOCKET socket, std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port, std::shared_ptr<jaf::time::Timer> timer)
    : socket_(socket)
    , remote_ip_(remote_ip)
    , remote_port_(remote_port)
    , local_ip_(local_ip)
    , local_port_(local_port)
    , timer_(timer == nullptr ? jaf::time::CommonTimer::Timer() : timer)
{
}

TcpChannel::~TcpChannel() {}

Coroutine<bool> TcpChannel::Start()
{
    co_return true;
}

void TcpChannel::Stop()
{
    CancelIo((HANDLE) socket_);
}

Coroutine<SChannelResult> TcpChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    SChannelResult result;
    result.len = co_await ReadAwaitable{this, socket_, buff, buff_size, timeout};

    if (result.len != 0)
    {
        result.success = true;
        co_return result;
    }

    DWORD dw = GetLastError();
    if (WAIT_TIMEOUT == dw)
    {
        result.timeout = true;
    }
    else
    {
        result.timeout = false;
        CancelIo((HANDLE) socket_);
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }

    result.error = std::format("TcpChannel code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));

    co_return result;
}

Coroutine<SChannelResult> TcpChannel::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    SChannelResult result;
    result.len = co_await WriteAwaitable{this, socket_, buff, buff_size, timeout};

    if (result.len != 0)
    {
        result.success = true;
        co_return result;
    }

    DWORD dw = GetLastError();
    if (WAIT_TIMEOUT == dw)
    {
        result.timeout = true;
    }
    else
    {
        result.timeout = false;
        CancelIo((HANDLE) socket_);
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }

    result.error = std::format("TcpChannel code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
    co_return SChannelResult{.success = true};
}

TcpChannel::ReadAwaitable::ReadAwaitable(TcpChannel* tcp_channel, SOCKET socket, unsigned char* buff, size_t size, uint64_t timeout)
    : tcp_channel_(tcp_channel)
    , socket_(socket)
    , wsbuffer_{.len = (ULONG) size, .buf = (char*) buff}
    , timeout_(timeout)
{
}

TcpChannel::ReadAwaitable::~ReadAwaitable()
{
}
bool TcpChannel::ReadAwaitable::await_ready()
{
    return false;
}

bool TcpChannel::ReadAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    DWORD flags      = 0;
    handle           = co_handle;
    iocp_data_.call_ = [this](IOCP_DATA* pData) { IoCallback(pData); };

    if (SOCKET_ERROR == WSARecv(socket_, &wsbuffer_, 1, nullptr, &flags, &iocp_data_.overlapped, NULL))
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            return false;
        }
    }

    jaf::time::STimerPara task{[this]() { TimerCallback(); }, timeout_};
    tcp_channel_->timer_->StartTask(task);

    return true;
}

size_t TcpChannel::ReadAwaitable::await_resume() const
{
    if (timeout_flag_)
    {
        WSASetLastError(WAIT_TIMEOUT);
    }
    return recv_len_;
}

void TcpChannel::ReadAwaitable::IoCallback(IOCP_DATA* pData)
{
    {
        std::unique_lock<std::mutex> lock(time_mutex_);
        callback_flag_ = true;
    }

    if (pData->success_ == 0)
    {
        recv_len_ = 0;
    }
    else
    {
        recv_len_ = (size_t) pData->bytesTransferred_;
    }

    handle.resume();
}

void TcpChannel::ReadAwaitable::TimerCallback()
{
    std::unique_lock<std::mutex> lock(time_mutex_);
    if (callback_flag_)
    {
        return;
    }
    callback_flag_ = true;
    timeout_flag_  = true;
    CancelIoEx((HANDLE) socket_, &iocp_data_.overlapped);
}

TcpChannel::WriteAwaitable::WriteAwaitable(TcpChannel* tcp_channel, SOCKET socket, const unsigned char* buff, size_t size, uint64_t timeout)
    : tcp_channel_(tcp_channel)
    , socket_(socket)
    , wsbuffer_{.len = (ULONG) size, .buf = (char*) buff}
    , timeout_(timeout)
{
}

TcpChannel::WriteAwaitable::~WriteAwaitable()
{
}

bool TcpChannel::WriteAwaitable::await_ready()
{
    return false;
}

bool TcpChannel::WriteAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    DWORD flags      = 0;
    handle           = co_handle;
    iocp_data_.call_ = std::bind(&WriteAwaitable::IoCallback, this, std::placeholders::_1);

    if (SOCKET_ERROR == WSASend(socket_, &wsbuffer_, 1, nullptr, flags, &iocp_data_.overlapped, NULL))
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            return false;
        }
    }

    jaf::time::STimerPara task{[this]() { TimerCallback(); }, timeout_};
    tcp_channel_->timer_->StartTask(task);

    return true;
}

size_t TcpChannel::WriteAwaitable::await_resume() const
{
    return write_len_;
}

void TcpChannel::WriteAwaitable::IoCallback(IOCP_DATA* pData)
{
    {
        std::unique_lock<std::mutex> lock(time_mutex_);
        callback_flag_ = true;
    }

    if (pData->success_ == 0)
    {
        write_len_ = 0;
    }
    else
    {
        write_len_ = (size_t) pData->bytesTransferred_;
    }

    handle.resume();
}

void TcpChannel::WriteAwaitable::TimerCallback()
{
    std::unique_lock<std::mutex> lock(time_mutex_);
    if (callback_flag_)
    {
        return;
    }
    callback_flag_ = true;
    timeout_flag_  = true;
    CancelIoEx((HANDLE) socket_, &iocp_data_.overlapped);
}

} // namespace comm
} // namespace jaf