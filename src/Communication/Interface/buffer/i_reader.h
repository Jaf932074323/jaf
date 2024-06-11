#pragma once
#include <memory>

namespace jaf
{
namespace buffer
{

// ���ݶ�ȡ��
class IReader
{
public:
    IReader() {}
    virtual ~IReader(){};

public:
    // ��ȡ����
    virtual bool Read(const unsigned char* dst, size_t len) = 0;

    // ��ȡδ��ȡ���ݵĵ�ַ
    virtual const unsigned char* GetNotReadData() = 0;
    // ��ȡδ��ȡ���ݵĳ���
    virtual size_t GetNotReadLen() = 0;

    // ��ȡ�ܵ����ݵ�ַ,�����Ѷ�ȡ����
    virtual const unsigned char* GetTotalData() = 0;
    // ��ȡ�ܵ����ݳ���,�����Ѷ�ȡ����
    virtual size_t GetTotalLen() = 0;
};

} // namespace buffer
} // namespace jaf