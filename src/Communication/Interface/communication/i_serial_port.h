#pragma once
#include "Interface/communication/i_channel_user.h"
#include "util/co_coroutine.h"
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

// ´®¿Ú
class ISerialPort
{
public:
    virtual ~ISerialPort() {};

public:
    virtual void SetAddr(uint8_t comm, uint32_t baud_rate, uint8_t data_bit, uint8_t stop_bit, uint8_t parity) = 0;
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user) = 0;
    virtual jaf::Coroutine<void> Run() = 0;
    virtual void Stop() = 0;
};

} // namespace comm
} // namespace jaf