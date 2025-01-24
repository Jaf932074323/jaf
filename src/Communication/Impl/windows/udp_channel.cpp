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
#include "Log/log_head.h"
#include "Impl/tool/run_with_timeout.h"
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

struct UdpChannel::AwaitableResult
{
    size_t len_ = 0; // 接收数据长度 为0时表示接收失败
    int err_    = 0; // 错误代码
};

class UdpChannel::ReadAwaitable
{
public:
    ReadAwaitable(UdpChannel* tcp_channel, SOCKET socket, unsigned char* buff, size_t size);
    ~ReadAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    AwaitableResult await_resume() const;
    void IoCallback(IOCP_DATA* pData);
    void Stop();

private:
    UdpChannel* tcp_channel_ = nullptr;
    SOCKET socket_           = 0; // 收发数据的套接字
    AwaitableResult reslult_;

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;

    WSABUF wsbuffer_;

    DWORD dwBytes = 0;

    std::mutex mutex_;
    bool callback_flag_ = false; // 已经回调标记
};

class UdpChannel::WriteAwaitable
{
public:
    WriteAwaitable(UdpChannel* tcp_channel, SOCKET socket, const unsigned char* buff, size_t size, sockaddr_in send_addr_);
    ~WriteAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    AwaitableResult await_resume() const;
    void IoCallback(IOCP_DATA* pData);
    void Stop();

private:
    UdpChannel* tcp_channel_ = nullptr;
    SOCKET socket_           = 0; // 收发数据的套接字
    AwaitableResult reslult_;

    sockaddr_in send_addr_ = {};

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;

    WSABUF wsbuffer_;

    DWORD dwBytes = 0;

    std::mutex mutex_;
    bool callback_flag_ = false; // 已经回调标记
};

UdpChannel::UdpChannel(HANDLE completion_handle, SOCKET socket,
    std::string remote_ip, uint16_t remote_port,
    std::string local_ip, uint16_t local_port,
    std::shared_ptr<jaf::time::ITimer> timer)
    : completion_handle_(completion_handle)
    , socket_(socket)
    , remote_ip_(remote_ip)
    , remote_port_(remote_port)
    , local_ip_(local_ip)
    , local_port_(local_port)
    , timer_(timer)
{
    send_addr_.sin_family      = AF_INET;
    send_addr_.sin_port        = htons(remote_port_);
    send_addr_.sin_addr.s_addr = inet_addr(remote_ip_.c_str());
}

UdpChannel::~UdpChannel()
{
}

Coroutine<void> UdpChannel::Run()
{
    control_start_stop_.Start();
    stop_flag_ = false;
    if (CreateIoCompletionPort((HANDLE) socket_, completion_handle_, 0, 0) == 0)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("Communication code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        LOG_ERROR() << str;
        co_return;
    }

    co_await jaf::CoWaitUtilControlledStop(control_start_stop_);
    co_await wait_all_tasks_done_;

    co_return;
}

void UdpChannel::Stop()
{
    stop_flag_ = true;
    control_start_stop_.Stop();
}

Coroutine<SChannelResult> UdpChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    SChannelResult result;
    if (stop_flag_)
    {
        result.state = SChannelResult::EState::CRS_CHANNEL_END;
        co_return result;
    }

    ReadAwaitable read_awaitable(this, socket_, buff, buff_size);
    RunWithTimeout run_with_timeout(control_start_stop_, read_awaitable, timeout, timer_);
    co_await run_with_timeout.Run();

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
            result.state = SChannelResult::EState::CRS_UNKNOWN;
        }
        else
        {
            result.state = SChannelResult::EState::CRS_FAIL;
            result.code_ = read_result.err_;
            Stop();
            result.error = std::format("TcpChannel code error:{},error-msg:{}", read_result.err_, GetFormatMessage(read_result.err_));
        }
    }

    co_return result;
}

