#include "unpack.h"

jaf::Coroutine<void> Unpack::Run(std::shared_ptr<jaf::comm::IChannel> channel, std::shared_ptr<jaf::comm::IDealPack> deal_pack)
{
	jaf::comm::RecvBuffer recv_buffer(channel); // 接收数据缓存
	recv_buffer.Init();

	while (true)
	{
		size_t recv_len = 0;
		auto [buff, len] = recv_buffer.GetRecvBuff();
		bool read_result = co_await channel->Read(buff, len, &recv_len);
		if (!read_result)
		{
			break;
		}
		recv_buffer.RecvData(recv_len);

		std::shared_ptr<jaf::comm::IPack> pack = recv_buffer.GetPack(0, recv_len);

		recv_buffer.RemoveFront(recv_len);

		deal_pack->Deal(pack);
	}

	co_return;
}
