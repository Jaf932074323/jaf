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
// 2024-6-16 ������

namespace jaf
{
namespace buffer
{

// ����д����
class IWriter
{
public:
    IWriter() {}
    virtual ~IWriter(){};

public:
    // ��������������Ϊsize
    virtual void Reserve(size_t size) = 0;
    // ��չδʹ������������size
    virtual void Extend(size_t size) = 0;
    // ��ȡ������
    virtual size_t Capacity() = 0;
    // ��ȡδʹ������
    virtual size_t UnusedCapacity() = 0;

    virtual const unsigned char* GetBuffer() const = 0;
    virtual size_t GetBufferLen() const            = 0;

public:
    // д������
    virtual bool Write(unsigned char* src, size_t len) = 0;
};

} // namespace buffer
} // namespace jaf