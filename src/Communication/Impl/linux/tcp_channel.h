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
// 2024-12-28 ������
#ifdef _WIN32
#elif defined(__linux__)

#include "global_timer/co_await_time.h"
#include "Interface/communication/comm_struct.h"
#include "Interface/communication/i_channel.h"
#include "time_head.h"
#include "util/co_wait_all_tasks_done.h"
#include <functional>
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

// TCPͨ��
class TcpChannel : public IChannel
{
    struct AwaitableResult;
    class ReadAwaitable;
    class WriteAwaitable;

public:
    TcpChannel(SOCKET socket, std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~TcpChannel();

public:
    virtual Coroutine<void> Run();
    virtual void Stop() override;
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    std::atomic<bool> stop_flag_ = false;

    std::shared_ptr<jaf::time::ITimer> timer_;

    SOCKET socket_ = 0; // �շ����ݵ��׽���
    std::string remote_ip_;
    uint16_t remote_port_ = 0;
    std::string local_ip_;
    uint16_t local_port_ = 0;

    jaf::ControlStartStop control_start_stop_;
    jaf::CoWaitAllTasksDone wait_all_tasks_done_;
};


} // namespace comm
} // namespace jaf

#endif