#pragma once
#include "Interface/communication/i_unpack.h"
#include "i_iocp_user.h"
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace jaf
{
namespace comm
{

// ����
class SerialPort
{
public:
    SerialPort(uint8_t comm, uint32_t baud_rate, uint8_t data_bit, uint8_t stop_bit, uint8_t parity);
    virtual ~SerialPort();

public:
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user);
    virtual jaf::Coroutine<void> Run(HANDLE completion_handle);
    virtual void Stop();

private:
    void Init(void);
    bool OpenSerialPort();
    void CloseSerialPort();

private:
    bool run_flag_ = false;

    HANDLE completion_handle_ = nullptr;

    std::string comm_;   //����
    uint32_t baud_rate_; // ������
    uint8_t data_bit_;   // ����λ
    uint8_t stop_bit_;   // ֹͣλ
    uint8_t parity_;     //У��λ

    HANDLE comm_handle_;

    std::shared_ptr<IChannelUser> user_ = nullptr; // ͨ��ʹ����
};

} // namespace comm
} // namespace jaf