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
// 2024-6-23 ½ª°²¸»
#include <assert.h>
#include <atomic>
#include <chrono>

namespace jaf
{

class Stopwatch
{
public:
    Stopwatch()
        : begin_time_(std::chrono::high_resolution_clock::now())
    {
    }

    ~Stopwatch()
    {
    }

    void Reset()
    {
        begin_time_ = std::chrono::high_resolution_clock::now();
    }
    std::chrono::nanoseconds Time()
    {
        std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now() - begin_time_;
        return elapsed;
        //std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time_).count();
    }


private:
    std::chrono::time_point<std::chrono::high_resolution_clock> begin_time_;
};

} // namespace jaf
