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
#include <WinSock2.h>
#include <Windows.h>
#include <functional>

namespace jaf
{
namespace comm
{

struct IOCP_DATA
{
    OVERLAPPED overlapped   = {0};
    BOOL success_            = 0;
    DWORD err_                = 0; // 错误代码
    DWORD bytesTransferred_ = 0;
    std::function<void(IOCP_DATA*)> call_;
};

struct CommunData
{
    IOCP_DATA iocp_data_;

    std::mutex mutex_;
    bool timeout_flag_ = false;
    bool finish_flag_  = false;

    jaf::time::STimerTask timeout_task_;

    SChannelResult result;

    // 执行通讯功能
    virtual bool DoOperate() = 0;
    // 停止通讯功能
    virtual void StopOperate() = 0;
};

} // namespace comm
} // namespace jaf

#elif defined(__linux__)
#endif