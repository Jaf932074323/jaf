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

#include "Impl/tool/run_with_timeout.h"
#include "Log/log_head.h"
#include "head.h"
#include "tcp_channel_read_write_helper.h"
#include "util/co_wait_util_controlled_stop.h"
#include "util/finally.h"
#include <assert.h>
#include <errno.h>
#include <format>
#include <memory.h>
#include <string.h>
#include <sys/epoll.h>

namespace jaf
{
namespace comm
{

template <typename AppendData>
TcpChannelReadWriteHelper<AppendData>::TcpChannelReadWriteHelper()
    : run_flag_(false)
{
}

template <typename AppendData>
TcpChannelReadWriteHelper<AppendData>::~TcpChannelReadWriteHelper()
{
}

template <typename AppendData>
void TcpChannelReadWriteHelper<AppendData>::Start(int socket)
{
    socket_   = socket;
    run_flag_ = true;
}

template <typename AppendData>
void TcpChannelReadWriteHelper<AppendData>::Stop()
{
    run_flag_ = false;
}


template <typename AppendData>
void TcpChannelReadWriteHelper<AppendData>::AddOperateData(std::shared_ptr<CommunData<AppendData>> data)
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

template <typename AppendData>
void TcpChannelReadWriteHelper<AppendData>::OnOperate(EpollData* data)
{
    operateable_flag_ = true;
    DoOperate();
}


template <typename AppendData>
void TcpChannelReadWriteHelper<AppendData>::DoOperate()
{
    operateable_flag_ = false;

    std::list<std::shared_ptr<CommunData<AppendData>>> finish_operate_datas;

    if (DoOperateImp(finish_operate_datas))
    {
        operateable_flag_ = true;
    }

    for (std::shared_ptr<CommunData<AppendData>>& finish_operate_data : finish_operate_datas)
    {
        finish_operate_data->call_();
    }

    if (!run_flag_)
    {
        std::list<std::shared_ptr<CommunData<AppendData>>> operate_queue;
        {
            std::unique_lock<std::mutex> lock(operate_mutex_);
            {
                operate_queue.swap(operate_queue_);
                std::unique_lock<std::mutex> lock_ready_operate_queue(ready_operate_queue_mutex_);
                operate_queue.splice(operate_queue.end(), ready_operate_queue_);
            }
        }

        for (std::shared_ptr<CommunData<AppendData>>& operate_data : operate_queue)
        {
            {
                std::unique_lock<std::mutex> lock_index(operate_data->mutex_);
                if (operate_data->timeout_flag_)
                {
                    continue;
                }
                operate_data->finish_flag_ = true;
            }
            operate_data->result.error = std::format(" The socket was disconnected.");
            operate_data->result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            operate_data->call_();
        }
    }
}

template <typename AppendData>
bool TcpChannelReadWriteHelper<AppendData>::DoOperateImp(std::list<std::shared_ptr<CommunData<AppendData>>>& finish_operate_datas)
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

        std::shared_ptr<CommunData<AppendData>> commun_data = operate_queue_.front();
        CommunData<AppendData>* p_commun_data = commun_data.get();

        {
            std::unique_lock<std::mutex> lock_index(p_commun_data->mutex_);
            if (p_commun_data->timeout_flag_)
            {
                operate_queue_.erase(operate_queue_.begin());
                continue;
            }

            Operate(p_commun_data);
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
            run_flag_                 = false;
            int error_code            = errno;
            p_commun_data->result.error = std::format("epoll_ctl(), error: {}, error-msg: {}", error_code, strerror(error_code));
            p_commun_data->result.state = SChannelResult::EState::CRS_FAIL;
            finish_operate_datas.push_back(commun_data);
            return false;
        }
        p_commun_data->result.state = SChannelResult::EState::CRS_SUCCESS;
        finish_operate_datas.push_back(commun_data);

        if (p_commun_data->result.len < p_commun_data->append_data_.need_len_)
        {
            return false;
        }
    }
}

} // namespace comm
} // namespace jaf

#endif