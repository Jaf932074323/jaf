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
#ifdef _WIN32

#include "serial_port_channel.h"
#include "Impl/tool/run_with_timeout.h"
#include "Log/log_head.h"
#include "channel_read_write_helper.h"
#include "util/co_wait_util_controlled_stop.h"
#include <assert.h>
#include <format>
#include <mutex>

namespace jaf
{
namespace comm
{
std::string GetFormatMessage(DWORD dw);

SerialPortChannel::SerialPortChannel(HANDLE completion_handle, HANDLE comm_handle, std::shared_ptr<jaf::time::ITimer> timer)
    : timer_(timer)
    , completion_handle_(completion_handle)
    , comm_handle_(comm_handle)
{
}

SerialPortChannel::~SerialPortChannel()
{
}

Coroutine<void> SerialPortChannel::Run()
{
    stop_flag_ = false;
    if (CreateIoCompletionPort(comm_handle_, completion_handle_, (ULONG_PTR) comm_handle_, 0) == 0)
    {
        DWORD dw        = GetLastError();
        std::string str = std::format("Communication code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
        LOG_ERROR() << str;
        co_return;
    }

    wait_stop_.Start();
    co_await wait_stop_.Wait();
    stop_flag_ = true;
    co_await wait_all_tasks_done_;

    co_return;
}

void SerialPortChannel::Stop()
{
    wait_stop_.Stop();
}

Coroutine<SChannelResult> SerialPortChannel::Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    struct ReadCommunData : public CommunData
    {
        HANDLE comm_handle_;
        WSABUF wsbuffer_{};
        // 执行通讯功能
        bool DoOperate()
        {
            if (!ReadFile(comm_handle_, wsbuffer_.buf, wsbuffer_.len, nullptr, &iocp_data_.overlapped))
            {
                result.code_ = WSAGetLastError();
                if (result.code_ != ERROR_IO_PENDING)
                {
                    result.error = GetFormatMessage(result.code_);
                    return false;
                }
            }

            return true;
        }

        // 停止通讯功能
        void StopOperate()
        {
            CancelIoEx(comm_handle_, &iocp_data_.overlapped);
        }
    };

    std::shared_ptr<ReadCommunData> commun_data = std::make_shared<ReadCommunData>();
    commun_data->comm_handle_                   = comm_handle_;
    commun_data->wsbuffer_.len                  = (ULONG) buff_size;
    commun_data->wsbuffer_.buf                  = (char*) buff;
    RWAwaitable read_awaitable(timer_, commun_data, timeout);
    co_await read_awaitable;

    SChannelResult& channel_result = commun_data->result;

    if (channel_result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        co_return channel_result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    if (channel_result.state == SChannelResult::EState::CRS_TIMEOUT)
    {
        co_return channel_result;
    }

    channel_result.error = std::format("SerialPortChannel code error:{},error-msg:{}", channel_result.code_, channel_result.error);
    co_return channel_result;
}

Coroutine<SChannelResult> SerialPortChannel::Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
{
    wait_all_tasks_done_.CountUp();
    FINALLY(wait_all_tasks_done_.CountDown(););

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    struct WriteCommunData : public CommunData
    {
        HANDLE comm_handle_;
        WSABUF wsbuffer_{};
        // 执行通讯功能
        bool DoOperate()
        {
            if (!WriteFile(comm_handle_, wsbuffer_.buf, wsbuffer_.len, nullptr, &iocp_data_.overlapped))
            {
                result.code_ = WSAGetLastError();
                if (result.code_ != ERROR_IO_PENDING)
                {
                    result.error = GetFormatMessage(result.code_);
                    return false;
                }
            }
            return true;
        }

        // 停止通讯功能
        void StopOperate()
        {
            CancelIoEx(comm_handle_, &iocp_data_.overlapped);
        }
    };

    std::shared_ptr<WriteCommunData> commun_data = std::make_shared<WriteCommunData>();
    commun_data->comm_handle_                    = comm_handle_;
    commun_data->wsbuffer_.len                   = (ULONG) buff_size;
    commun_data->wsbuffer_.buf                   = (char*) buff;
    RWAwaitable write_awaitable(timer_, commun_data, timeout);
    co_await write_awaitable;

    SChannelResult& channel_result = commun_data->result;

    if (channel_result.state == SChannelResult::EState::CRS_SUCCESS)
    {
        co_return channel_result;
    }

    if (stop_flag_)
    {
        co_return SChannelResult{.state = SChannelResult::EState::CRS_CHANNEL_END};
    }

    if (channel_result.state == SChannelResult::EState::CRS_TIMEOUT)
    {
        co_return channel_result;
    }

    channel_result.error = std::format("SerialPortChannel code error:{},error-msg:{}", channel_result.code_, channel_result.error);
    co_return channel_result;
}

} // namespace comm
} // namespace jaf

#elif defined(__linux__)
#endif