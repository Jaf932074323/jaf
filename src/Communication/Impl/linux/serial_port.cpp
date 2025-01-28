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
#elif defined(__linux__)

#include "serial_port.h"
#include "define_constant.h"
#include "log_head.h"
#include "serial_port_channel.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <format>
#include <string.h>
#include <termios.h>

namespace jaf
{
namespace comm
{

SerialPort::SerialPort(IGetEpollFd* get_epoll_fd, std::shared_ptr<jaf::time::ITimer> timer)
    : get_epoll_fd_(get_epoll_fd)
    , timer_(timer)
{
    assert(get_epoll_fd != nullptr);
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

jaf::Coroutine<void> SerialPort::Run()
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    epoll_fd_ = get_epoll_fd_->Get();
    Init();

    std::shared_ptr<SerialPortChannel> channel = std::make_shared<SerialPortChannel>(file_descriptor_, epoll_fd_, timer_);

    {
        std::unique_lock lock(channel_mutex_);
        channel_ = channel;
    }

    jaf::Coroutine<void> channel_run = channel->Run();
    co_await handle_channel_(channel);
    channel->Stop();
    co_await channel_run;

    run_flag_ = false;
    CloseSerialPort();

    co_return;
}

void SerialPort::Stop()
{
    run_flag_ = false;
    std::unique_lock lock(channel_mutex_);
    if (channel_ != nullptr)
    {
        channel_->Stop();
        channel_ = std::make_shared<EmptyChannel>();
    }
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

void SerialPort::Init(void)
{
    OpenSerialPort();
}

bool SerialPort::OpenSerialPort()
{
    file_descriptor_ = ::open(comm_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (file_descriptor_ < 0)
    {
        int error   = errno;
        error_info_ = std::format("Failed to create a listen socket,code:{},:{}", error, strerror(error));
        return false;
    }

    struct termios tios;

    tcgetattr(file_descriptor_, &tios);

    cfmakeraw(&tios);
    tios.c_cflag &= ~(CSIZE | CRTSCTS);
    tios.c_iflag &= ~(IXON | IXOFF | IXANY | IGNPAR);
    tios.c_lflag &= ~(ECHOK | ECHOCTL | ECHOKE);
    tios.c_oflag &= ~(OPOST | ONLCR);

    speed_t baud = TransitionBaudRate(baud_rate_);
    if (baud <= 0)
    {
        return false;
    }
#if defined(_BSD_SOURCE) || defined(_DEFAULT_SOURCE)
    ::cfsetspeed(&tios, baud);
#else
    ::cfsetispeed(&tios, baud);
    ::cfsetospeed(&tios, baud);
#endif

    // tios.c_iflag |= (options.xon ? IXON : 0) | (options.xoff ? IXOFF : 0) | (options.xany ? IXANY : 0);

    // data bits
    // tios.c_cflag &= ~0x30;
    switch (data_bit_)
    {
    case 5: tios.c_cflag |= CS5; break;
    case 6: tios.c_cflag |= CS6; break;
    case 7: tios.c_cflag |= CS7; break;
    case 8: tios.c_cflag |= CS8; break;
    default: break;
    }


    // stop bits
    if (stop_bit_ == 1)
    {
        tios.c_cflag |= CSTOPB;
    }
    else
    {
        tios.c_cflag &= ~CSTOPB;
    }

    enum Parity
    {
        ParityNone,
        ParityEven,
        PariteMark,
        ParityOdd,
        ParitySpace
    };

    // parity
    if (parity_ == ParityNone)
    {
        tios.c_cflag &= ~PARENB;
    }
    else
    {
        tios.c_cflag |= PARENB;

        if (parity_ == PariteMark)
        {
            tios.c_cflag |= PARMRK;
        }
        else
        {
            tios.c_cflag &= ~PARMRK;
        }

        if (parity_ == ParityOdd)
        {
            tios.c_cflag |= PARODD;
        }
        else
        {
            tios.c_cflag &= ~PARODD;
        }
    }

    // tios.c_cc[VMIN] = options.vmin;
    // tios.c_cc[VTIME] = options.vtime;

    tcsetattr(file_descriptor_, TCSANOW, &tios);
    tcflush(file_descriptor_, TCIOFLUSH);

    return true;
}

void SerialPort::CloseSerialPort()
{
    ::close(file_descriptor_);
}

speed_t SerialPort::TransitionBaudRate(uint32_t baud_rate)
{
    speed_t baud;
    switch (baud_rate)
    {
    // Do POSIX-specified rates first.
    case 0: baud = B0; break;
    case 50: baud = B50; break;
    case 75: baud = B75; break;
    case 110: baud = B110; break;
    case 134: baud = B134; break;
    case 150: baud = B150; break;
    case 200: baud = B200; break;
    case 300: baud = B300; break;
    case 600: baud = B600; break;
    case 1200: baud = B1200; break;
    case 1800: baud = B1800; break;
    case 2400: baud = B2400; break;
    case 4800: baud = B4800; break;
    case 9600: baud = B9600; break;
    case 19200: baud = B19200; break;
    case 38400:
        baud = B38400;
        break;
        // And now the extended ones conditionally.
#ifdef B7200
    case 7200: baud = B7200; break;
#endif
#ifdef B14400
    case 14400: baud = B14400; break;
#endif
#ifdef B57600
    case 57600: baud = B57600; break;
#endif
#ifdef B115200
    case 115200: baud = B115200; break;
#endif
#ifdef B230400
    case 230400: baud = B230400; break;
#endif
#ifdef B460800
    case 460800: baud = B460800; break;
#endif
#ifdef B500000
    case 500000: baud = B500000; break;
#endif
#ifdef B576000
    case 576000: baud = B576000; break;
#endif
#ifdef B921600
    case 921600: baud = B921600; break;
#endif
#ifdef B1000000
    case 1000000: baud = B1000000; break;
#endif
#ifdef B1152000
    case 1152000: baud = B1152000; break;
#endif
#ifdef B2000000
    case 2000000: baud = B2000000; break;
#endif
#ifdef B3000000
    case 3000000: baud = B3000000; break;
#endif
#ifdef B3500000
    case 3500000: baud = B3500000; break;
#endif
#ifdef B4000000
    case 4000000: baud = B4000000; break;
#endif
    default: baud = 0; break;
    }

    return baud;
}

} // namespace comm
} // namespace jaf

#endif