#pragma once
#include "Interface/communication/i_pack.h"

namespace jaf
{
namespace comm
{

// ����ͨ��ͨ��ͨ��ͨ�� �����ͨ����д����
class Pack: public IPack
{
public:
	Pack(std::shared_ptr<IChannel> channel, std::shared_ptr<unsigned char[]> buff, SConstData data);
	virtual ~Pack();;

public:
	// ��ȡ��Ӧ��ͨ��
	virtual std::shared_ptr<IChannel> GetChannel() override;
	virtual SConstData GetData() override;

private:
	std::shared_ptr<IChannel> channel_;
	std::shared_ptr<unsigned char[]> buff_;
	SConstData data_;
};

}
}