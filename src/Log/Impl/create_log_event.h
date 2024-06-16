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
#include "interface/log/i_logger.h"
#include <memory>

namespace jaf
{
namespace log
{

// 控制台日志输出器
class API_LOG_EXPORT CreateLogEvent
{
public:
    CreateLogEvent(
        uint32_t level, std::string file_name, uint32_t line, std::string fun_name, std::shared_ptr<jaf::log::ILogger> logger, uint64_t time = GetCurTime(), uint64_t thread_id = GetCurThreadId(), uint32_t group_number = 0);

    CreateLogEvent(
        uint32_t level, std::string file_name, uint32_t line, std::string fun_name, uint64_t time = GetCurTime(), uint64_t thread_id = GetCurThreadId(), uint32_t group_number = 0);

    ~CreateLogEvent();

    CreateLogEvent(const CreateLogEvent&)            = delete;
    CreateLogEvent& operator=(const CreateLogEvent&) = delete;

public:
    CreateLogEvent& operator<<(const std::string_view& arg);
    CreateLogEvent& operator<<(const char* arg);

    static uint64_t GetCurThreadId();
    static uint64_t GetCurTime();

protected:
    struct Impl;
    Impl* m_impl;
};

} // namespace log
} // namespace jaf