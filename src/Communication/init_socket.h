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
// 2025-1-18 Ω™∞≤∏ª
#ifdef _WIN32
#include <string>
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

class InitSocket
{
public:
    InitSocket(BYTE version_low = 2, BYTE version_high = 2)
    {
        //WinSock∞Ê±æ∫≈
       WORD version = MAKEWORD(version_low, version_high);

        // WinSockø‚≥ı ºªØ
        WSADATA wsa_data;
        int result = ::WSAStartup(version, &wsa_data);
        if (result != 0) // WinSockø‚≥ı ºªØ ß∞‹!
        {
            std::string err = "WinSockø‚≥ı ºªØ ß∞‹,¥ÌŒÛ∫≈:" + WSAGetLastError();
            throw err;
        }
    }
    ~InitSocket()
    {
        ::WSACleanup();
    }
};

#elif defined(__linux__)

class InitSocket
{
public:
    InitSocket(unsigned long version_low = 2, unsigned long version_high = 2)
    {
    }
    ~InitSocket()
    {
    }
};
#endif
