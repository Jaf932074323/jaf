#pragma once
#include <stdint.h>

namespace jaf
{
namespace time
{

// 过滤器
class IGetTime
{
public:
	virtual ~IGetTime(){};

public:
	// 获取当前时间，不同的方式获取的时间，得到的定时精度不相同
	virtual uint64_t GetNowTime() = 0;
};

}
}