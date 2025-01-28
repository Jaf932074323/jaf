#pragma once
// MIT License
//
// Copyright(c) 2021 Jaf932074323
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 2024-6-16 姜安富
#ifdef _WIN32
#elif defined(__linux__)

#include "Impl/empty_channel.h"
#include "Interface/communication/i_serial_port.h"
#include "Interface/communication/i_unpack.h"
#include "Interface/i_timer.h"
#include "head.h"
#include "i_get_epoll_fd.h"
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <termios.h>

namespace jaf
{
namespace comm
{

// 串口
class SerialPort : public ISerialPort
{
public:
    SerialPort(IGetEpollFd* get_epoll_fd, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~SerialPort();

public:
    virtual void SetAddr(const std::string& comm_name, uint32_t baud_rate, uint8_t data_bit, uint8_t stop_bit, uint8_t parity) override;
    virtual void SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel) override;
    virtual jaf::Coroutine<void> Run() override;
    virtual void Stop() override;
    virtual std::shared_ptr<IChannel> GetChannel() override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    void Init(void);
    bool OpenSerialPort();
    void CloseSerialPort();

    speed_t TransitionBaudRate(uint32_t baud_rate);

private:
    std::shared_ptr<jaf::time::ITimer> timer_;

    int epoll_fd_ = -1;         // epoll描述符
    IGetEpollFd* get_epoll_fd_; // 获取epoll对象
    int file_descriptor_ = -1;

    std::string comm_;   //串口
    uint32_t baud_rate_; // 波特率
    uint8_t data_bit_;   // 数据位
    uint8_t stop_bit_;   // 停止位
    uint8_t parity_;     //校验位

    std::string error_info_;

    std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel_; // 操作通道
    std::mutex channel_mutex_;
    std::atomic<bool> run_flag_        = false;
    std::shared_ptr<IChannel> channel_ = std::make_shared<EmptyChannel>();
};

} // namespace comm
} // namespace jaf

#endif