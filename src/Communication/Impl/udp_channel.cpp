#include "udp_channel.h"
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

class UdpChannel::ReadAwaitable
{
public:
    ReadAwaitable(UdpChannel* tcp_channel, SOCKET socket, unsigned char* buff, size_t size);
    ~ReadAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    size_t await_resume() const;
    void IoCallback(IOCP_DATA* pData);
    void Stop();

private:
    UdpChannel* tcp_channel_ = nullptr;
    SOCKET socket_           = 0; // 收发数据的套接字
    size_t recv_len_         = 0; // 接收数据长度

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
    size_t await_resume() const;
    void IoCallback(IOCP_DATA* pData);
    void Stop();

private:
    UdpChannel* tcp_channel_ = nullptr;
    SOCKET socket_           = 0; // 收发数据的套接字
    size_t write_len_        = 0; // 接收数据长度

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
    std::string local_ip, uint16_t local_port)
    : completion_handle_(completion_handle)
    , socket_(socket)
    , remote_ip_(remote_ip)
    , remote_port_(remote_port)
    , local_ip_(local_ip)
    , local_port_(local_port)
{
    send_addr_.sin_family      = AF_INET;
    send_addr_.sin_port        = htons(remote_port_);
    send_addr_.sin_addr.s_addr = inet_addr(remote_ip_.c_str());
}

UdpChannel::~UdpChannel()
{
}

Coroutine<bool> UdpChannel::Start()
{
    CreateIoCompletionPort((HANDLE) socket_, completion_handle_, 0, 0);
    co_return true;
}

Coroutine<SChannelResult> UdpChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    SChannelResult result;
    result.len = co_await ReadAwaitable{this, socket_, buff, buff_size};

    if (result.len == 0)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_UNKNOWN};
    }

    co_return SChannelResult{.state = SChannelResult::EState::CRS_SUCCESS};
}

Coroutine<SChannelResult> UdpChannel::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    co_await WriteAwaitable{this, socket_, buff, buff_size, send_addr_};
    co_return SChannelResult{.state = SChannelResult::EState::CRS_SUCCESS};
}

Coroutine<SChannelResult> UdpChannel::WriteTo(const unsigned char* buff, size_t buff_size, std::string remote_ip, uint16_t remote_port, uint64_t timeout)
{
    sockaddr_in send_addr     = {};
    send_addr.sin_family      = AF_INET;
    send_addr.sin_port        = htons(remote_port);
    send_addr.sin_addr.s_addr = inet_addr(remote_ip.c_str());

    co_await WriteAwaitable{this, socket_, buff, buff_size, send_addr};
    co_return SChannelResult{.state = SChannelResult::EState::CRS_SUCCESS};
}

void UdpChannel::Stop()
{
    write_await_.Stop();
    read_await_.Stop();
    closesocket(socket_);
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
    DWORD flags = 0;
    handle      = co_handle;

    iocp_data_.call_ = std::bind(&ReadAwaitable::IoCallback, this, std::placeholders::_1);

    int recv = WSARecv(socket_, &wsbuffer_, 1, nullptr, &flags, &iocp_data_.overlapped, NULL);
    DWORD dw = GetLastError();
    if (WAIT_TIMEOUT != dw && ERROR_IO_PENDING != dw)
    {
        std::string str =
            std::format("UdpChannel code error: {} \t  error-msg: {}\r\n", dw,
                GetFormatMessage(dw));
        OutputDebugString(str.c_str());
    }

    return true;
}

size_t UdpChannel::ReadAwaitable::await_resume() const
{
    return recv_len_;
}

void UdpChannel::ReadAwaitable::IoCallback(IOCP_DATA* pData)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        callback_flag_ = true;
    }

    if (pData->success_ == 0)
    {
        DWORD dw = GetLastError();
        if (WAIT_TIMEOUT != dw)
        {
            std::string str =
                std::format("UdpChannel code error: {} \t  error-msg: {}\r\n", dw,
                    GetFormatMessage(dw));
            OutputDebugString(str.c_str());
        }

        CancelIo((HANDLE) socket_);
        closesocket(socket_);
        socket_   = INVALID_SOCKET;
        recv_len_ = 0;
    }
    else
    {
        recv_len_ = (size_t) pData->bytesTransferred_;
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
// TODO: 写入的时候是否会修改buff，需要后续检查
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
    DWORD flags = 0;
    handle      = co_handle;

    iocp_data_.call_ = std::bind(&WriteAwaitable::IoCallback, this, std::placeholders::_1);

    int send = WSASendTo(socket_, &wsbuffer_, 1, nullptr, flags, (SOCKADDR*) &send_addr_,
        sizeof(send_addr_), &iocp_data_.overlapped, NULL);

    return true;
}

size_t UdpChannel::WriteAwaitable::await_resume() const
{
    return write_len_;
}

void UdpChannel::WriteAwaitable::IoCallback(IOCP_DATA* pData)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        callback_flag_ = true;
    }

    if (pData->success_ == 0)
    {
        DWORD dw = GetLastError();
        if (WAIT_TIMEOUT != dw)
        {
            std::string str =
                std::format("UdpChannel code error: {} \t  error-msg: {}\r\n", dw,
                    GetFormatMessage(dw));
            OutputDebugString(str.c_str());
        }

        CancelIo((HANDLE) socket_);
        closesocket(socket_);
        socket_    = INVALID_SOCKET;
        write_len_ = 0;
    }
    else
    {
        write_len_ = (size_t) pData->bytesTransferred_;
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