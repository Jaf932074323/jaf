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

#include "Impl/empty_channel.h"
#include "interface/i_udp.h"
#include "i_get_completion_port.h"
#include "time/time_head.h"
#include "util/co_wait_util_stop.h"
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <winsock2.h>

namespace jaf
{
namespace comm
{

class Udp : public IUdp
{
public:
    Udp(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~Udp();

public:
    virtual void SetAddr(const Endpoint& remote_endpoint, const Endpoint& local_endpoint = Endpoint("0.0.0.0", 0)) override;
    virtual void SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IUdpChannel> channel)> handle_channel) override;
    virtual Coroutine<RunResult> Run() override;
    virtual void Stop() override;
    virtual std::shared_ptr<IUdpChannel> GetChannel() override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> WriteTo(const unsigned char* buff, size_t buff_size, const Endpoint* endpoint, uint64_t timeout) override;

private:
    SOCKET CreateSocket(void);
    Coroutine<void> RunSocket(SOCKET the_socket);

private:
    IGetCompletionPort* get_completion_port_ = nullptr;
    HANDLE completion_handle_                = nullptr;

    std::shared_ptr<jaf::time::ITimer> timer_;

    Endpoint remote_endpoint_;
    Endpoint local_endpoint_;
    std::string local_ip_  = "0.0.0.0";
    uint16_t local_port_   = 0;
    std::string remote_ip_ = "0.0.0.0";
    uint16_t remote_port_  = 0;

    std::string error_info_;

    std::function<Coroutine<void>(std::shared_ptr<IUdpChannel> channel)> handle_channel_; // 操作通道
    std::mutex channel_mutex_;
    std::shared_ptr<IUdpChannel> channel_ = std::make_shared<EmptyUdpChannel>();

    std::atomic<bool> run_flag_           = false;
    CoWaitUtilStop wait_stop_;
};

} // namespace comm
} // namespace jaf

#elif defined(__linux__)
#endif