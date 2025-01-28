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

#include "get_time_high_precision.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <Windows.h>
#elif defined(__linux__)
#include <time.h>
#endif

namespace jaf
{
namespace time
{

GetTimeHighPrecision::GetTimeHighPrecision()
{
#ifdef _WIN32
    QueryPerformanceFrequency(&m_cpuFreq);
#elif defined(__linux__)
#endif
}

GetTimeHighPrecision::~GetTimeHighPrecision() {}

#ifdef _WIN32

uint64_t GetTimeHighPrecision::GetNowTime()
{
    LARGE_INTEGER time;
    double rumTime = 0.0;
    QueryPerformanceCounter(&time);
    rumTime = (time.QuadPart * 1000.0f) / m_cpuFreq.QuadPart;

    return (unsigned __int64) rumTime;
}

#elif defined(__linux__)

uint64_t GetTimeHighPrecision::GetNowTime()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ((unsigned long long)ts.tv_nsec / 1000000) + ((unsigned long long)ts.tv_sec * 1000);
}

#endif

} // namespace time
} // namespace jaf
