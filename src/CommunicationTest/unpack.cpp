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
#include "unpack.h"
#include "log_head.h"

jaf::Coroutine<void> Unpack::Run(std::shared_ptr<jaf::comm::IChannel> channel, std::shared_ptr<jaf::comm::IDealPack> deal_pack)
{
    jaf::comm::RecvBuffer recv_buffer(channel); // 接收数据缓存
    recv_buffer.Init();

    while (true)
    {
        auto [buff, len]                 = recv_buffer.GetRecvBuff();
        jaf::comm::SChannelResult result = co_await channel->Read(buff, len, 5000);
        if (result.state != jaf::comm::SChannelResult::EState::CRS_SUCCESS)
        {
            if (result.state == jaf::comm::SChannelResult::EState::CRS_TIMEOUT)
            {
                continue;
            }

            if (result.state == jaf::comm::SChannelResult::EState::CRS_CHANNEL_END)
            {
                break;
            }

            LOG_INFO() << result.error;
            break;
        }

        recv_buffer.RecvData(result.len);
        std::shared_ptr<jaf::comm::IPack> pack = recv_buffer.GetPack(0, result.len);

        recv_buffer.RemoveFront(result.len);

        deal_pack->Deal(pack);
    }

    co_return;
}
