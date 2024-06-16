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

SerialPort::SerialPort(uint8_t comm, uint32_t baud_rate, uint8_t data_bit, uint8_t stop_bit, uint8_t parity)
    : comm_(comm >= 10 ? std::format("\\\\.\\COM{}", comm) : std::format("COM{}", comm))
    , baud_rate_(baud_rate)
    , data_bit_(data_bit)
    , stop_bit_(stop_bit)
    , parity_(parity)
{
}

SerialPort::~SerialPort()
{
}

void SerialPort::SetChannelUser(std::shared_ptr<IChannelUser> user)
{
    user_ = user;
}

jaf::Coroutine<void> SerialPort::Run(HANDLE completion_handle)
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    completion_handle_ = completion_handle;
    Init();

    std::shared_ptr<SerialPortChannel> channel = std::make_shared<SerialPortChannel>(completion_handle_, comm_handle_);

    if (co_await channel->Start())
    {
        co_await user_->Access(channel);
        channel->Stop();
    }

    co_return;
}

void SerialPort::Stop()
{
    if (!run_flag_)
    {
        return;
    }
    run_flag_ = false;
}

void SerialPort::Init(void)
{
    OpenSerialPort();
}

bool SerialPort::OpenSerialPort()
{
    HANDLE comm_handle = CreateFile(comm_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
    if (comm_handle == INVALID_HANDLE_VALUE)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("Iocp code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        LOG_ERROR(LOG_NAME) << str;
        return false;
    }

    // timeout
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(comm_handle, &timeouts);
    timeouts.ReadIntervalTimeout        = 200;
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

    comm_handle_ = comm_handle;

    return true;
}

void SerialPort::CloseSerialPort()
{
    if (completion_handle_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(completion_handle_);
        completion_handle_ = INVALID_HANDLE_VALUE;
    }
}

} // namespace comm
} // namespace jaf