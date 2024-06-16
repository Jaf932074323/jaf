#pragma once
#include "Interface/communication/i_channel_user.h"
#include "Interface/communication/i_serial_port.h"
#include "Interface/communication/i_unpack.h"
#include "iocp_head.h"
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace jaf
{
namespace comm
{

// 串口
class SerialPort : public ISerialPort
{
public:
    SerialPort(IGetCompletionPort* get_completion_port);
    virtual ~SerialPort();

public:
    virtual void SetAddr(uint8_t comm, uint32_t baud_rate, uint8_t data_bit, uint8_t stop_bit, uint8_t parity) override;
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user) override;
    virtual jaf::Coroutine<void> Run() override;
    virtual void Stop() override;

private:
    void Init(void);
    bool OpenSerialPort();
    void CloseSerialPort();

private:
    bool run_flag_ = false;

    IGetCompletionPort* get_completion_port_ = nullptr;
    HANDLE completion_handle_                = nullptr;

    std::string comm_;   //串口
    uint32_t baud_rate_; // 波特率
    uint8_t data_bit_;   // 数据位
    uint8_t stop_bit_;   // 停止位
    uint8_t parity_;     //校验位

    HANDLE comm_handle_;

    std::shared_ptr<IChannelUser> user_ = nullptr; // 通道使用者
};

} // namespace comm
} // namespace jaf