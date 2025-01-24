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
#include "Interface/communication/i_pack.h"

namespace jaf
{
namespace comm
{

// 处理通信通道通信通道 负责从通道读写数据
class Pack : public IPack
{
public:
    Pack(std::shared_ptr<IChannel> channel, std::shared_ptr<unsigned char[]> buff, SConstData data);
    virtual ~Pack();
    ;

public:
    // 获取对应的通道
    virtual std::shared_ptr<IChannel> GetChannel() override;
    virtual SConstData GetData() override;

private:
    std::shared_ptr<IChannel> channel_;
    std::shared_ptr<unsigned char[]> buff_;
    SConstData data_;
};

} // namespace comm
} // namespace jaf