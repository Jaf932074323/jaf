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
#include "global_timer/co_await_time.h"
#include "time_head.h"
#include "util/co_wait_all_tasks_done.h"
#include <functional>
#include <memory>
#include <string>
#include "head.h"

namespace jaf
{
namespace comm
{

// TCP通道
class TcpChannelWrite
{
public:
    TcpChannelWrite();
    virtual ~TcpChannelWrite();

    void Start(int socket_);
    void Stop();
    void AddWriteData(std::shared_ptr<WriteCommunData> write_data);
    void OnWrite(EpollData* data);
private:
    void Write();
    bool WriteImp(std::list<std::shared_ptr<WriteCommunData>>& finish_write_datas); // 读取数据，结束时若缓存区还有数据则返回true
private:
    int socket_   = 0;  // 收发数据的套接字
        
    std::atomic<bool> run_flag_   = true;  // 套接字是否已经关闭标志
    std::atomic<bool> writeable_flag_  = false; // 是否可写

    std::list<std::shared_ptr<WriteCommunData>> ready_write_queue_;
    std::mutex ready_write_queue_mutex_;
    std::list<std::shared_ptr<WriteCommunData>> write_queue_;
    std::mutex write_mutex_;
};


} // namespace comm
} // namespace jaf

#endif