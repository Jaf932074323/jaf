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
#include "interface/i_timer.h"
#include <memory>

namespace jaf
{
namespace time
{

class CommonTimer
{
public:
    CommonTimer() {}
    virtual ~CommonTimer() {}

public:
    static CommonTimer& Instance()
    {
        static CommonTimer common_timer;
        return common_timer;
    }

    static std::shared_ptr<ITimer> Timer()
    {
        return Instance().timer_;
    }
    static void SetTimer(std::shared_ptr<ITimer> timer)
    {
        Instance().timer_ = timer;
    }

private:
    std::shared_ptr<ITimer> timer_ = nullptr;
};

} // namespace time
} // namespace jaf
