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
#include "log_format.h"
#include "Interface/log/event_level_str.h"
#include <chrono>
#include <format>

namespace jaf
{
namespace log
{

LogFormat::LogFormat()
{
}

std::string LogFormat::EventToString(const IEvent& log_event)
{
    std::string str_log = std::format("[{:%Y-%m-%d %H:%M:%S}][{}][{:6X}]:{}", std::chrono::system_clock::time_point{std::chrono::milliseconds{log_event.Time()}}, jaf::log::LogLevelManage::Get(log_event.Level()), log_event.ThreadId(), log_event.Info());

    return str_log;
}

} // namespace log
} // namespace jaf