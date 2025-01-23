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

#include "tcp_channel.h"
#include "Impl/tool/run_with_timeout.h"
#include "Log/log_head.h"
#include "head.h"
#include "util/co_wait_util_controlled_stop.h"
#include "util/finally.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <format>
#include <memory.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace jaf
{
namespace comm
{

struct TcpChannel::AwaitableResult
{
    size_t len_   = 0;
    bool success_ = false;
    std::string error_info_;
};
class TcpChannel::ReadAwaitable
{
public:
    ReadAwaitable(TcpChannel* tcp_channel, unsigned char* buff, size_t buff_size, uint32_t timeout)
        : tcp_channel_(tcp_channel)
        , buff_(buff)
        , buff_size_(buff_size)
    {
        std::shared_ptr<CommunData> read_data = std::make_shared<CommunData>();
        read_data_                            = read_data;

        CommunData* p_read_data  = read_data_.get();
        p_read_data->call_       = [this]() { IoCallback(); };
        p_read_data->need_len_   = buff_size;
        p_read_data->result_buf_ = buff;

        p_read_data->timeout_task_.interval = timeout;
        p_read_data->timeout_task_.fun      = [this, read_data](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {
            OnTimeout(read_data.get());
            read_data->timeout_task_.fun = [](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {};
        };
    }

    ~ReadAwaitable()
    {
    }

    bool await_ready()
    {
        return false;
    }

    bool await_suspend(std::coroutine_handle<> co_handle)
    {
        handle_ = co_handle;
        tcp_channel_->tcp_channel_read_.AddReadData(read_data_);
        tcp_channel_->timer_->StartTask(&read_data_->timeout_task_);
        return true;
    }

    void await_resume() const
    {
        return;
    }

    void IoCallback()
    {
        tcp_channel_->timer_->StopTask(&read_data_->timeout_task_);
        handle_.resume();
    }


    void OnTimeout(CommunData* p_read_data)
    {
        {
            std::unique_lock<std::mutex> lock(p_read_data->mutex_);

            if (p_read_data->finish_flag_)
            {
                return;
            }
            p_read_data->timeout_flag_ = true;
        }

        p_read_data->result.state = SChannelResult::EState::CRS_TIMEOUT;
        p_read_data->result.error = std::format("Read timeout");

        handle_.resume();
    }

    const CommunData* ReadData()
    {
        return read_data_.get();
    }

private:
    TcpChannel* tcp_channel_ = nullptr;

    EpollData iocp_data_;
    std::coroutine_handle<> handle_;

    unsigned char* buff_;
    size_t buff_size_;

    std::shared_ptr<CommunData> read_data_;
};

class TcpChannel::WriteAwaitable
{
public:
    WriteAwaitable(TcpChannel* tcp_channel, int socket, int epoll_fd, const unsigned char* buff, size_t size);
    ~WriteAwaitable();
    bool await_ready();
    bool await_suspend(std::coroutine_handle<> co_handle);
    AwaitableResult await_resume() const;
    void IoCallback(EpollData* pData);
    void Stop();

private:
    TcpChannel* tcp_channel_ = nullptr;
    int socket_              = 0;  // �շ����ݵ��׽���
    int epoll_fd_            = -1; // epoll������
    AwaitableResult reslult_;

    EpollData iocp_data_;
    std::coroutine_handle<> handle;

    WSABUF wsbuffer_;

    std::mutex mutex_;
    bool callback_flag_ = false; // �Ѿ��ص����?
};

TcpChannel::TcpChannel(int socket, int epoll_fd, std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port, std::shared_ptr<jaf::time::ITimer> timer)
    : socket_(socket)
    , epoll_fd_(epoll_fd)
    , remote_ip_(remote_ip)
    , remote_port_(remote_port)
    , local_ip_(local_ip)
    , local_port_(local_port)
    , timer_(timer)
{
    close_flag_ = false;
    int flags   = fcntl(socket, F_GETFL);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

TcpChannel::~TcpChannel() {}

Coroutine<void> TcpChannel::Run()
{
    stop_flag_ = false;

    epoll_data_.call_ = [this](EpollData* pData) { OnEpoll(pData); };

    struct epoll_event ev;
    ev.events   = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;
    ev.data.ptr = &epoll_data_;
    int ret     = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_, &ev);
    if (ret == -1)
    {
        close(socket_);
        std::string str = std::format("epoll_ctl(): error: {} \t  error-msg: {}\r\n", errno, strerror(errno));
        stop_flag_      = true;
        co_return;
    }

    tcp_channel_read_.Start(socket_);

    control_start_stop_.Start();
    co_await jaf::CoWaitUtilControlledStop(control_start_stop_);
    co_await wait_all_tasks_done_;
}

void TcpChannel::Stop()
{
    stop_flag_ = true;
    close(socket_);
    tcp_channel_read_.Stop();
    control_start_stop_.Stop();
}

Coroutine<SChannelResult> TcpChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    ReadAwaitable read_awaitable(this, buff, buff_size, timeout);
    co_await read_awaitable;

    const CommunData* read_data = read_awaitable.ReadData();

    if (read_data->result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        co_return read_data->result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    co_return read_data->result;
}

Coroutine<SChannelResult> TcpChannel::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    SChannelResult result;
    if (stop_flag_)
    {
        result.state = SChannelResult::EState::CRS_CHANNEL_END;
        co_return result;
    }

    WriteAwaitable write_awaitable(this, socket_, epoll_fd_, buff, buff_size);
    RunWithTimeout run_with_timeout(control_start_stop_, write_awaitable, timeout, timer_);
    co_await run_with_timeout.Run();

    const AwaitableResult& write_result = run_with_timeout.Result();
    if (run_with_timeout.IsTimeout())
    {
        result.state = SChannelResult::EState::CRS_TIMEOUT;
    }
    result.len = write_result.len_;

    if (result.len != 0)
    {
        result.state = SChannelResult::EState::CRS_SUCCESS;
        co_return result;
    }

    if (stop_flag_)
    {
        result.state = SChannelResult::EState::CRS_CHANNEL_END;
        co_return result;
    }

    // if (result.state != SChannelResult::EState::CRS_TIMEOUT)
    // {
    //     if (write_result.err_ == 0)
    //     {
    //         result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
    //         result.error = "The connection has been disconnected";
    //     }
    //     else
    //     {
    //         result.state = SChannelResult::EState::CRS_FAIL;
    //         result.code_ = write_result.err_;
    //         close(socket_);
    //         result.error = std::format("TcpChannel code error:{},error-msg: {}", write_result.err_, strerror(write_result.err_));
    //     }
    // }

    co_return result;
}

void TcpChannel::OnEpoll(EpollData* data)
{
    if (data->events_ & EPOLLIN)
    {
        char buff[1];
        if (0 == ::recv(socket_, buff, 1, MSG_PEEK))
        {
            finish_reason_ = std::format("Connection closed");
            Stop();
            return;
        }
        tcp_channel_read_.OnRead(data);
    }

    if (data->events_ & EPOLLOUT)
    {
        OnWrite(data);
    }
}

void TcpChannel::OnWrite(EpollData* data)
{
}

TcpChannel::WriteAwaitable::WriteAwaitable(TcpChannel* tcp_channel, int socket, int epoll_fd, const unsigned char* buff, size_t size)
    : tcp_channel_(tcp_channel)
    , socket_(socket)
    , epoll_fd_(epoll_fd)
    , wsbuffer_{.len = (uint32_t) size, .buf = (char*) buff}
{
}

TcpChannel::WriteAwaitable::~WriteAwaitable()
{
}

bool TcpChannel::WriteAwaitable::await_ready()
{
    return false;
}

bool TcpChannel::WriteAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    handle           = co_handle;
    iocp_data_.call_ = [this](EpollData* pData) { IoCallback(pData); };

    struct epoll_event ev;
    ev.events   = EPOLLOUT;
    ev.data.ptr = &iocp_data_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_, &ev);
    return true;
}

TcpChannel::AwaitableResult TcpChannel::WriteAwaitable::await_resume() const
{
    return reslult_;
}

void TcpChannel::WriteAwaitable::IoCallback(EpollData* pData)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        callback_flag_ = true;
    }

    int buflen = ::write(socket_, wsbuffer_.buf, wsbuffer_.len); // TODO: ��������δ������ȫ�����?

    if (buflen < 0)
    {
        reslult_.len_ = 0;
    }
    else
    {
        reslult_.len_ = (size_t) buflen;
    }

    handle.resume();
}

void TcpChannel::WriteAwaitable::Stop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback_flag_)
    {
        return;
    }
    callback_flag_ = true;

    struct epoll_event ev;
    ev.events   = EPOLLIN;
    ev.data.ptr = &iocp_data_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket_, &ev);
}

} // namespace comm
} // namespace jaf

#endif