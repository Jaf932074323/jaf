#include "pack.h"

namespace jaf
{
namespace comm
{

Pack::Pack(std::shared_ptr<IChannel> channel,
    std::shared_ptr<unsigned char[]> buff, SConstData data)
    : channel_(channel), buff_(buff), data_(data) {}

Pack::~Pack() {}

std::shared_ptr<IChannel> Pack::GetChannel() { return channel_; }

SConstData Pack::GetData() { return data_; }
} // namespace comm
} // namespace jaf