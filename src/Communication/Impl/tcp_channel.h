#pragma once
#include <memory>
#include <functional>
#include <string>
#include "Interface/communication/i_channel.h"
#include "Interface/communication/comm_struct.h"

namespace jaf
{
namespace comm
{

// TCP通道
class TcpChannel:public IChannel
{
public:
    TcpChannel(HANDLE completion_handle, SOCKET socket, std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port);
	virtual ~TcpChannel();
public:
    virtual Coroutine<bool> Start();
	virtual Coroutine<bool> Read(unsigned char* buff, size_t buff_size, size_t* recv_len) override;
	virtual Coroutine<bool> Write(const unsigned char* buff, size_t buff_size) override;
	virtual void Stop() override;

private:
    class ReadAwaitable
    {
    public:
        ReadAwaitable(TcpChannel* tcp_channel, SOCKET socket, unsigned char* buff, size_t size);
        ~ReadAwaitable();
        bool await_ready();
        bool await_suspend(std::coroutine_handle<> co_handle);
        size_t await_resume();
        void IoCallback(IOCP_DATA* pData);
    private:
        TcpChannel* tcp_channel_ = nullptr;
        SOCKET socket_ = 0; // 收发数据的套接字
        size_t recv_len_ = 0; // 接收数据长度

        IOCP_DATA iocp_data_;
        std::coroutine_handle<> handle;

        WSABUF wsbuffer_;

        DWORD dwBytes = 0;
    };

    class WriteAwaitable
    {
    public:
        WriteAwaitable(TcpChannel* tcp_channel, SOCKET socket, const unsigned char* buff, size_t size);
        ~WriteAwaitable();
        bool await_ready();
        bool await_suspend(std::coroutine_handle<> co_handle);
        size_t await_resume();
        void IoCallback(IOCP_DATA* pData);
    private:
        TcpChannel* tcp_channel_ = nullptr;
        SOCKET socket_ = 0; // 收发数据的套接字
        size_t write_len_ = 0; // 接收数据长度

        IOCP_DATA iocp_data_;
        std::coroutine_handle<> handle;

        WSABUF wsbuffer_;

        DWORD dwBytes = 0;
    };

    HANDLE completion_handle_ = nullptr;
    SOCKET socket_ = 0; // 收发数据的套接字
    std::string remote_ip_;
    uint16_t remote_port_ = 0;
    std::string local_ip_;
    uint16_t local_port_ = 0;
};



}
}