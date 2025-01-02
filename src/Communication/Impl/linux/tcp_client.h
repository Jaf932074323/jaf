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

#include "Interface/communication/i_tcp_client.h"
#include "Impl/empty_channel.h"
#include "global_timer/co_await_time.h"
#include "Interface/communication/i_channel.h"
#include "i_get_epoll_fd.h"
#include "time_head.h"
#include "util/co_coroutine.h"
#include "util/co_wait_util_stop.h"
#include <string>

namespace jaf
{
namespace comm
{

// TCP客户端
class TcpClient : public ITcpClient
{
public:
    TcpClient(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~TcpClient();

public:
    virtual void SetAddr(const std::string& remote_ip, uint16_t remote_port, const std::string& local_ip = "0.0.0.0", uint16_t local_port = 0) override;
    // 设置连接时间
    // connect_timeout 连接超时时间
    // reconnect_wait_time 重连等待时间
    virtual void SetConnectTime(uint64_t connect_timeout, uint64_t reconnect_wait_time) override;
    virtual void SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel) override;
    virtual jaf::Coroutine<void> Run() override;
    virtual void Stop() override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    void Init(void);
    jaf::Coroutine<void> Execute();

    SOCKET CreationSocket();

private:
    struct ConnectResult;
    class ConnectAwaitable;

private:
    bool run_flag_ = false;
    CoWaitUtilStop wait_stop_;

    std::shared_ptr<jaf::time::ITimer> timer_;
    jaf::ControlStartStop control_start_stop_;

    IGetCompletionPort* get_completion_port_ = nullptr;
    HANDLE completion_handle_                = nullptr;

    std::string local_ip_ = "0.0.0.0";
    uint16_t local_port_  = 0;
    std::string remote_ip_;
    uint16_t remote_port_         = 0;
    uint64_t connect_timeout_     = 5000; // 连接等待时间
    uint64_t reconnect_wait_time_ = 5000; // 重连等待时间

    std::string error_info_;

    std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel_; // 操作通道

    std::mutex channel_mutex_;
    std::shared_ptr<IChannel> empty_channel_ = std::make_shared<EmptyChannel>();
    std::shared_ptr<IChannel> channel_ = empty_channel_;
};

} // namespace comm
} // namespace jaf

#endif