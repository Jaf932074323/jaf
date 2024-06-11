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

// 2020-12-3 姜安富
// Tick定时器
// 使用GetTickCount64()的方式去获取系统时间，
#include "interface/i_get_time.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace jaf
{
namespace time
{

class CHighPrecisionTimer : public IGetTime
{
public:
    CHighPrecisionTimer();
    virtual ~CHighPrecisionTimer();

protected:
    // 获取当前时间，不同的方式获取的时间，得到的定时精度不相同
    virtual uint64_t GetNowTime() override;

private:
    LARGE_INTEGER m_cpuFreq;
};

} // namespace time
} // namespace jaf
