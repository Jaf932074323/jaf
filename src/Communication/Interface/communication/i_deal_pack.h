#pragma once
#include <memory>
#include "i_pack.h"

namespace jaf
{
namespace comm
{

// ����ͨ��ͨ��ͨ��ͨ�� �����ͨ����д����
class IDealPack
{
public:
	IDealPack(){}
	virtual ~IDealPack() {};

public:
	// �����
	virtual void Deal(std::shared_ptr<IPack> pack) = 0;
};

}
}