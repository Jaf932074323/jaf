#pragma once
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