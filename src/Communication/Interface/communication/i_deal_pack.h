#pragma once
#include "i_pack.h"
#include <memory>

namespace jaf
{
namespace comm
{

// ����ͨ��ͨ��ͨ��ͨ�� �����ͨ����д����
class IDealPack
{
public:
    IDealPack() {}
    virtual ~IDealPack(){};

public:
    // �����
    virtual void Deal(std::shared_ptr<IPack> pack) = 0;
};

} // namespace comm
} // namespace jaf