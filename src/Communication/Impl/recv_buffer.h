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
#include "Interface/communication/i_recv_buffer.h"

namespace jaf
{
namespace comm
{

// �������ݻ���
class RecvBuffer : public IRecvBuffer
{
public:
    RecvBuffer(std::shared_ptr<IChannel> channel);
    virtual ~RecvBuffer();

public:
    // ���ö�ȡ��������
    // buff_len �����С
    // receive_min_buff_len һ�ζ�ȡ��Ҫ����С�����С
    virtual void SetRecvConfig(size_t buff_len, size_t receive_min_buff_len) override;

    virtual void Init() override;

    // ��ȡ�������õĻ��棬���ڶ�ȡ����
    virtual SData GetRecvBuff() override;
    // �Ѿ�������len���ȵ������ڻ�����
    virtual void RecvData(size_t len) override;

    // ��ȡ�Ѿ����յ�������
    virtual SConstData GetRecvData() override;

    // ��ǰ���Ƴ�һ������
    virtual void RemoveFront(size_t len) override;
    // ��start_indexλ�õ�len�������ݷ�װ�ɵ��������ݶ�ȡ��
    virtual std::shared_ptr<IPack> GetPack(size_t start_index, size_t len) override;

    // ��ȡ��Ӧ��ͨ��
    virtual std::shared_ptr<IChannel> GetChannel() override;

private:
    // �л�Ϊ�µĽ������ݻ���
    void SwitchNewRecvBuff();

private:
    std::shared_ptr<IChannel> channel_;

    size_t total_buff_len_       = 10 * 1024 * 1024; // �ܻ����С
    size_t receive_min_buff_len_ = 1024 * 1024;      // һ�ζ�ȡ��Ҫ����С�����С

    std::shared_ptr<unsigned char[]> buff_ = nullptr; // �������3�����֣�1.ǰ���Ѿ����Ƴ������ݣ�2.�Ѿ����յ����ݣ�3.�ȴ��������ݵĲ���
    size_t curr_buff_len_                  = 0;       // ��ǰ�����С

    unsigned char* recv_data_ = nullptr; // �������ݵ���ʼ��ַ ָ���Ѿ����������ݵĲ���
    size_t recv_len_          = 0;       // �ѽ������ݵĻ��泤��
    size_t wait_recv_len_     = 0;       // �ȴ��������ݵĻ��泤��
};

} // namespace comm
} // namespace jaf