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
#include "serial_port_channel.h"
#include "Log/log_head.h"
#include "impl/tool/run_with_timeout.h"
#include <assert.h>
#include <format>
#include <mutex>


namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

struct SerialPortChannel::AwaitableResult
{
    size_t len_ = 0; // 接收数据长度 为0时表示接收失败
    int err_    = 0; // 错误代码
};

class SerialPortChannel::ReadAwaitable
{
public:
    ReadAwaitable(SerialPortChannel* serial_port_channel, unsigned char* buff, size_t size);
    ~ReadAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    AwaitableResult await_resume() const;
    void IoCallback(IOCP_DATA* pData);
    void Stop();

private:
    SerialPortChannel* serial_port_channel_ = nullptr;
    AwaitableResult reslult_;

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;

    WSABUF wsbuffer_;

    DWORD dwBytes = 0;

    std::mutex mutex_;
    bool callback_flag_ = false; // 已经回调标记
};

class SerialPortChannel::WriteAwaitable
{
public:
    WriteAwaitable(SerialPortChannel* serial_port_channel, const unsigned char* buff, size_t size);
    ~WriteAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    AwaitableResult await_resume() const;
    void IoCallback(IOCP_DATA* pData);
    void Stop();

private:
    SerialPortChannel* serial_port_channel_ = nullptr;
    AwaitableResult reslult_;

    sockaddr_in send_addr_ = {};

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;

    WSABUF wsbuffer_;

    DWORD dwBytes = 0;

    std::mutex mutex_;
    bool callback_flag_ = false; // 已经回调标记
};

SerialPortChannel::SerialPortChannel(HANDLE completion_handle, HANDLE comm_handle)
    : completion_handle_(completion_handle)
    , comm_handle_(comm_handle)
{
}

SerialPortChannel::~SerialPortChannel()
{
}

Coroutine<bool> SerialPortChannel::Start()
{
    stop_flag_ = false;
    read_await_.Start();
    write_await_.Start();
    if (CreateIoCompletionPort(comm_handle_, completion_handle_, (ULONG_PTR) comm_handle_, 0) == 0)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("Iocp code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        LOG_ERROR() << str;
        co_return false;
    }
    co_return true;
}

void SerialPortChannel::Stop()
{
    stop_flag_ = true;
    write_await_.Stop();
    read_await_.Stop();
}

Coroutine<SChannelResult> SerialPortChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    ReadAwaitable read_awaitable(this, buff, buff_size);
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
            result.state = SChannelResult::EState::CRS_UNKNOWN;
        }
        else
        {
            result.state = SChannelResult::EState::CRS_FAIL;
            result.code_ = read_result.err_;
            result.error = std::format("SerialPortChannel code error:{},error-msg:{}", read_result.err_, GetFormatMessage(read_result.err_));
        }
    }

    co_return result;
}

Coroutine<SChannelResult> SerialPortChannel::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    WriteAwaitable write_awaitable(this, buff, buff_size);
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
            result.state = SChannelResult::EState::CRS_UNKNOWN;
        }
        else
        {
            result.state = SChannelResult::EState::CRS_FAIL;
            result.code_ = write_result.err_;
            result.error = std::format("SerialPortChannel code error:{},error-msg:{}", write_result.err_, GetFormatMessage(write_result.err_));
        }
    }

    co_return result;
}

SerialPortChannel::ReadAwaitable::ReadAwaitable(SerialPortChannel* serial_port_channel, unsigned char* buff, size_t size)
    : serial_port_channel_(serial_port_channel)
    , wsbuffer_{.len = (ULONG) size, .buf = (char*) buff}
{
}

SerialPortChannel::ReadAwaitable::~ReadAwaitable()
{
}

bool SerialPortChannel::ReadAwaitable::await_ready()
{
    return false;
}

bool SerialPortChannel::ReadAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    DWORD flags = 0;
    handle      = co_handle;

    iocp_data_.call_ = std::bind(&ReadAwaitable::IoCallback, this, std::placeholders::_1);

    if (!ReadFile(serial_port_channel_->comm_handle_, wsbuffer_.buf, wsbuffer_.len, nullptr, &iocp_data_.overlapped))
    {
        int error = GetLastError();
        if (error != ERROR_IO_PENDING)
        {
            reslult_.err_ = error;
            return false;
        }
    }

    return true;
}

SerialPortChannel::AwaitableResult SerialPortChannel::ReadAwaitable::await_resume() const
{
    return reslult_;
}

void SerialPortChannel::ReadAwaitable::IoCallback(IOCP_DATA* pData)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        callback_flag_ = true;
    }

    if (pData->success_ == 0)
    {
        reslult_.len_ = 0;
        reslult_.err_ = GetLastError();
    }
    else
    {
        reslult_.len_ = (size_t) pData->bytesTransferred_;
    }

    handle.resume();
}

void SerialPortChannel::ReadAwaitable::Stop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback_flag_)
    {
        return;
    }
    callback_flag_ = true;
    CancelIoEx(serial_port_channel_->comm_handle_, &iocp_data_.overlapped);
}

SerialPortChannel::WriteAwaitable::WriteAwaitable(SerialPortChannel* serial_port_channel, const unsigned char* buff, size_t size)
    : serial_port_channel_(serial_port_channel)
    , wsbuffer_{.len = (ULONG) size, .buf = (char*) buff}
{
}

SerialPortChannel::WriteAwaitable::~WriteAwaitable()
{
}

bool SerialPortChannel::WriteAwaitable::await_ready()
{
    return false;
}

bool SerialPortChannel::WriteAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    DWORD flags = 0;
    handle      = co_handle;

    iocp_data_.call_ = std::bind(&WriteAwaitable::IoCallback, this, std::placeholders::_1);

    if (!WriteFile(serial_port_channel_->comm_handle_, wsbuffer_.buf, wsbuffer_.len, nullptr, &iocp_data_.overlapped))
    {
        int error = GetLastError();
        if (error != ERROR_IO_PENDING)
        {
            reslult_.err_ = error;
            return false;
        }
    }

    return true;
}

SerialPortChannel::AwaitableResult SerialPortChannel::WriteAwaitable::await_resume() const
{
    return reslult_;
}

void SerialPortChannel::WriteAwaitable::IoCallback(IOCP_DATA* pData)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        callback_flag_ = true;
    }

    if (pData->success_ == 0)
    {
        reslult_.len_ = 0;
        reslult_.err_ = GetLastError();
    }
    else
    {
        reslult_.len_ = (size_t) pData->bytesTransferred_;
    }

    handle.resume();
}

void SerialPortChannel::WriteAwaitable::Stop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback_flag_)
    {
        return;
    }
    callback_flag_ = true;
    CancelIoEx(serial_port_channel_->comm_handle_, &iocp_data_.overlapped);
}

} // namespace comm
} // namespace jaf