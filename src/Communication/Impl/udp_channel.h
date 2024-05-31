#pragma once
#include "impl/co_await_time.h"
#include "interface/communication/comm_struct.h"
#include "interface/communication/i_channel.h"
#include <functional>
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

// TCPͨ��
class UdpChannel : public IChannel
{
public:
    UdpChannel(HANDLE completion_handle, SOCKET socket, std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port);
    virtual ~UdpChannel();

public:
    virtual Coroutine<bool> Start();
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> WriteTo(const unsigned char* buff, size_t buff_size, std::string remote_ip, uint16_t remote_port, uint64_t timeout);
    virtual void Stop() override;

private:
    class ReadAwaitable;
    class WriteAwaitable;

    HANDLE completion_handle_ = nullptr;
    SOCKET socket_            = 0; // �շ����ݵ��׽���
    std::string remote_ip_;
    uint16_t remote_port_ = 0;
    std::string local_ip_;
    uint16_t local_port_   = 0;
    sockaddr_in send_addr_ = {};

    jaf::time::CoAwaitTime read_await_;
    jaf::time::CoAwaitTime write_await_;
};


} // namespace comm
} // namespace jaf