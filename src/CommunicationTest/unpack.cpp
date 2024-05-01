#include "unpack.h"

jaf::Coroutine<void> Unpack::Run(std::shared_ptr<jaf::comm::IChannel> channel, std::shared_ptr<jaf::comm::IDealPack> deal_pack)
{
    jaf::comm::RecvBuffer recv_buffer(channel); // 接收数据缓存
    recv_buffer.Init();

    while (true)
    {
        auto [buff, len] = recv_buffer.GetRecvBuff();
        auto result      = co_await channel->Read(buff, len, 5000);
        if (!result.success)
        {
            if (result.timeout)
            {
                continue;
            }
            break;
        }

        recv_buffer.RecvData(result.len);
        std::shared_ptr<jaf::comm::IPack> pack = recv_buffer.GetPack(0, result.len);

        recv_buffer.RemoveFront(result.len);

        deal_pack->Deal(pack);
    }

    co_return;
}
