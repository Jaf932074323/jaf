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
#include "interface/i_logger.h"
#include <memory>
#include <string>

namespace jaf
{
namespace log
{

// 公用日志
class API_LOG_EXPORT CommonLogger
{
    CommonLogger();
    CommonLogger(const CommonLogger&)            = delete;
    CommonLogger& operator=(const CommonLogger&) = delete;

public:
    ~CommonLogger();

public:
    static void SetDefaultLogger(std::shared_ptr<ILogger> logger);
    static std::shared_ptr<ILogger> DefaultLogger();

    static void SetLogger(const std::string& log_name, std::shared_ptr<ILogger> logger);
    static std::shared_ptr<ILogger> Logger(const std::string& log_name);

private:
    static CommonLogger& Instance();

protected:
    struct Impl;
    Impl* m_impl;
};


} // namespace log
} // namespace jaf