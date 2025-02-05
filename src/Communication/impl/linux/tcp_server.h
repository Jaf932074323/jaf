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
// 2025-1-23 姜安富
#ifdef _WIN32
#elif defined(__linux__)

#include "impl/empty_channel.h"
#include "interface/i_channel.h"
#include "interface/i_tcp_server.h"
#include "time/interface/i_timer.h"
#include "global_timer/co_await_time.h"
#include "head.h"
#include "i_get_epoll_fd.h"
#include "time/time_head.h"
#include "util/co_coroutine.h"
#include "util/co_wait_all_tasks_done.h"
#include "util/co_wait_util_stop.h"
#include <string>
#include <sys/socket.h>

namespace jaf
{
namespace comm
{

// TCP服务端
class TcpServer : public ITcpServer
{
public:
    TcpServer(IGetEpollFd* get_epoll_fd, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~TcpServer();

public:
    virtual void SetAddr(const Endpoint& endpoint) override;
    virtual void SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel) override;
    virtual void SetAcceptCount(size_t accept_count) override;
    virtual void SetMaxClientCount(size_t max_client_count) override;
    virtual Coroutine<RunResult> Run() override;
    virtual void Stop() override;
    virtual Coroutine<void> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    bool Init(void);
    Coroutine<void> RunSocket(int socket);

private:
    void OnListen(EpollData* data);

private:
    std::atomic<bool> run_flag_ = false;
    CoWaitUtilStop wait_stop_;

    CoWaitAllTasksDone wait_all_tasks_done_;

    std::shared_ptr<jaf::time::ITimer> timer_;

    int epoll_fd_ = -1;         // epoll描述符
    IGetEpollFd* get_epoll_fd_; // 获取epoll对象
    int listen_socket_ = 0;     // 侦听套接字
    EpollData listen_epoll_data_; // 侦听时用的通讯数据

    std::string ip_ = "0.0.0.0";
    uint16_t port_  = 0;
    Endpoint endpoint_;
    
    std::string error_info_;    

    std::function<Coroutine<void>(std::shared_ptr<IChannel> channel)> handle_channel_; // 操作通道
    size_t accept_count_     = 5;
    size_t max_client_count_ = SOMAXCONN; // 最大客户端连接数量

    std::mutex channels_mutex_;                                 // 所有通道的同步锁
    std::map<std::string, std::shared_ptr<IChannel>> channels_; // 当前连接的所有通道 key由IP和端口组成
};

} // namespace comm
} // namespace jaf
#endif