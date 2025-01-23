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

#include "tcp_channel_write.h"
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

TcpChannelWrite::TcpChannelWrite()
    : run_flag_(false)
{
}

TcpChannelWrite::~TcpChannelWrite() {}

void TcpChannelWrite::Start(int socket)
{
    socket_   = socket;
    run_flag_ = true;
}

void TcpChannelWrite::Stop()
{
    run_flag_ = false;
}

void TcpChannelWrite::AddWriteData(std::shared_ptr<WriteCommunData> write_data)
{
    {
        std::unique_lock lock_ready_write_queue(ready_write_queue_mutex_);
        ready_write_queue_.push_back(write_data);
    }
    if (!writeable_flag_)
    {
        return;
    }
    Write();
}

void TcpChannelWrite::OnWrite(EpollData* data)
{
    writeable_flag_ = true;
    Write();
}

void TcpChannelWrite::Write()
{
    writeable_flag_ = false;

    std::list<std::shared_ptr<WriteCommunData>> finish_write_datas;

    if (WriteImp(finish_write_datas))
    {
        writeable_flag_ = true;
    }

    for (std::shared_ptr<WriteCommunData>& finish_write_data : finish_write_datas)
    {
        finish_write_data->call_();
    }

    if (!run_flag_)
    {
        std::list<std::shared_ptr<WriteCommunData>> write_queue;
        {
            std::unique_lock<std::mutex> lock(write_mutex_);
            {
                write_queue.swap(write_queue_);
                std::unique_lock<std::mutex> lock_ready_write_queue(ready_write_queue_mutex_);
                write_queue.splice(write_queue.end(), ready_write_queue_);
            }
        }

        for (std::shared_ptr<WriteCommunData>& write_data : write_queue)
        {
            {
                std::unique_lock<std::mutex> lock_index(write_data->mutex_);
                if (write_data->timeout_flag_)
                {
                    continue;
                }
                write_data->finish_flag_ = true;
            }
            write_data->result.error = std::format(" The socket was disconnected.");
            write_data->result.state = SChannelResult::EState::CRS_CHANNEL_DISCONNECTED;
            write_data->call_();
        }
    }
}

bool TcpChannelWrite::WriteImp(std::list<std::shared_ptr<WriteCommunData>>& finish_write_datas)
{
    std::unique_lock lock(write_mutex_);
    {
        std::unique_lock lock_ready_write_queue(ready_write_queue_mutex_);
        write_queue_.splice(write_queue_.end(), ready_write_queue_);
    }

    while (true)
    {
        if (write_queue_.empty())
        {
            return true;
        }

        std::shared_ptr<WriteCommunData> commun_data = write_queue_.front();

        {
            std::unique_lock<std::mutex> lock_index(commun_data->mutex_);
            if (commun_data->timeout_flag_)
            {
                write_queue_.erase(write_queue_.begin());
                continue;
            }

            commun_data->result.len = ::write(socket_, commun_data->result_buf_, commun_data->need_len_);
            if (commun_data->result.len == 0)
            {
                return false;
            }

            write_queue_.erase(write_queue_.begin());
            commun_data->finish_flag_ = true;
        }

        if (commun_data->result.len < 0)
        {
            close(socket_);
            run_flag_                 = false;
            int error_code            = errno;
            commun_data->result.error = std::format("epoll_ctl(), error: {}, error-msg: {}", error_code, strerror(error_code));
            commun_data->result.state = SChannelResult::EState::CRS_FAIL;
            finish_write_datas.push_back(commun_data);
            return false;
        }
        commun_data->result.state = SChannelResult::EState::CRS_SUCCESS;
        finish_write_datas.push_back(commun_data);

        if (commun_data->result.len < commun_data->need_len_)
        {
            return false;
        }
    }
}

} // namespace comm
} // namespace jaf

#endif