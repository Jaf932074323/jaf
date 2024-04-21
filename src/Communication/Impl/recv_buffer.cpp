#pragma once
#include "recv_buffer.h"
#include <assert.h>
#include "pack.h"

namespace jaf
{
namespace comm
{

RecvBuffer::RecvBuffer(std::shared_ptr<IChannel> channel)
	:channel_(channel)
{
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::SetRecvConfig(size_t total_buff_len, size_t receive_min_buff_len)
{
	total_buff_len_ = total_buff_len;
	receive_min_buff_len_ = receive_min_buff_len;
}

void RecvBuffer::Init()
{
	SwitchNewRecvBuff();
}

SData RecvBuffer::GetRecvBuff()
{
	return SData{ .buff = recv_data_ + recv_len_, .len = wait_recv_len_ };
}

void RecvBuffer::RecvData(size_t len)
{
	assert(wait_recv_len_ >= len);
	recv_len_ += len;
	wait_recv_len_ -= len;

	// 后续空间不够再接收一次数据，则申请新的缓存
	if (wait_recv_len_ < receive_min_buff_len_)
	{
		SwitchNewRecvBuff();
		assert(wait_recv_len_ >= receive_min_buff_len_); // 已接收数据太多没有处理会出现问题
	}
}

SConstData RecvBuffer::GetRecvData()
{
	return SConstData{ .buff = recv_data_, .len = recv_len_ };
}

void RecvBuffer::RemoveFront(size_t len)
{
	assert(recv_len_ >= len);
	recv_data_ += len;
	recv_len_ -= len;
}

std::shared_ptr<IPack> RecvBuffer::GetPack(size_t start_index, size_t len)
{
	assert(start_index + len <= recv_len_);
	return std::make_shared<Pack>(channel_, buff_, SConstData{ recv_data_ + start_index, len });
}

std::shared_ptr<IChannel> RecvBuffer::GetChannel()
{
	return channel_;
}

void RecvBuffer::SwitchNewRecvBuff()
{
	// TODO: 当剩余数据太多时，应当如何处理？
	std::shared_ptr<unsigned char[]> new_recv_buff = std::make_shared<unsigned char[]>(total_buff_len_);

	assert(total_buff_len_ >= recv_len_);
	if (recv_data_ != nullptr)
	{
		memcpy(new_recv_buff.get(), recv_data_, recv_len_);
	}
	memset(new_recv_buff.get() + recv_len_, 0, total_buff_len_ - recv_len_);

	buff_ = new_recv_buff;
	curr_buff_len_ = total_buff_len_;

	recv_data_ = buff_.get();
	wait_recv_len_ = curr_buff_len_ - recv_len_;
}

}
}