#pragma once
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