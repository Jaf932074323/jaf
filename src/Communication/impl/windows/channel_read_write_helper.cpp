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

#include "channel_read_write_helper.h"
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

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

RWAwaitable::RWAwaitable(std::shared_ptr<jaf::time::ITimer> timer, std::shared_ptr<CommunData> data, uint32_t timeout)
    : timer_(timer)
    , data_(data)
{
    CommunData* p_data = data_.get();

    p_data->iocp_data_.call_ = [this, data](IOCP_DATA* iocp_data) {
        IoCallback(data.get());
        data->iocp_data_.call_ = [](IOCP_DATA* iocp_data) {};
    };

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

    CommunData* p_data = data_.get();
    if (!p_data->DoOperate())
    {
        return false;
    }

    timer_->StartTask(&data_->timeout_task_);
    return true;
}

void RWAwaitable::await_resume() const
{
    return;
}

void RWAwaitable::IoCallback(CommunData* p_data)
{
    {
        std::unique_lock<std::mutex> lock(p_data->mutex_);

        if (p_data->timeout_flag_)
        {
            return;
        }
        p_data->finish_flag_ = true;
    }
    timer_->StopTask(&data_->timeout_task_);

    p_data->result.len   = p_data->iocp_data_.bytesTransferred_;
    p_data->result.code_ = p_data->iocp_data_.err_;
    if (p_data->iocp_data_.success_)
    {
        p_data->result.state = SChannelResult::EState::CRS_SUCCESS;
    }
    else
    {
        p_data->result.state = SChannelResult::EState::CRS_FAIL;
        p_data->result.error = GetFormatMessage(p_data->iocp_data_.err_);
    }

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
    p_data->StopOperate();

    p_data->result.state = SChannelResult::EState::CRS_TIMEOUT;
    p_data->result.error = std::format("timeout");

    handle_.resume();
}

} // namespace comm
} // namespace jaf

#elif defined(__linux__)
#endif