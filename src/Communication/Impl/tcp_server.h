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
#include "Interface/communication/i_tcp_server.h"
#include "Interface/i_timer.h"
#include "interface/communication/i_unpack.h"
#include "iocp_head.h"
#include "util/co_coroutine.h"
#include "util/co_wait_all_tasks_done.h"
#include "util/control_start_stop.h"
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <string>
#include <winsock2.h>

namespace jaf
{
namespace comm
{

// TCP服务端
class TcpServer : public ITcpServer
{
public:
    TcpServer(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~TcpServer();

public:
    virtual void SetAddr(const std::string& ip, uint16_t port) override;
    virtual void SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel) override;
    virtual void SetAcceptCount(size_t accept_count) override;
    virtual void SetMaxClientCount(size_t max_client_count) override;
    virtual Coroutine<void> Run() override;
    virtual void Stop() override;
    virtual Coroutine<void> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    void Init(void);
    Coroutine<void> Accept();
    Coroutine<void> RunSocket(SOCKET socket);

private:
    struct AcceptAwaitableResult;
    class AcceptAwaitable;

private:
    std::atomic<bool> run_flag_ = false;

    jaf::ControlStartStop control_start_stop_;
    CoWaitAllTasksDone wait_all_tasks_done_;

    std::shared_ptr<jaf::time::ITimer> timer_;

    IGetCompletionPort* get_completion_port_ = nullptr;
    HANDLE completion_handle_                = nullptr;
    SOCKET listen_socket_                    = 0; // 侦听套接字

    std::string ip_ = "0.0.0.0";
    uint16_t port_  = 0;

    std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel_; // 操作通道
    size_t accept_count_     = 5;
    size_t max_client_count_ = SOMAXCONN; // 最大客户端连接数量

    std::mutex channels_mutex_;                                 // 所有通道的同步锁
    std::map<std::string, std::shared_ptr<IChannel>> channels_; // 当前连接的所有通道 key由IP和端口
};

} // namespace comm
} // namespace jaf