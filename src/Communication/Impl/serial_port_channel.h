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

// 串口通道
class SerialPortChannel:public IChannel
{
public:
    SerialPortChannel(HANDLE completion_handle, HANDLE comm_handle);
	virtual ~SerialPortChannel();
public:
    virtual Coroutine<bool> Start();
	virtual void Stop() override;
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    class ReadAwaitable
    {
    public:
        ReadAwaitable(SerialPortChannel* serial_port_channel, unsigned char* buff, size_t size);
        ~ReadAwaitable();
        bool await_ready();
        bool await_suspend(std::coroutine_handle<> co_handle);
        size_t await_resume();
        void IoCallback(IOCP_DATA* pData);
    private:
        SerialPortChannel* serial_port_channel_ = nullptr;
        size_t recv_len_ = 0; // 接收数据长度

        IOCP_DATA iocp_data_;
        std::coroutine_handle<> handle;

        WSABUF wsbuffer_;

        DWORD dwBytes = 0;
    };

    class WriteAwaitable
    {
    public:
        WriteAwaitable(SerialPortChannel* serial_port_channel, const unsigned char* buff, size_t size);
        ~WriteAwaitable();
        bool await_ready();
        bool await_suspend(std::coroutine_handle<> co_handle);
        size_t await_resume();
        void IoCallback(IOCP_DATA* pData);
    private:
        SerialPortChannel* serial_port_channel_ = nullptr;
        size_t write_len_ = 0; // 接收数据长度

        sockaddr_in send_addr_ = {};

        IOCP_DATA iocp_data_;
        std::coroutine_handle<> handle;

        WSABUF wsbuffer_;

        DWORD dwBytes = 0;
    };

    HANDLE completion_handle_ = nullptr;
    HANDLE comm_handle_;
};



}
}