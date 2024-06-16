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

// ´®¿ÚÍ¨µÀ
class SerialPortChannel : public IChannel
{
    struct AwaitableResult;
    class ReadAwaitable;
    class WriteAwaitable;

public:
    SerialPortChannel(HANDLE completion_handle, HANDLE comm_handle);
    virtual ~SerialPortChannel();

public:
    virtual Coroutine<bool> Start();
    virtual void Stop() override;
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    bool stop_flag_ = false;

    HANDLE completion_handle_ = nullptr;
    HANDLE comm_handle_;

    jaf::time::CoAwaitTime read_await_;
    jaf::time::CoAwaitTime write_await_;
};


} // namespace comm
} // namespace jaf