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
#include "log/console_appender.h"
#include "log/filter.h"
#include "log/log_format.h"
#include <iostream>

namespace jaf
{
namespace log
{

struct ConsoleAppender::Impl
{
    std::shared_ptr<IFilter> filter_; // 过滤器
    std::shared_ptr<IFormat> format_; // 日志格式化
};

ConsoleAppender::ConsoleAppender(std::shared_ptr<IFormat> format, std::shared_ptr<IFilter> filter)
{
    m_impl = new Impl{
        .filter_ = filter == nullptr ? std::make_shared<Filter>(LOG_LEVEL_INFO) : filter,
        .format_ = format == nullptr ? std::make_shared<LogFormat>() : format};
}

ConsoleAppender::~ConsoleAppender()
{
    delete m_impl;
}

void ConsoleAppender::OnLogEvent(const IEvent& log_event)
{
    if (!m_impl->filter_->Filtration(log_event))
    {
        return;
    }

    std::cout << m_impl->format_->EventToString(log_event) << std::endl;
}

} // namespace log
} // namespace jaf