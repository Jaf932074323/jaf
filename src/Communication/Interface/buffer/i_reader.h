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
#include <memory>

namespace jaf
{
namespace buffer
{

// 数据读取器
class IReader
{
public:
    IReader() {}
    virtual ~IReader(){};

public:
    // 读取数据
    virtual bool Read(const unsigned char* dst, size_t len) = 0;

    // 获取未读取数据的地址
    virtual const unsigned char* GetNotReadData() = 0;
    // 获取未读取数据的长度
    virtual size_t GetNotReadLen() = 0;

    // 获取总的数据地址,包括已读取部分
    virtual const unsigned char* GetTotalData() = 0;
    // 获取总的数据长度,包括已读取部分
    virtual size_t GetTotalLen() = 0;
};

} // namespace buffer
} // namespace jaf