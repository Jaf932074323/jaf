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

#include "serial_port.h"
#include "define_constant.h"
#include "log_head.h"
#include "serial_port_channel.h"
#include <WS2tcpip.h>
#include <format>

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

SerialPort::SerialPort(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer)
    : get_completion_port_(get_completion_port)
    , timer_(timer)
{
    assert(get_completion_port_ != nullptr);
}

SerialPort::~SerialPort()
{
}

void SerialPort::SetAddr(const std::string& comm_name, uint32_t baud_rate, uint8_t data_bit, uint8_t stop_bit, uint8_t parity)
{
    // comm_      = comm >= 10 ? std::format("\\\\.\\COM{}", comm) : std::format("COM{}", comm);
    comm_      = comm_name;
    baud_rate_ = baud_rate;
    data_bit_  = data_bit;
    stop_bit_  = stop_bit;
    parity_    = parity;
}

void SerialPort::SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel)
{
    handle_channel_ = handle_channel;
}

jaf::Coroutine<RunResult> SerialPort::Run()
{
    assert(!run_flag_);
    run_flag_ = true;

    completion_handle_ = get_completion_port_->Get();

    HANDLE comm_handle = OpenSerialPort();
    if (comm_handle == INVALID_HANDLE_VALUE)
    {
        run_flag_ = false;
        co_return error_info_;
    }

    wait_stop_.Start();
    auto run = RunSerialPort(comm_handle);

    co_await wait_stop_.Wait();
    run_flag_ = false;

    {
        std::unique_lock lock(channel_mutex_);
        channel_->Stop();
    }
    CloseHandle(comm_handle);
    co_await run;

    co_return true;
}

void SerialPort::Stop()
{
    wait_stop_.Stop();
}

std::shared_ptr<IChannel> SerialPort::GetChannel()
{
    std::unique_lock lock(channel_mutex_);
    assert(channel_ != nullptr);
    return channel_;
}

Coroutine<SChannelResult> SerialPort::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    std::shared_ptr<IChannel> channel = GetChannel();
    co_return co_await channel->Write(buff, buff_size, timeout);
}

HANDLE SerialPort::OpenSerialPort()
{
    HANDLE comm_handle = CreateFile(comm_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
    if (comm_handle == INVALID_HANDLE_VALUE)
    {
        DWORD dw        = GetLastError();
        error_info_ = std::format("Communication code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        return INVALID_HANDLE_VALUE;
    }

    // TODO:这些参数的设置会影响读写效果，后续需要提供接口让用户配置
    // timeout
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(comm_handle, &timeouts);
    timeouts.ReadIntervalTimeout        = 20;
    timeouts.ReadTotalTimeoutConstant   = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    SetCommTimeouts(comm_handle, &timeouts);

    // rw queue
    SetupComm(comm_handle, 100 * 1024, 100 * 1024);

    // dcb
    DCB dcb;
    GetCommState(comm_handle, &dcb);
    if (baud_rate_ < CBR_110)
    {
        dcb.BaudRate = CBR_110;
    }
    else if (baud_rate_ > CBR_256000)
    {
        dcb.BaudRate = CBR_256000;
    }
    else
    {
        dcb.BaudRate = baud_rate_;
    }

    dcb.Parity   = parity_;
    dcb.ByteSize = data_bit_;
    dcb.StopBits = stop_bit_;
    SetCommState(comm_handle, &dcb);

    PurgeComm(comm_handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    return comm_handle;
}

Coroutine<void> SerialPort::RunSerialPort(HANDLE comm_handle)
{
    std::shared_ptr<SerialPortChannel> channel = std::make_shared<SerialPortChannel>(completion_handle_, comm_handle, timer_);

    {
        std::unique_lock lock(channel_mutex_);
        if (!run_flag_)
        {
            co_return;
        }
        channel_ = channel;        
    }

    jaf::Coroutine<RunResult> channel_run = channel->Run();
    co_await handle_channel_(channel);
    channel->Stop();
    co_await channel_run;

    {
        std::unique_lock lock(channel_mutex_);
        channel_ = std::make_shared<EmptyChannel>();
    }

    Stop();
}

} // namespace comm
} // namespace jaf

#elif defined(__linux__)
#endif