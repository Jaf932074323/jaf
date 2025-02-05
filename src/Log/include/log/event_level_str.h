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
#include "log/interface/i_event.h"
#include <map>

namespace jaf
{
namespace log
{

class LogLevelManage
{
    // 单例模式
    LogLevelManage()
        : event_levels_{
              {LOG_LEVEL_FATAL, "fatal"},
              {LOG_LEVEL_ERROR, "error"},
              {LOG_LEVEL_WARNING, "warning"},
              {LOG_LEVEL_INFO, "info"},
              {LOG_LEVEL_DEBUG, "debug"},
              {LOG_LEVEL_TRANCE, "trance"}}
    {
    }
    LogLevelManage(const LogLevelManage&)            = delete;
    LogLevelManage& operator=(const LogLevelManage&) = delete;

public:
    ~LogLevelManage() {}
    static LogLevelManage& Instance()
    {
        static LogLevelManage instance; // 单例实例
        return instance;
    }

    static void Set(uint32_t level, const char* str)
    {
        Instance().event_levels_.insert(std::make_pair(level, str));
    }

    static const char* Get(uint32_t level)
    {
        auto it = Instance().event_levels_.find(level);
        if (it == Instance().event_levels_.end())
        {
            return "";
        }
        return it->second;
        ;
    }

private:
    std::map<uint32_t, const char*> event_levels_; // key为日志等级，value为对应的字符串
};


} // namespace log
} // namespace jaf