Coroutine<SChannelResult> UdpChannel::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    SChannelResult result;
    if (stop_flag_)
    {
        result.state = SChannelResult::EState::CRS_CHANNEL_END;
        co_return result;
    }

    WriteAwaitable write_awaitable(this, socket_, buff, buff_size, send_addr_);
    RunWithTimeout run_with_timeout(control_start_stop_, write_awaitable, timeout, timer_);
    co_await run_with_timeout.Run();

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
            result.state = SChannelResult::EState::CRS_UNKNOWN;
        }
        else
        {
            result.state = SChannelResult::EState::CRS_FAIL;
            result.code_ = write_result.err_;
            Stop();
            result.error = std::format("TcpChannel code error:{},error-msg: {}", write_result.err_, GetFormatMessage(write_result.err_));
        }
    }

    co_return result;
}

Coroutine<SChannelResult> UdpChannel::WriteTo(const unsigned char* buff, size_t buff_size, std::string remote_ip, uint16_t remote_port, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    SChannelResult result;
    if (stop_flag_)
    {
        result.state = SChannelResult::EState::CRS_CHANNEL_END;
        co_return result;
    }

    sockaddr_in send_addr     = {};
    send_addr.sin_family      = AF_INET;
    send_addr.sin_port        = htons(remote_port);
    send_addr.sin_addr.s_addr = inet_addr(remote_ip.c_str());

    WriteAwaitable write_awaitable(this, socket_, buff, buff_size, send_addr);
    RunWithTimeout run_with_timeout(control_start_stop_, write_awaitable, timeout, timer_);
    co_await run_with_timeout.Run();

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
            result.state = SChannelResult::EState::CRS_UNKNOWN;
        }
        else
        {
            result.state = SChannelResult::EState::CRS_FAIL;
            result.code_ = write_result.err_;
            Stop();
            result.error = std::format("TcpChannel code error:{},error-msg: {}", write_result.err_, GetFormatMessage(write_result.err_));
        }
    }

    co_return result;
}

UdpChannel::ReadAwaitable::ReadAwaitable(UdpChannel* tcp_channel, SOCKET socket, unsigned char* buff, size_t size)
    : tcp_channel_(tcp_channel)
    , socket_(socket)
    , wsbuffer_{.len = (ULONG) size, .buf = (char*) buff}
{
}

UdpChannel::ReadAwaitable::~ReadAwaitable()
{
}

bool UdpChannel::ReadAwaitable::await_ready()
{
    return false;
}

bool UdpChannel::ReadAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    DWORD flags      = 0;
    handle           = co_handle;
    iocp_data_.call_ = std::bind(&ReadAwaitable::IoCallback, this, std::placeholders::_1);

    if (SOCKET_ERROR == WSARecv(socket_, &wsbuffer_, 1, nullptr, &flags, &iocp_data_.overlapped, NULL))
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            reslult_.err_ = error;
            return false;
        }
    }

    return true;
}

UdpChannel::AwaitableResult UdpChannel::ReadAwaitable::await_resume() const
{
    return reslult_;
}

void UdpChannel::ReadAwaitable::IoCallback(IOCP_DATA* pData)
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

void UdpChannel::ReadAwaitable::Stop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback_flag_)
    {
        return;
    }
    callback_flag_ = true;
    CancelIoEx((HANDLE) socket_, &iocp_data_.overlapped);
}

UdpChannel::WriteAwaitable::WriteAwaitable(UdpChannel* tcp_channel, SOCKET socket, const unsigned char* buff, size_t size, sockaddr_in send_addr)
    : send_addr_(send_addr)
    , tcp_channel_(tcp_channel)
    , socket_(socket)
    , wsbuffer_{.len = (ULONG) size, .buf = (char*) buff}
{
}

UdpChannel::WriteAwaitable::~WriteAwaitable()
{
}

bool UdpChannel::WriteAwaitable::await_ready()
{
    return false;
}

bool UdpChannel::WriteAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    DWORD flags      = 0;
    handle           = co_handle;
    iocp_data_.call_ = std::bind(&WriteAwaitable::IoCallback, this, std::placeholders::_1);

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

UdpChannel::AwaitableResult UdpChannel::WriteAwaitable::await_resume() const
{
    return reslult_;
}

void UdpChannel::WriteAwaitable::IoCallback(IOCP_DATA* pData)
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

void UdpChannel::WriteAwaitable::Stop()
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

#elif defined(__linux__)
#endif