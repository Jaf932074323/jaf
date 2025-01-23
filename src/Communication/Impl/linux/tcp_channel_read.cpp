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

#include "tcp_channel_read.h"
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

namespace jaf
{
namespace comm
{

TcpChannelRead::TcpChannelRead()
    : run_flag_(false)
{
}

TcpChannelRead::~TcpChannelRead() {}

void TcpChannelRead::Start(int socket)
{
    socket_   = socket;
    run_flag_ = true;
}

void TcpChannelRead::Stop()
{
    run_flag_ = false;
}

void TcpChannelRead::AddReadData(std::shared_ptr<CommunData> read_data)
{
    {
        std::unique_lock lock_ready_read_queue(ready_read_queue_mutex_);
        ready_read_queue_.push_back(read_data);
    }
    if (!readable_flag_)
    {
        return;
    }
    Read();
}

void TcpChannelRead::OnRead(EpollData* data)
{
    readable_flag_ = true;
    Read();
}

void TcpChannelRead::Read()
{
    readable_flag_ = false;

    std::list<std::shared_ptr<CommunData>> finish_read_datas;

    if (ReadImp(finish_read_datas))
    {
        readable_flag_ = true;
    }

    for (std::shared_ptr<CommunData>& finish_read_data : finish_read_datas)
    {
        finish_read_data->call_();
    }

    if (!run_flag_)
    {
        std::list<std::shared_ptr<CommunData>> read_queue;
        {
            std::unique_lock<std::mutex> lock(read_mutex_);
            {
                read_queue.swap(read_queue_);
                std::unique_lock<std::mutex> lock_ready_read_queue(ready_read_queue_mutex_);
                read_queue.splice(read_queue.end(), ready_read_queue_);
            }
        }

        for (std::shared_ptr<CommunData>& read_data : read_queue)
        {
            {
                std::unique_lock<std::mutex> lock_index(read_data->mutex_);
                if (read_data->timeout_flag_)
                {
                    continue;
                }
                read_data->finish_flag_ = true;
            }
            read_data->result.error = std::format(" The socket was disconnected.");
            read_data->result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            read_data->call_();
        }
    }
}

bool TcpChannelRead::ReadImp(std::list<std::shared_ptr<CommunData>>& finish_read_datas)
{
    std::unique_lock lock(read_mutex_);
    {
        std::unique_lock lock_ready_read_queue(ready_read_queue_mutex_);
        read_queue_.splice(read_queue_.end(), ready_read_queue_);
    }

    while (true)
    {
        if (read_queue_.empty())
        {
            return true;
        }

        std::shared_ptr<CommunData> commun_data = read_queue_.front();

        {
            std::unique_lock<std::mutex> lock_index(commun_data->mutex_);
            if (commun_data->timeout_flag_)
            {
                read_queue_.erase(read_queue_.begin());
                continue;
            }

            commun_data->result.len = ::read(socket_, commun_data->result_buf_, commun_data->need_len_);
            if (commun_data->result.len == 0)
            {
                return false;
            }

            read_queue_.erase(read_queue_.begin());
            commun_data->finish_flag_ = true;
        }

        if (commun_data->result.len < 0)
        {
            close(socket_);
            run_flag_                 = false;
            int error_code            = errno;
            commun_data->result.error = std::format("epoll_ctl(), error: {}, error-msg: {}", error_code, strerror(error_code));
            commun_data->result.state = SChannelResult::EState::CRS_FAIL;
            finish_read_datas.push_back(commun_data);
            return false;
        }
        commun_data->result.state = SChannelResult::EState::CRS_SUCCESS;
        finish_read_datas.push_back(commun_data);

        if (commun_data->result.len < commun_data->need_len_)
        {
            return false;
        }
    }
}

} // namespace comm
} // namespace jaf

#endif