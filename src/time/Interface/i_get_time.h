#pragma once
#include <stdint.h>

namespace jaf
{
namespace time
{

// ������
class IGetTime
{
public:
	virtual ~IGetTime(){};

public:
	// ��ȡ��ǰʱ�䣬��ͬ�ķ�ʽ��ȡ��ʱ�䣬�õ��Ķ�ʱ���Ȳ���ͬ
	virtual uint64_t GetNowTime() = 0;
};

}
}