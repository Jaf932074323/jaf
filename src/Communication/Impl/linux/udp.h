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
#include "Interface/communication/i_channel.h"
#include "Interface/communication/i_udp.h"
#include "Interface/i_timer.h"
#include "global_timer/co_await_time.h"
#include "head.h"
#include "i_get_epoll_fd.h"
#include "time_head.h"
#include "util/co_coroutine.h"
#include "util/co_wait_all_tasks_done.h"
#include "util/co_wait_util_stop.h"
#include <string>
#include <sys/socket.h>
#include <atomic>

namespace jaf
{
namespace comm
{

class Udp : public IUdp
{
public:
    Udp(IGetEpollFd* get_epoll_fd, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~Udp();

public:
    virtual void SetAddr(const std::string& local_ip, uint16_t local_port, const std::string& remote_ip, uint16_t remote_port) override;
    virtual void SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IUdpChannel> channel)> handle_channel) override;
    virtual Coroutine<void> Run() override;
    virtual void Stop() override;
    virtual std::shared_ptr<IUdpChannel> GetChannel() override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    bool Init(void);

private:
    int epoll_fd_ = -1;         // epoll描述符
    IGetEpollFd* get_epoll_fd_; // 获取epoll对象
    int socket_ = -1;
    
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
    std::atomic<bool> run_flag_        = false;
    std::shared_ptr<IUdpChannel> channel_ = std::make_shared<EmptyUdpChannel>();
};

} // namespace comm
} // namespace jaf

#endif