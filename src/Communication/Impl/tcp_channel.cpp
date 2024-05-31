#include "tcp_channel.h"
#include "impl/tool/run_with_timeout.h"
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

struct TcpChannel::AwaitableResult
{
    size_t len_ = 0; // 接收数据长度 为0时表示接收失败
    int err_    = 0; // 错误代码
};
class TcpChannel::ReadAwaitable
{
public:
    ReadAwaitable(TcpChannel* tcp_channel, SOCKET socket, unsigned char* buff, size_t size);
    ~ReadAwaitable();

    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    AwaitableResult await_resume() const;
    void IoCallback(IOCP_DATA* pData);
    void Stop();

private:
    TcpChannel* tcp_channel_ = nullptr;
    SOCKET socket_           = 0; // 收发数据的套接字
    AwaitableResult reslult_;

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;
    WSABUF wsbuffer_;

    std::mutex mutex_;
    bool callback_flag_ = false; // 已经回调标记
};

class TcpChannel::WriteAwaitable
{
public:
    WriteAwaitable(TcpChannel* tcp_channel, SOCKET socket, const unsigned char* buff, size_t size);
    ~WriteAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    AwaitableResult await_resume() const;
    void IoCallback(IOCP_DATA* pData);
    void Stop();

private:
    TcpChannel* tcp_channel_ = nullptr;
    SOCKET socket_           = 0; // 收发数据的套接字
    AwaitableResult reslult_;

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;

    WSABUF wsbuffer_;
    DWORD dwBytes = 0;

    std::mutex mutex_;
    bool callback_flag_ = false; // 已经回调标记
};

TcpChannel::TcpChannel(SOCKET socket, std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port, std::shared_ptr<jaf::time::ITimer> timer)
    : socket_(socket)
    , remote_ip_(remote_ip)
    , remote_port_(remote_port)
    , local_ip_(local_ip)
    , local_port_(local_port)
    , read_await_(timer)
    , write_await_(timer)
{
}

TcpChannel::~TcpChannel() {}

Coroutine<bool> TcpChannel::Start()
{
    stop_flag_ = false;
    read_await_.Start();
    write_await_.Start();
    co_return true;
}

void TcpChannel::Stop()
{
    stop_flag_ = true;
    write_await_.Stop();
    read_await_.Stop();
}

Coroutine<SChannelResult> TcpChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    ReadAwaitable read_awaitable(this, socket_, buff, buff_size);
    RunWithTimeout run_with_timeout(read_await_, read_awaitable, timeout);
    co_await run_with_timeout.Run();

    SChannelResult result;

    const AwaitableResult& read_result = run_with_timeout.Result();
    if (run_with_timeout.IsTimeout())
    {
        result.state = SChannelResult::EState::CRS_TIMEOUT;
    }
    result.len = read_result.len_;

    if (result.len != 0)
    {
        result.state = SChannelResult::EState::CRS_SUCCESS;
        co_return result;
    }

    if (stop_flag_)
    {
        result.state = SChannelResult::EState::CRS_CHANNEL_END;
        co_return result;
    }

    if (result.state != SChannelResult::EState::CRS_TIMEOUT)
    {
        if (read_result.err_ == 0)
        {
            result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            result.error = "The connection has been disconnected";
        }
        else
        {
            result.state = SChannelResult::EState::CRS_FAIL;
            result.code_ = read_result.err_;
            CancelIo((HANDLE) socket_);
            closesocket(socket_);
            socket_      = INVALID_SOCKET;
            result.error = std::format("TcpChannel code error:{},error-msg:{}", read_result.err_, GetFormatMessage(read_result.err_));
        }
    }

    co_return result;
}

Coroutine<SChannelResult> TcpChannel::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    WriteAwaitable write_awaitable(this, socket_, buff, buff_size);
    RunWithTimeout run_with_timeout(write_await_, write_awaitable, timeout);
    co_await run_with_timeout.Run();

    SChannelResult result;
    const AwaitableResult& write_result = run_with_timeout.Result();
    if (run_with_timeout.IsTimeout())
    {
        result.state = SChannelResult::EState::CRS_TIMEOUT;
    }
    result.len = write_result.len_;

    if (result.len != 0)
    {
        result.state = SChannelResult::EState::CRS_SUCCESS;
        co_return result;
    }

    if (stop_flag_)
    {
        result.state = SChannelResult::EState::CRS_CHANNEL_END;
        co_return result;
    }

    if (result.state != SChannelResult::EState::CRS_TIMEOUT)
    {
        if (write_result.err_ == 0)
        {
            result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            result.error = "The connection has been disconnected";
        }
        else
        {
            result.state = SChannelResult::EState::CRS_FAIL;
            result.code_ = write_result.err_;
            CancelIo((HANDLE) socket_);
            closesocket(socket_);
            socket_      = INVALID_SOCKET;
            result.error = std::format("TcpChannel code error:{},error-msg: {}", write_result.err_, GetFormatMessage(write_result.err_));
        }
    }

    co_return result;
}

TcpChannel::ReadAwaitable::ReadAwaitable(TcpChannel* tcp_channel, SOCKET socket, unsigned char* buff, size_t size)
    : tcp_channel_(tcp_channel)
    , socket_(socket)
    , wsbuffer_{.len = (ULONG) size, .buf = (char*) buff}
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
    handle = co_handle;
    DWORD flags      = 0;
    iocp_data_.call_ = [this](IOCP_DATA* pData) { IoCallback(pData); };

    if (SOCKET_ERROR == WSARecv(socket_, &wsbuffer_, 1, nullptr, &flags, &iocp_data_.overlapped, NULL))
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            return false;
        }
    }

    return true;
}

TcpChannel::AwaitableResult TcpChannel::ReadAwaitable::await_resume() const
{
    return reslult_;
}

void TcpChannel::ReadAwaitable::IoCallback(IOCP_DATA* pData)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        callback_flag_ = true;
    }

    if (pData->success_ == 0)
    {
        reslult_.len_ = 0;
        reslult_.err_ = WSAGetLastError();
    }
    else
    {
        reslult_.len_ = (size_t) pData->bytesTransferred_;
    }

    handle.resume();
}

void TcpChannel::ReadAwaitable::Stop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback_flag_)
    {
        return;
    }
    callback_flag_ = true;
    CancelIoEx((HANDLE) socket_, &iocp_data_.overlapped);
}

TcpChannel::WriteAwaitable::WriteAwaitable(TcpChannel* tcp_channel, SOCKET socket, const unsigned char* buff, size_t size)
    : tcp_channel_(tcp_channel)
    , socket_(socket)
    , wsbuffer_{.len = (ULONG) size, .buf = (char*) buff}
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

    return true;
}

TcpChannel::AwaitableResult TcpChannel::WriteAwaitable::await_resume() const
{
    return reslult_;
}

void TcpChannel::WriteAwaitable::IoCallback(IOCP_DATA* pData)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        callback_flag_ = true;
    }

    if (pData->success_ == 0)
    {
        reslult_.len_ = 0;
        reslult_.err_ = WSAGetLastError();
    }
    else
    {
        reslult_.len_ = (size_t) pData->bytesTransferred_;
    }

    handle.resume();
}

void TcpChannel::WriteAwaitable::Stop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback_flag_)
    {
        return;
    }
    callback_flag_ = true;
    CancelIoEx((HANDLE) socket_, &iocp_data_.overlapped);
}

} // namespace comm
} // namespace jaf