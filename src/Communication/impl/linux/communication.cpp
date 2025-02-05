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

#include "communication.h"
#include "time/timer.h"
#include "define_constant.h"
#include "log/log_head.h"
#include "serial_port.h"
#include "tcp_client.h"
#include "tcp_server.h"
#include "udp.h"
#include "util/finally.h"
#include "util/simple_thread_pool.h"
#include <assert.h>
#include <errno.h>
#include <format>
#include <functional>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

namespace jaf
{
namespace comm
{

Communication::Communication(std::shared_ptr<IThreadPool> thread_pool, std::shared_ptr<jaf::time::ITimer> timer)
    : thread_pool_(thread_pool == nullptr ? std::make_shared<SimpleThreadPool>() : thread_pool)
    , timer_(timer == nullptr ? std::make_shared<time::Timer>() : timer)
    , get_epoll_fd_(this)
{
}

Communication::~Communication()
{
}

jaf::Coroutine<RunResult> Communication::Run()
{
    assert(!run_flag_);
    run_flag_ = true;

    wait_stop_.Start();
    wait_work_thread_finish_.Start(1);

    if (!CreateEpoll())
    {
        run_flag_ = false;
        co_return error_info_;
    }

    CreateWorkThread();

    co_await wait_stop_.Wait();

    run_flag_ = false;

    assert(stop_fd_ >= 0);
    uint64_t counter = 1;
    int result       = ::write(stop_fd_, &counter, sizeof(uint64_t));

    co_await wait_work_thread_finish_;

    close(stop_fd_);
    stop_fd_ = -1;
    close(funs_fd_);
    funs_fd_ = -1;
    close(epoll_fd_);
    epoll_fd_ = -1;

    co_return true;
}

void Communication::Stop()
{
    wait_stop_.Stop();
}

std::shared_ptr<ITcpServer> Communication::CreateTcpServer()
{
    std::shared_ptr<TcpServer> server = std::make_shared<TcpServer>(&get_epoll_fd_, timer_);
    return std::static_pointer_cast<ITcpServer>(server);
}

std::shared_ptr<ITcpClient> Communication::CreateTcpClient()
{
    std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(&get_epoll_fd_, timer_);
    return std::static_pointer_cast<ITcpClient>(client);
}

std::shared_ptr<IUdp> Communication::CreateUdp()
{
    std::shared_ptr<Udp> udp = std::make_shared<Udp>(&get_epoll_fd_, timer_);
    return std::static_pointer_cast<IUdp>(udp);
}

std::shared_ptr<ISerialPort> Communication::CreateSerialPort()
{
    std::shared_ptr<SerialPort> serial_port = std::make_shared<SerialPort>(&get_epoll_fd_, timer_);
    return std::static_pointer_cast<ISerialPort>(serial_port);
}

bool Communication::CreateEpoll()
{
    if ((epoll_fd_ = epoll_create(1)) < 0)
    {
        error_info_ = std::format("Communication CreateEpoll() create epoll error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
        return false;
    }

    // 创建停止事件描述符，用于在停止时通知停止
    stop_fd_ = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE);
    if (stop_fd_ < 0)
    {
        error_info_ = std::format("Communication CreateEpoll() create stop_fd_ error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
        return false;
    }

    epoll_event stop_ = {0, {0}};
    stop_.events      = EPOLLIN | EPOLLERR;
    stop_.data.ptr    = &stop_comm_data_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, stop_fd_, &stop_) < 0)
    {
        error_info_ = std::format("Communication CreateEpoll() epoll_ctl stop_fd_ error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
        return false;
    }

    // 创建执行功能事件描述符
    funs_fd_ = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (funs_fd_ < 0)
    {
        error_info_ = std::format("Communication CreateEpoll() create funs_fd_ error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
        return false;
    }

    epoll_event funs_event = {0, {0}};
    funs_event.events      = EPOLLIN | EPOLLERR;
    funs_event.data.ptr    = &funs_data_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, funs_fd_, &funs_event) < 0)
    {
        error_info_ = std::format("Communication CreateEpoll() epoll_ctl funs_event error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
        return false;
    }

    return true;
}

void Communication::CreateWorkThread()
{
    thread_pool_->Post(std::bind(&Communication::WorkThreadRun, this));
}

void Communication::WorkThreadRun()
{
    FINALLY(wait_work_thread_finish_.Notify(););

    assert(epoll_fd_ >= 0);
    const int events_size = 10;
    epoll_event events[events_size];
    while (run_flag_)
    {
        //获取已经准备好的描述符事件
        int events_wait_count = epoll_wait(epoll_fd_, events, events_size, -1);
        if (events_wait_count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            std::string err = std::format("OnConnect(): connect error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
            return;
        }
        for (int i = 0; i < events_wait_count; ++i)
        {
            EpollData* comm_data = (EpollData*) events[i].data.ptr;
            comm_data->events_   = events[i].events;
            comm_data->call_(comm_data);
        }
    }
}

} // namespace comm
} // namespace jaf

#endif