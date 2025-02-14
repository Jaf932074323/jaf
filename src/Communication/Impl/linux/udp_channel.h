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
// 2024-12-28 姜安富
#ifdef _WIN32
#elif defined(__linux__)

#include "interface/i_udp_channel.h"
#include "channel_read_write_helper.h"
#include "endpoint.h"
#include "global_timer/co_await_time.h"
#include "head.h"
#include "time/time_head.h"
#include "util/co_wait_all_tasks_done.h"
#include "util/co_wait_util_stop.h"
#include <functional>
#include <memory>
#include <string>
#include "run_result.h"

namespace jaf
{
namespace comm
{

// TCP通道
class UdpChannel : public IUdpChannel
{
    class ReadAwaitable;
    class WriteAwaitable;

public:
    UdpChannel(int socket, int epoll_fd, const Endpoint& remote_endpoint, const Endpoint& local_endpoint, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~UdpChannel();

public:
    virtual Coroutine<RunResult> Run();
    virtual void Stop() override;
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> ReadFrom(unsigned char* buff, size_t buff_size, Endpoint* endpoint, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> WriteTo(const unsigned char* buff, size_t buff_size, const Endpoint* endpoint, uint64_t timeout) override;

private:
    void OnEpoll(EpollData* data);

private:
    std::atomic<bool> stop_flag_ = true;
    std::string finish_reason_;

    std::shared_ptr<jaf::time::ITimer> timer_;

    int socket_   = 0;  // 收发数据的套接字
    int epoll_fd_ = -1; // epoll描述符

    Endpoint remote_endpoint_;
    Endpoint local_endpoint_;

    CoWaitUtilStop wait_stop_;
    jaf::CoWaitAllTasksDone wait_all_tasks_done_;

    EpollData epoll_data_; // 连接时用的通讯数据

    std::atomic<bool> close_flag_   = true;  // 套接字是否已经关闭标志
    std::atomic<bool> write_status_ = false; // 是否可写

    ChannelReadWriteHelper read_helper_;
    ChannelReadWriteHelper write_helper_;
};


} // namespace comm
} // namespace jaf

#endif