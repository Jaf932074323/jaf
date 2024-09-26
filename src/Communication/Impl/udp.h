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
#include "Interface/communication/i_channel_user.h"
#include "Interface/communication/i_udp.h"
#include "empty_channel.h"
#include "interface/communication/i_unpack.h"
#include "iocp_head.h"
#include "time_head.h"
#include "util/co_wait_notice.h"
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace jaf
{
namespace comm
{

class Udp : public IUdp
{
public:
    Udp(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer);
    virtual ~Udp();

public:
    virtual void SetAddr(const std::string& local_ip, uint16_t local_port, const std::string& remote_ip, uint16_t remote_port);
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user);
    virtual Coroutine<void> Run();
    virtual void Stop();
    virtual std::shared_ptr<IChannel> GetChannel();
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout);

private:
    void Init(void);


private:
    IGetCompletionPort* get_completion_port_ = nullptr;
    HANDLE completion_handle_                = nullptr;
    SOCKET socket_                           = 0; // 侦听套接字

    std::shared_ptr<jaf::time::ITimer> timer_;

    std::string local_ip_  = "0.0.0.0";
    uint16_t local_port_   = 0;
    std::string remote_ip_ = "0.0.0.0";
    uint16_t remote_port_  = 0;

    std::shared_ptr<IChannelUser> user_ = nullptr; // 通道使用者
    std::mutex channel_mutex_;
    bool run_flag_                     = false;
    std::shared_ptr<IChannel> channel_ = std::make_shared<EmptyChannel>();
};

} // namespace comm
} // namespace jaf