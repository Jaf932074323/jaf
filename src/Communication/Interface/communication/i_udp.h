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
#include "Interface/communication/i_unpack.h"
#include "endpoint.h"
#include "util/co_coroutine.h"
#include <functional>
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

class IUdp
{
public:
    virtual ~IUdp() {};

public:
    virtual void SetAddr(const Endpoint& remote_endpoint, const Endpoint& local_endpoint = Endpoint("0.0.0.0", 0))     = 0;
    virtual void SetHandleChannel(std::function<Coroutine<void>(std::shared_ptr<IUdpChannel> channel)> handle_channel) = 0;
    virtual Coroutine<void> Run()                                                                                      = 0;
    virtual void Stop()                                                                                                = 0;
    virtual std::shared_ptr<IUdpChannel> GetChannel()                                                                  = 0;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout)             = 0;
};

} // namespace comm
} // namespace jaf