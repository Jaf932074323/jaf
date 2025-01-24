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
#include "comm_struct.h"
#include "i_channel.h"
#include "i_pack.h"

namespace jaf
{
namespace comm
{

// 接收数据缓存
class IRecvBuffer
{
public:
    IRecvBuffer() {}
    virtual ~IRecvBuffer(){};

public:
    // 设置读取缓存配置
    // buff_len 缓存大小
    // receive_min_buff_len 一次读取需要的最小缓存大小
    virtual void SetRecvConfig(size_t buff_len, size_t receive_min_buff_len) = 0;

    virtual void Init() = 0;

    // 获取读数据用的缓存，用于读取数据
    virtual SData GetRecvBuff() = 0;
    // 已经接收了len长度的数据在缓存中
    virtual void RecvData(size_t len) = 0;

    // 获取已经接收到的数据
    virtual SConstData GetRecvData() = 0;

    // 从前面移除一段数据
    virtual void RemoveFront(size_t len) = 0;
    // 将start_index位置的len长度数据封装成单独的数据读取器
    virtual std::shared_ptr<IPack> GetPack(size_t start_index, size_t len) = 0;

    // 获取对应的通道
    virtual std::shared_ptr<IChannel> GetChannel() = 0;
};

} // namespace comm
} // namespace jaf