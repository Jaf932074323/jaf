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
// 2025-2-3 姜安富
#ifdef _WIN32
#include <WS2tcpip.h>
#include <winsock2.h>
#include <ws2def.h>
#elif defined(__linux__)
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#include <string>

namespace jaf
{
namespace comm
{

class RunResult
{
public:
    RunResult()
        : succcess_(false)
        , error_info_("empty")
    {
    }

    RunResult(bool succcess)
        : succcess_(succcess)
    {
    }

    RunResult(const std::string& error_info)
        : succcess_(false)
        , error_info_(error_info)
    {
    }

    RunResult(bool succcess, const std::string& error_info)
        : succcess_(succcess)
        , error_info_(error_info)
    {
    }

    inline operator bool() const
    {
        return succcess_;
    }

    inline const std::string& ErrorInfo() const
    {
        return error_info_;
    }

private:
    bool succcess_;
    std::string error_info_;
};

} // namespace comm
} // namespace jaf