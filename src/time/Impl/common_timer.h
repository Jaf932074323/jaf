#pragma once
#include "interface/i_timer.h"

namespace jaf
{
namespace time
{

class CommonTimer
{
public:
    CommonTimer() {}
    virtual ~CommonTimer() {}

public:
    static CommonTimer& Instance()
    {
        static CommonTimer common_timer;
        return common_timer;
    }

    static std::shared_ptr<ITimer> Timer()
    {
        return Instance().timer_;
    }
    static void SetTimer(std::shared_ptr<ITimer> timer)
    {
        Instance().timer_ = timer;
    }

private:
    std::shared_ptr<ITimer> timer_ = nullptr;
};

} // namespace time
} // namespace jaf
