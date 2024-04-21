#pragma once
#include <memory>
#include "i_appender.h"
#include "event.h"

namespace jaf
{
namespace log
{

// ������
class IFilter
{
public:
	virtual ~IFilter() {};

public:
	// ɸѡ��־ trueͨ��ɸѡ��false��ͨ��ɸѡ
	virtual bool Filtration(const Event& log_event) = 0;
};

}
}