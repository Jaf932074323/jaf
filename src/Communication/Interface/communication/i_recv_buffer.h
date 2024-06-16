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
#include "comm_struct.h"
#include "i_channel.h"
#include "i_pack.h"

namespace jaf
{
namespace comm
{

// �������ݻ���
class IRecvBuffer
{
public:
    IRecvBuffer() {}
    virtual ~IRecvBuffer(){};

public:
    // ���ö�ȡ��������
    // buff_len �����С
    // receive_min_buff_len һ�ζ�ȡ��Ҫ����С�����С
    virtual void SetRecvConfig(size_t buff_len, size_t receive_min_buff_len) = 0;

    virtual void Init() = 0;

    // ��ȡ�������õĻ��棬���ڶ�ȡ����
    virtual SData GetRecvBuff() = 0;
    // �Ѿ�������len���ȵ������ڻ�����
    virtual void RecvData(size_t len) = 0;

    // ��ȡ�Ѿ����յ�������
    virtual SConstData GetRecvData() = 0;

    // ��ǰ���Ƴ�һ������
    virtual void RemoveFront(size_t len) = 0;
    // ��start_indexλ�õ�len�������ݷ�װ�ɵ��������ݶ�ȡ��
    virtual std::shared_ptr<IPack> GetPack(size_t start_index, size_t len) = 0;

    // ��ȡ��Ӧ��ͨ��
    virtual std::shared_ptr<IChannel> GetChannel() = 0;
};

} // namespace comm
} // namespace jaf