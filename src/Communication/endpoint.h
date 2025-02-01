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
// 2024-12-28 姜安富
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

using SockAddr = sockaddr_in;

struct Endpoint
{
    Endpoint()
        : Endpoint("0.0.0.0", 0)
    {
    }

    Endpoint(const std::string& ip, uint16_t port)
        : addr_{}
    {
        addr_.sin_family = AF_INET;
        addr_.sin_port   = ::htons(port);
        ::inet_pton(AF_INET, ip.c_str(), (void*) &addr_.sin_addr);
    }

    Endpoint(const SockAddr& addr)
        : addr_(addr)
    {
    }

    void Set(const std::string& ip, uint16_t port)
    {
        addr_.sin_family = AF_INET;
        addr_.sin_port   = ::htons(port);
        ::inet_pton(AF_INET, ip.c_str(), (void*) &addr_.sin_addr);
    }

    void Set(const SockAddr& addr)
    {
        addr_ = addr;
    }

    SockAddr& GetSockAddr()
    {
        return addr_;
    }
    const SockAddr& GetSockAddr() const
    {
        return addr_;
    }

    std::string Ip() const
    {
        char buff[20] = {0};
        ::inet_ntop(AF_INET, &addr_.sin_addr, buff, sizeof(buff)); //其中recvAddr为SOCKADDR_IN类型
        return std::string(buff);
    }
    uint16_t Port() const
    {
        return ::ntohs(addr_.sin_port);
    }

private:
    SockAddr addr_;
};

} // namespace comm
} // namespace jaf