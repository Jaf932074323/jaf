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
#include "event.h"

namespace jaf
{
namespace log
{
Event::Event(
    uint32_t level, std::string file_name, uint32_t line, std::string fun_name, uint64_t time, uint64_t thread_id, uint32_t group_number)
    : level_(level)
    , group_number_(group_number)
    , thread_id_(thread_id)
    , time_(time)
    , file_name_(file_name)
    , line_(line)
    , fun_name_(fun_name)
{
}

Event::~Event() {}

uint32_t Event::Level() const
{
    return level_;
}

uint32_t Event::GroupNumber() const
{
    return group_number_;
}

uint64_t Event::ThreadId() const
{
    return thread_id_;
}

uint64_t Event::Time() const
{
    return time_;
}

const char* Event::FileName() const
{
    return file_name_.c_str();
}

uint32_t Event::Line() const
{
    return line_;
}

const char* Event::Info() const
{
    return info_.c_str();
}

} // namespace log
} // namespace jaf