#pragma once
#include "Interface/communication/i_deal_pack.h"

// ����ͨ��ͨ��ͨ��ͨ�� �����ͨ����д����
class DealPack: public jaf::comm::IDealPack
{
public:
	DealPack(){}
	virtual ~DealPack() {};

public:
	// �����
	virtual void Deal(std::shared_ptr<jaf::comm::IPack> pack) override;
};
