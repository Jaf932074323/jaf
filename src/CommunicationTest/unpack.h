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
#include "Interface/communication/i_unpack.h"
#include "impl/recv_buffer.h"
#include "util/co_coroutine.h"
#include <functional>
#include <memory>

// 处理通信通道通信通道 负责从通道读写数据
class Unpack : public jaf::comm::IUnpack
{
public:
    Unpack(std::function<void(std::shared_ptr<jaf::comm::IPack> pack)> fun_deal_pack);
    virtual ~Unpack(){};

public:
    virtual jaf::Coroutine<void> Run(std::shared_ptr<jaf::comm::IChannel> channel) override;

private:
    std::function<void(std::shared_ptr<jaf::comm::IPack> pack)> fun_deal_pack_;
};
