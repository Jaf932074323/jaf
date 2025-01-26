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
#include "head.h"
#include "time_head.h"
#include "util/co_wait_all_tasks_done.h"
#include <functional>
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

// TCP通道
template <typename AppendData>
class TcpChannelReadWriteHelper
{
public:
    TcpChannelReadWriteHelper();
    virtual ~TcpChannelReadWriteHelper();

    void Start(int socket);
    void Stop();

    void AddOperateData(std::shared_ptr<CommunData<AppendData>> data);
    void OnOperate(EpollData* data);

protected:
    virtual void Operate(CommunData<AppendData>* data) = 0;

private:
    void DoOperate();
    // 读取数据，结束时若缓存区还有数据则返回true
    bool DoOperateImp(std::list<std::shared_ptr<CommunData<AppendData>>>& finish_operate_datas);

protected:
    int socket_ = 0; // 收发数据的套接字

    std::atomic<bool> run_flag_         = true;  // 套接字是否已经关闭标志
    std::atomic<bool> operateable_flag_ = false; // 是否可读/写

    std::list<std::shared_ptr<CommunData<AppendData>>> ready_operate_queue_;
    std::mutex ready_operate_queue_mutex_;
    std::list<std::shared_ptr<CommunData<AppendData>>> operate_queue_;
    std::mutex operate_mutex_;
};


} // namespace comm
} // namespace jaf

#endif