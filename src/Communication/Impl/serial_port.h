#pragma once
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <mutex>
#include "i_iocp_user.h"
#include "Interface/communication/i_unpack.h"

namespace jaf
{
namespace comm
{

// 串口
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

    jaf::Coroutine<void> Run();

private:
    bool run_flag_ = false;

	HANDLE completion_handle_ = nullptr;
	SOCKET socket_ = 0; // 侦听套接字

	std::string comm_; //串口
    uint32_t baud_rate_; // 波特率
    uint8_t data_bit_; // 数据位
    uint8_t stop_bit_; // 停止位
    uint8_t parity_; //校验位

    HANDLE comm_handle_;

    std::shared_ptr<IChannelUser> user_ = nullptr; // 通道使用者

};

}
}