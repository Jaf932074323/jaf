#pragma once
#include "Interface/communication/comm_struct.h"
#include "Interface/communication/i_channel.h"
#include "time_head.h"
#include <functional>
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

// TCP通道
class TcpChannel : public IChannel
{
public:
    TcpChannel(SOCKET socket, std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port, std::shared_ptr<jaf::time::Timer> timer = nullptr);
    virtual ~TcpChannel();

public:
    virtual Coroutine<bool> Start() override;
    virtual void Stop() override;
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    class ReadAwaitable;
    class WriteAwaitable;

    std::shared_ptr<jaf::time::ITimer> timer_;

    SOCKET socket_ = 0; // 收发数据的套接字
    std::string remote_ip_;
    uint16_t remote_port_ = 0;
    std::string local_ip_;
    uint16_t local_port_ = 0;
};


} // namespace comm
} // namespace jaf