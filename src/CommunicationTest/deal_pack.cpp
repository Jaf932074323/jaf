#include "deal_pack.h"
#include <iostream>
#include <string>
#include "log_head.h"

void DealPack::Deal(std::shared_ptr<jaf::comm::IPack> pack)
{
	auto [buff, len] = pack->GetData();
	std::string str((const char*)buff, len);
	LOG_INFO() << "½ÓÊÕ:" << str;
	pack->GetChannel()->Write(buff, len);
}