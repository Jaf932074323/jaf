#include "unpack.h"
#include "log_head.h"

jaf::Coroutine<void> Unpack::Run(std::shared_ptr<jaf::comm::IChannel> channel, std::shared_ptr<jaf::comm::IDealPack> deal_pack)
{
    jaf::comm::RecvBuffer recv_buffer(channel); // �������ݻ���
    recv_buffer.Init();

    while (true)
    {
        auto [buff, len] = recv_buffer.GetRecvBuff();
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
