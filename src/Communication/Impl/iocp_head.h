#pragma once
#include "Interface/communication/i_channel_user.h"
#include "util/co_coroutine.h"

typedef void* HANDLE;

namespace jaf
{
namespace comm
{

class IGetCompletionPort
{
public:
    virtual ~IGetCompletionPort() {}

public:
    virtual HANDLE Get() = 0;
};

} // namespace comm
} // namespace jaf