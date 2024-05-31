#include "serial_port_channel.h"
#include "Log/log_head.h"
#include <assert.h>
#include <format>


namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

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
    if (CreateIoCompletionPort(comm_handle_, completion_handle_, (ULONG_PTR) comm_handle_, 0) == 0)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("Iocp code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        LOG_ERROR() << str;
        CloseHandle(comm_handle_);
        co_return false;
    }
    co_return true;
}

Coroutine<SChannelResult> SerialPortChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    SChannelResult result;
    result.len = co_await ReadAwaitable{this, buff, buff_size};

    if (result.len == 0)
    {
        result.state = SChannelResult::EState::CRS_UNKNOWN;
        co_return result;
    }
    result.state = SChannelResult::EState::CRS_SUCCESS;
    co_return result;
}

Coroutine<SChannelResult> SerialPortChannel::Write(const unsigned char* buff,
    size_t buff_size, uint64_t timeout)
{
    co_await WriteAwaitable{this, buff, buff_size};
    co_return SChannelResult{.state = SChannelResult::EState::CRS_SUCCESS};
}

void SerialPortChannel::Stop()
{
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
        int dw = GetLastError();
        if (WAIT_TIMEOUT != dw && WSAGetLastError() != ERROR_IO_PENDING)
        {
            std::string str = std::format("SerialPortChannel code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
            OutputDebugString(str.c_str());
        }
    }

    return true;
}

size_t SerialPortChannel::ReadAwaitable::await_resume()
{
    return recv_len_;
}

void SerialPortChannel::ReadAwaitable::IoCallback(IOCP_DATA* pData)
{
    if (pData->success_ == 0)
    {
        DWORD dw = GetLastError();
        if (WAIT_TIMEOUT != dw)
        {
            std::string str = std::format("SerialPortChannel code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
            OutputDebugString(str.c_str());
        }

        recv_len_ = 0;
    }
    else
    {
        recv_len_ = (size_t) pData->bytesTransferred_;
    }

    handle.resume();
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
        if (error != ERROR_IO_PENDING && WSAGetLastError() != ERROR_IO_PENDING)
        {
            errno = error;
            return false;
        }
    }

    return true;
}

size_t SerialPortChannel::WriteAwaitable::await_resume()
{
    return write_len_;
}

void SerialPortChannel::WriteAwaitable::IoCallback(IOCP_DATA* pData)
{
    if (pData->success_ == 0)
    {
        DWORD dw = GetLastError();
        if (WAIT_TIMEOUT != dw)
        {
            std::string str = std::format("SerialPortChannel code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
            OutputDebugString(str.c_str());
        }

        write_len_ = 0;
    }
    else
    {
        write_len_ = (size_t) pData->bytesTransferred_;
    }

    handle.resume();
}

} // namespace comm
} // namespace jaf