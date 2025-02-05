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
#include "interface/buffer/i_recv_buffer.h"
#include "pack.h"
#include <assert.h>
#include <string.h>

namespace jaf
{
namespace comm
{

// 接收数据缓存
class RecvBuffer : public IRecvBuffer
{
public:
    RecvBuffer(std::shared_ptr<IChannel> channel)
        : channel_(channel)
    {
    }
    virtual ~RecvBuffer() {}

public:
    // 设置读取缓存配置
    // buff_len 缓存大小
    // receive_min_buff_len 一次读取需要的最小缓存大小
    virtual void SetRecvConfig(size_t buff_len, size_t receive_min_buff_len) override
    {
        total_buff_len_       = buff_len;
        receive_min_buff_len_ = receive_min_buff_len;
    }

    virtual void Init() override { SwitchNewRecvBuff(); }

    // 获取读数据用的缓存，用于读取数据
    virtual SData GetRecvBuff() override
    {
        return SData{.buff = recv_data_ + recv_len_, .len = wait_recv_len_};
    }
    // 已经接收了len长度的数据在缓存中
    virtual void RecvData(size_t len) override
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

    // 获取已经接收到的数据
    virtual SConstData GetRecvData() override
    {
        return SConstData{.buff = recv_data_, .len = recv_len_};
    }

    // 从前面移除一段数据
    virtual void RemoveFront(size_t len) override
    {
        assert(recv_len_ >= len);
        recv_data_ += len;
        recv_len_ -= len;
    }
    // 将start_index位置的len长度数据封装成单独的数据读取器
    virtual std::shared_ptr<IPack> GetPack(size_t start_index, size_t len) override
    {
        assert(start_index + len <= recv_len_);
        return std::make_shared<Pack>(channel_, buff_,
            SConstData{recv_data_ + start_index, len});
    }

    // 获取对应的通道
    virtual std::shared_ptr<IChannel> GetChannel() override
    {
        return channel_;
    }

private:
    // 切换为新的接收数据缓存
    void SwitchNewRecvBuff()
    {
        // TODO: 当剩余数据太多时，应当如何处理？
        std::shared_ptr<unsigned char[]> new_recv_buff = std::make_shared<unsigned char[]>(total_buff_len_);

        assert(total_buff_len_ >= recv_len_);
        if (recv_data_ != nullptr)
        {
            memcpy(new_recv_buff.get(), recv_data_, recv_len_);
        }
        memset(new_recv_buff.get() + recv_len_, 0, total_buff_len_ - recv_len_);

        buff_          = new_recv_buff;
        curr_buff_len_ = total_buff_len_;

        recv_data_     = buff_.get();
        wait_recv_len_ = curr_buff_len_ - recv_len_;
    }

private:
    std::shared_ptr<IChannel> channel_;

    size_t total_buff_len_       = 10 * 1024 * 1024; // 总缓存大小
    size_t receive_min_buff_len_ = 1024 * 1024;      // 一次读取需要的最小缓存大小

    std::shared_ptr<unsigned char[]> buff_ = nullptr; // 缓存包含3个部分，1.前面已经被移除的数据，2.已经接收的数据，3.等待接收数据的部分
    size_t curr_buff_len_                  = 0;       // 当前缓存大小

    unsigned char* recv_data_ = nullptr; // 接收数据的起始地址 指向已经接收了数据的部分
    size_t recv_len_          = 0;       // 已接收数据的缓存长度
    size_t wait_recv_len_     = 0;       // 等待接收数据的缓存长度
};

} // namespace comm
} // namespace jaf