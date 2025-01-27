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

#include "channel_read_write_helper.h"
#include "Impl/tool/run_with_timeout.h"
#include "Log/log_head.h"
#include "head.h"
#include "util/co_wait_util_controlled_stop.h"
#include "util/finally.h"
#include <assert.h>
#include <errno.h>
#include <format>
#include <memory.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace jaf
{
namespace comm
{

ChannelReadWriteHelper::ChannelReadWriteHelper()
    : run_flag_(false)
{
}

ChannelReadWriteHelper::~ChannelReadWriteHelper()
{
}

void ChannelReadWriteHelper::Start(int socket)
{
    socket_   = socket;
    run_flag_ = true;
}

void ChannelReadWriteHelper::Stop()
{
    run_flag_ = false;
}


void ChannelReadWriteHelper::AddOperateData(std::shared_ptr<CommunData> data)
{
    {
        std::unique_lock lock_ready_operate_queue(ready_operate_queue_mutex_);
        ready_operate_queue_.push_back(data);
    }
    if (!operateable_flag_)
    {
        return;
    }
    DoOperate();
}

void ChannelReadWriteHelper::OnOperate(EpollData* data)
{
    operateable_flag_ = true;
    DoOperate();
}

void ChannelReadWriteHelper::DoOperate()
{
    operateable_flag_ = false;

    std::list<std::shared_ptr<CommunData>> finish_operate_datas;

    if (DoOperateImp(finish_operate_datas))
    {
        operateable_flag_ = true;
    }

    for (std::shared_ptr<CommunData>& finish_operate_data : finish_operate_datas)
    {
        finish_operate_data->call_();
    }

    if (!run_flag_)
    {
        std::list<std::shared_ptr<CommunData>> operate_queue;
        {
            std::unique_lock<std::mutex> lock(operate_mutex_);
            {
                operate_queue.swap(operate_queue_);
                std::unique_lock<std::mutex> lock_ready_operate_queue(ready_operate_queue_mutex_);
                operate_queue.splice(operate_queue.end(), ready_operate_queue_);
            }
        }

        for (std::shared_ptr<CommunData>& operate_data : operate_queue)
        {
            {
                std::unique_lock<std::mutex> lock_index(operate_data->mutex_);
                if (operate_data->timeout_flag_)
                {
                    continue;
                }
                operate_data->finish_flag_ = true;
            }
            operate_data->result.error = std::format("The socket was disconnected.");
            operate_data->result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            operate_data->call_();
        }
    }
}

bool ChannelReadWriteHelper::DoOperateImp(std::list<std::shared_ptr<CommunData>>& finish_operate_datas)
{
    std::unique_lock lock(operate_mutex_);
    {
        std::unique_lock lock_ready_operate_queue(ready_operate_queue_mutex_);
        operate_queue_.splice(operate_queue_.end(), ready_operate_queue_);
    }

    while (true)
    {
        if (operate_queue_.empty())
        {
            return true;
        }

        std::shared_ptr<CommunData> commun_data = operate_queue_.front();
        CommunData* p_commun_data               = commun_data.get();

        {
            std::unique_lock<std::mutex> lock_index(p_commun_data->mutex_);
            if (p_commun_data->timeout_flag_)
            {
                operate_queue_.erase(operate_queue_.begin());
                continue;
            }

            p_commun_data->DoOperate(socket_);
            if (p_commun_data->result.len == 0)
            {
                return false;
            }

            operate_queue_.erase(operate_queue_.begin());
            p_commun_data->finish_flag_ = true;
        }

        if (p_commun_data->result.len < 0)
        {
            close(socket_);
            run_flag_                   = false;
            int error_code              = errno;
            p_commun_data->result.error = std::format("epoll_ctl(), error: {}, error-msg: {}", error_code, strerror(error_code));
            p_commun_data->result.state = SChannelResult::EState::CRS_FAIL;
            finish_operate_datas.push_back(commun_data);
            return false;
        }
        p_commun_data->result.state = SChannelResult::EState::CRS_SUCCESS;
        finish_operate_datas.push_back(commun_data);

        if (p_commun_data->result.len < p_commun_data->need_len_)
        {
            return false;
        }
    }
}


RWAwaitable::RWAwaitable(ChannelReadWriteHelper& helper, std::shared_ptr<jaf::time::ITimer> timer, std::shared_ptr<CommunData> data, uint32_t timeout)
    : helper_(helper)
    , timer_(timer)
    , data_(data)
{
    CommunData* p_data = data_.get();
    p_data->call_      = [this]() { IoCallback(); };

    p_data->timeout_task_.interval = timeout;
    p_data->timeout_task_.fun      = [this, data](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {
        OnTimeout(data.get());
        data->timeout_task_.fun = [](jaf::time::ETimerResultType result_type, jaf::time::STimerTask* task) {};
    };
}

RWAwaitable::~RWAwaitable()
{
}

bool RWAwaitable::await_ready()
{
    return false;
}

bool RWAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
    handle_ = co_handle;
    helper_.AddOperateData(data_);
    timer_->StartTask(&data_->timeout_task_);
    return true;
}

void RWAwaitable::await_resume() const
{
    return;
}

void RWAwaitable::IoCallback()
{
    timer_->StopTask(&data_->timeout_task_);
    handle_.resume();
}

void RWAwaitable::OnTimeout(CommunData* p_data)
{
    {
        std::unique_lock<std::mutex> lock(p_data->mutex_);

        if (p_data->finish_flag_)
        {
            return;
        }
        p_data->timeout_flag_ = true;
    }

    p_data->result.state = SChannelResult::EState::CRS_TIMEOUT;
    p_data->result.error = std::format("timeout");

    handle_.resume();
}

} // namespace comm
} // namespace jaf

#endif