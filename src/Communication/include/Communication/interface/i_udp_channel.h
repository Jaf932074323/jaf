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
#include "../endpoint.h"
#include "i_channel.h"
#include "util/co_coroutine.h"

namespace jaf
{
namespace comm
{

// UDP通信通道
class IUdpChannel : public IChannel
{

public:
    IUdpChannel() {}
    virtual ~IUdpChannel() {};

public:
    virtual Coroutine<SChannelResult> ReadFrom(unsigned char* buff, size_t buff_size, Endpoint* endpoint, uint64_t timeout)            = 0;
    virtual Coroutine<SChannelResult> WriteTo(const unsigned char* buff, size_t buff_size, const Endpoint* endpoint, uint64_t timeout) = 0;
};

} // namespace comm
} // namespace jaf