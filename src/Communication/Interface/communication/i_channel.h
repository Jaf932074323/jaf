#pragma once
#include "util/co_coroutine.h"
#include <string>

namespace jaf
{
namespace comm
{

// ͨ��ͨ��ִ�н��
struct SChannelResult
{
    enum class EState
    {
        CRS_SUCCESS    = 0, // �ɹ�
        CRS_CHANNEL_END          = 1, // ͨ����������
        CRS_FAIL = 2,         // ʧ��,�������ԭ����code_��ʾ
        CRS_CHANNEL_DISCONNECTED    = 3, // ͨ���Ͽ�����
        CRS_TIMEOUT  = 4, // ��ʱ
        CRS_UNKNOWN, // δ֪����
    };
    
    EState state = EState::CRS_UNKNOWN;
    size_t len   = 0;     // ������
    int code_ = 0; // ������� stateΪCRS_FAILʱ��Ч
    std::string error;    // ��ʧ����δ��ʱ��ʧ��ԭ��
};

// ͨ��ͨ��
class IChannel
{
public:
    IChannel() {}
    virtual ~IChannel(){};

public:
    virtual Coroutine<bool> Start()                                                                        = 0;
    virtual void Stop()                                                                                    = 0;
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout)        = 0;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) = 0;
};

} // namespace comm
} // namespace jaf