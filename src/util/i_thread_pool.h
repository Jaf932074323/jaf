#pragma once
#include <functional>

namespace jaf
{

class IThreadPool
{
public:
	virtual ~IThreadPool() {}

	virtual void Post(std::function<void(void)> fun) = 0;
};

}
