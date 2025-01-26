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

#include "tcp_channel.h"
#include "Impl/tool/run_with_timeout.h"
#include "Log/log_head.h"
#include "head.h"
#include "tcp_channel_read_write_helper.ipp"
#include "util/co_wait_util_controlled_stop.h"
#include "util/finally.h"
#include <assert.h>
#include <errno.h>
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
        std::shared_ptr<CommunData<ReadAppendata>> read_data = std::make_shared<CommunData<ReadAppendata>>();
        read_data_                                           = read_data;

        CommunData<ReadAppendata>* p_read_data = read_data_.get();
        p_read_data->call_                     = [this]() { IoCallback(); };
        p_read_data->append_data_.need_len_    = buff_size;
        p_read_data->append_data_.result_buf_  = buff;

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
        tcp_channel_->tcp_channel_read_.AddOperateData(read_data_);
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

    void OnTimeout(CommunData<ReadAppendata>* p_read_data)
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
        p_read_data->result.error = std::format("timeout");

        handle_.resume();
    }

    const CommunData<ReadAppendata>* ReadData()
    {
        return read_data_.get();
    }

private:
    TcpChannel* tcp_channel_ = nullptr;

    EpollData iocp_data_;
    std::coroutine_handle<> handle_;

    unsigned char* buff_;
    size_t buff_size_;

    std::shared_ptr<CommunData<ReadAppendata>> read_data_;
};

class TcpChannel::WriteAwaitable
{
public:
    WriteAwaitable(TcpChannel* tcp_channel, const unsigned char* buff, size_t buff_size, uint32_t timeout)
        : tcp_channel_(tcp_channel)
        , buff_(buff)
        , buff_size_(buff_size)
    {
        std::shared_ptr<CommunData<WriteAppendata>> write_data = std::make_shared<CommunData<WriteAppendata>>();
        write_data_                                            = write_data;

        CommunData<WriteAppendata>* p_write_data = write_data_.get();
        p_write_data->call_                      = [this]() { IoCallback(); };
        p_write_data->append_data_.need_len_     = buff_size;
        p_write_data->append_data_.result_buf_   = buff;

        p_write_data->timeout_task_.interval = timeout;
        p_write_data->timeout_task_.fun      = [this, write_data](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {
            OnTimeout(write_data.get());
            write_data->timeout_task_.fun = [](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {};
        };
    }

    ~WriteAwaitable()
    {
    }

    bool await_ready()
    {
        return false;
    }

    bool await_suspend(std::coroutine_handle<> co_handle)
    {
        handle_ = co_handle;
        tcp_channel_->tcp_channel_write_.AddOperateData(write_data_);
        tcp_channel_->timer_->StartTask(&write_data_->timeout_task_);
        return true;
    }

    void await_resume() const
    {
        return;
    }

    void IoCallback()
    {
        tcp_channel_->timer_->StopTask(&write_data_->timeout_task_);
        handle_.resume();
    }

    void OnTimeout(CommunData<WriteAppendata>* p_write_data)
    {
        {
            std::unique_lock<std::mutex> lock(p_write_data->mutex_);

            if (p_write_data->finish_flag_)
            {
                return;
            }
            p_write_data->timeout_flag_ = true;
        }

        p_write_data->result.state = SChannelResult::EState::CRS_TIMEOUT;
        p_write_data->result.error = std::format("timeout");

        handle_.resume();
    }

    const CommunData<WriteAppendata>* ReadData()
    {
        return write_data_.get();
    }


private:
    TcpChannel* tcp_channel_ = nullptr;

    EpollData iocp_data_;
    std::coroutine_handle<> handle_;

    const unsigned char* buff_;
    size_t buff_size_;

    std::shared_ptr<CommunData<WriteAppendata>> write_data_;
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
    tcp_channel_write_.Start(socket_);

    control_start_stop_.Start();
    co_await jaf::CoWaitUtilControlledStop(control_start_stop_);
    co_await wait_all_tasks_done_;
}

void TcpChannel::Stop()
{
    stop_flag_ = true;
    close(socket_);
    tcp_channel_read_.Stop();
    tcp_channel_write_.Stop();
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

    const CommunData<ReadAppendata>* read_data = read_awaitable.ReadData();

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

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    WriteAwaitable write_awaitable(this, buff, buff_size, timeout);
    co_await write_awaitable;

    const CommunData<WriteAppendata>* write_data = write_awaitable.ReadData();

    if (write_data->result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        co_return write_data->result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    co_return write_data->result;
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
        tcp_channel_read_.OnOperate(data);
    }

    if (data->events_ & EPOLLOUT)
    {
        tcp_channel_write_.OnOperate(data);
    }
}

void TcpChannel::TcpChannelReadHelper::Operate(CommunData<ReadAppendata>* data)
{
    data->result.len = ::read(socket_, data->append_data_.result_buf_, data->append_data_.need_len_);
}


void TcpChannel::TcpChannelWriteHelper::Operate(CommunData<WriteAppendata>* data)
{
    data->result.len = ::write(socket_, data->append_data_.result_buf_, data->append_data_.need_len_);
}

} // namespace comm
} // namespace jaf

#endif