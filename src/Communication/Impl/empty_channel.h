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
#include "Interface/communication/i_channel.h"

namespace jaf
{
namespace comm
{

// 空的通信通道
// 在通讯接口获取通道时，若没有连接时返回，避免判空
class EmptyChannel: public IChannel
{
public:
    EmptyChannel() {}
    virtual ~EmptyChannel(){};

public:
    virtual Coroutine<bool> Start() { co_return false; };
    virtual void Stop() { return; };
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
    {
        SChannelResult result;
        result.state = SChannelResult::EState::CRS_EMPTY;
        co_return result;
    }
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
    {
        SChannelResult result;
        result.state = SChannelResult::EState::CRS_EMPTY;
        co_return result;
    }
};

// 空的通信通道
// 在通讯接口获取通道时，若没有连接时返回，避免判空
class EmptyUdpChannel: public IUdpChannel
{
public:
    EmptyUdpChannel() {}
    virtual ~EmptyUdpChannel(){};

public:
    virtual Coroutine<bool> Start() { co_return false; };
    virtual void Stop() { return; };
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout)
    {
        SChannelResult result;
        result.state = SChannelResult::EState::CRS_EMPTY;
        co_return result;
    }
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)
    {
        SChannelResult result;
        result.state = SChannelResult::EState::CRS_EMPTY;
        co_return result;
    }

    virtual Coroutine<SChannelResult> ReadFrom(unsigned char* buff, size_t buff_size, Endpoint* endpoint, uint64_t timeout)
    {
        SChannelResult result;
        result.state = SChannelResult::EState::CRS_EMPTY;
        co_return result;
    }

    virtual Coroutine<SChannelResult> WriteTo(const unsigned char* buff, size_t buff_size, Endpoint* endpoint, uint64_t timeout)
    {
        SChannelResult result;
        result.state = SChannelResult::EState::CRS_EMPTY;
        co_return result;
    }

};
} // namespace comm
} // namespace jaf