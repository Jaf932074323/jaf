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
// 2024-12-28 姜安富
#ifdef _WIN32
#elif defined(__linux__)

#include "Interface/communication/comm_struct.h"
#include "Interface/communication/i_channel.h"
#include "channel_read_write_helper.h"
#include "global_timer/co_await_time.h"
#include "head.h"
#include "time_head.h"
#include "util/co_wait_all_tasks_done.h"
#include "util/co_wait_util_stop.h"
#include <functional>
#include <memory>
#include <string>
#include "run_result.h"

namespace jaf
{
namespace comm
{

// TCP通道
class Channel : public IChannel
{
public:
    Channel(int file_descriptor, int epoll_fd, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~Channel();

public:
    virtual Coroutine<RunResult> Run();
    virtual void Stop() override;
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout) override;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) override;

private:
    void OnEpoll(EpollData* data);

protected:
    std::atomic<bool> stop_flag_ = true;
    CoWaitUtilStop wait_stop_;
    std::string finish_reason_;

    std::shared_ptr<jaf::time::ITimer> timer_;

    int file_descriptor_   = 0;  // 读写数据的文件描述符
    int epoll_fd_ = -1; // epoll描述符

    jaf::CoWaitAllTasksDone wait_all_tasks_done_;

    EpollData epoll_data_; // 连接时用的通讯数据

    ChannelReadWriteHelper read_helper_;
    ChannelReadWriteHelper write_helper_;
};


} // namespace comm
} // namespace jaf

#endif