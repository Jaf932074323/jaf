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
#include "define_log_export.h"
#include <memory>
#include <thread>

namespace jaf
{
namespace log
{

constexpr uint32_t LOG_LEVEL_FATAL   = 0; // 致命错误
constexpr uint32_t LOG_LEVEL_ERROR   = 1; // 错误
constexpr uint32_t LOG_LEVEL_WARNING = 2; // 警告
constexpr uint32_t LOG_LEVEL_INFO    = 3; // 信息
constexpr uint32_t LOG_LEVEL_DEBUG   = 4; // 调试信息
constexpr uint32_t LOG_LEVEL_TRANCE  = 5; // 追踪信息

// 日志事件
class API_LOG_EXPORT IEvent
{
public:
    IEvent(){};
    virtual ~IEvent() {}

public:
    virtual uint32_t Level() const       = 0;
    virtual uint32_t GroupNumber() const = 0;
    virtual uint64_t ThreadId() const    = 0;
    virtual uint64_t Time() const        = 0;
    virtual const char* FileName() const = 0;
    virtual uint32_t Line() const        = 0;
    virtual const char* Info() const     = 0;
};

} // namespace log
} // namespace jaf