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
#include "Interface/log/i_event.h"
#include <string>

namespace jaf
{
namespace log
{

// 日志事件
class Event : public IEvent
{
public:
    Event(
        uint32_t level, std::string file_name, uint32_t line, std::string fun_name, uint64_t time, uint64_t thread_id, uint32_t group_number);
    ~Event();

    virtual uint32_t Level() const override;
    virtual uint32_t GroupNumber() const override;
    virtual uint64_t ThreadId() const override;
    virtual uint64_t Time() const override;
    virtual const char* FileName() const override;
    virtual uint32_t Line() const override;
    virtual const char* Info() const override;

public:
    Event& operator<<(const std::string_view& arg)
    {
        info_ += arg;
        return *this;
    }

    Event& operator<<(const char* arg)
    {
        info_ += std::string_view(arg);
        return *this;
    }

private:
    uint32_t level_        = LOG_LEVEL_INFO;
    uint32_t group_number_ = 0;
    uint64_t thread_id_    = 0;
    uint64_t time_         = 0;
    std::string file_name_;
    uint32_t line_ = 0;
    std::string fun_name_;
    std::string info_;
};

} // namespace log
} // namespace jaf