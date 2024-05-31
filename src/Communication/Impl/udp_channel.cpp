#include "udp_channel.h"
#include "impl/tool/run_with_timeout.h"
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
    size_t len_ = 0; // �������ݳ��� Ϊ0ʱ��ʾ����ʧ��
    int err_    = 0; // �������
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
    SOCKET socket_           = 0; // �շ����ݵ��׽���
    AwaitableResult reslult_;

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;

    WSABUF wsbuffer_;

    DWORD dwBytes = 0;

    std::mutex mutex_;
    bool callback_flag_ = false; // �Ѿ��ص����
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
    SOCKET socket_           = 0; // �շ����ݵ��׽���
    AwaitableResult reslult_;

    sockaddr_in send_addr_ = {};

    IOCP_DATA iocp_data_;
    std::coroutine_handle<> handle;

    WSABUF wsbuffer_;

    DWORD dwBytes = 0;

    std::mutex mutex_;
    bool callback_flag_ = false; // �Ѿ��ص����
};


UdpChannel::UdpChannel(HANDLE completion_handle, SOCKET socket,
    std::string remote_ip, uint16_t remote_port,
    std::string local_ip, uint16_t local_port
    , std::shared_ptr<jaf::time::ITimer> timer)
    : completion_handle_(completion_handle)
    , socket_(socket)
    , remote_ip_(remote_ip)
    , remote_port_(remote_port)
    , local_ip_(local_ip)
    , local_port_(local_port)
    , read_await_(timer)
    , write_await_(timer)
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
    stop_flag_ = false;
    read_await_.Start();
    write_await_.Start();
    CreateIoCompletionPort((HANDLE) socket_, completion_handle_, 0, 0);
    co_return true;
}

void UdpChannel::Stop()
{
    stop_flag_ = true;
    write_await_.Stop();
    read_await_.Stop();
    closesocket(socket_);
}

Coroutine<SChannelResult> UdpChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
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
            result.state = SChannelResult::EState::CRS_UNKNOWN;
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

Coroutine<SChannelResult> UdpChannel::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    WriteAwaitable write_awaitable(this, socket_, buff, buff_size, send_addr_);
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
            CancelIo((HANDLE) socket_);
            closesocket(socket_);
            socket_      = INVALID_SOCKET;
            result.error = std::format("TcpChannel code error:{},error-msg: {}", write_result.err_, GetFormatMessage(write_result.err_));
        }
    }

    co_return result;
}

Coroutine<SChannelResult> UdpChannel::WriteTo(const unsigned char* buff, size_t buff_size, std::string remote_ip, uint16_t remote_port, uint64_t timeout)
{
    sockaddr_in send_addr     = {};
    send_addr.sin_family      = AF_INET;
    send_addr.sin_port        = htons(remote_port);
    send_addr.sin_addr.s_addr = inet_addr(remote_ip.c_str());

    WriteAwaitable write_awaitable(this, socket_, buff, buff_size, send_addr);
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
            CancelIo((HANDLE) socket_);
            closesocket(socket_);
            socket_      = INVALID_SOCKET;
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
    DWORD flags = 0;
    handle      = co_handle;
    iocp_data_.call_ = std::bind(&ReadAwaitable::IoCallback, this, std::placeholders::_1);

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
    DWORD flags = 0;
    handle      = co_handle;
    iocp_data_.call_ = std::bind(&WriteAwaitable::IoCallback, this, std::placeholders::_1);

    if(SOCKET_ERROR == WSASendTo(socket_, &wsbuffer_, 1, nullptr, flags, (SOCKADDR*) &send_addr_, sizeof(send_addr_), &iocp_data_.overlapped, NULL))
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