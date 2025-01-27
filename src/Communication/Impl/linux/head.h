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
// 2024-6-16 姜安富
#ifdef _WIN32
#elif defined(__linux__)

#include "Interface/communication/i_channel.h"
#include "Time/Interface/i_timer.h"
#include <functional>
#include <mutex>
#include <stdint.h>

namespace jaf
{
namespace comm
{

struct EpollData
{
    std::function<void(EpollData*)> call_;
    uint32_t events_;
};

struct WSABUF
{
    uint32_t len; /* the length of the buffer */
    char* buf;    /* the pointer to the buffer */
};

struct CommunData
{
    std::function<void(void)> call_;

    std::mutex mutex_;

    jaf::time::STimerTask timeout_task_;

    bool timeout_flag_ = false;
    bool finish_flag_  = false;

    int need_len_ = 0;
    unsigned char* operate_buf_ = nullptr;
    SChannelResult result;

    // 执行通讯功能
    virtual void DoOperate(int file) = 0;
};


} // namespace comm
} // namespace jaf

#endif