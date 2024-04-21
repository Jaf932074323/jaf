#pragma once
#include "Interface/communication/i_recv_buffer.h"

namespace jaf
{
namespace comm
{

// 接收数据缓存
class RecvBuffer :public IRecvBuffer
{
public:
	RecvBuffer(std::shared_ptr<IChannel> channel);
	virtual ~RecvBuffer();

public:
	// 设置读取缓存配置
	// buff_len 缓存大小
	// receive_min_buff_len 一次读取需要的最小缓存大小
	virtual void SetRecvConfig(size_t buff_len, size_t receive_min_buff_len) override;

	virtual void Init() override;

	// 获取读数据用的缓存，用于读取数据
	virtual SData GetRecvBuff() override;
	// 已经接收了len长度的数据在缓存中
	virtual void RecvData(size_t len) override;

	// 获取已经接收到的数据
	virtual SConstData GetRecvData() override;

	// 从前面移除一段数据
	virtual void RemoveFront(size_t len) override;
	// 将start_index位置的len长度数据封装成单独的数据读取器
	virtual std::shared_ptr<IPack> GetPack(size_t start_index, size_t len) override;

	// 获取对应的通道
	virtual std::shared_ptr<IChannel> GetChannel() override;

private:
	// 切换为新的接收数据缓存
	void SwitchNewRecvBuff();

private:
	std::shared_ptr<IChannel> channel_;

	size_t total_buff_len_ = 10 * 1024 * 1024; // 总缓存大小
	size_t receive_min_buff_len_ = 1024 * 1024; // 一次读取需要的最小缓存大小

	std::shared_ptr<unsigned char[]> buff_ = nullptr; // 缓存包含3个部分，1.前面已经被移除的数据，2.已经接收的数据，3.等待接收数据的部分
	size_t curr_buff_len_ = 0; // 当前缓存大小

	unsigned char* recv_data_ = nullptr; // 接收数据的起始地址 指向已经接收了数据的部分
	size_t recv_len_ = 0; // 已接收数据的缓存长度
	size_t wait_recv_len_ = 0; // 等待接收数据的缓存长度
};

}
}