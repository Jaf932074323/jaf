#pragma once
#include <string>
#include "event.h"

namespace jaf
{
namespace log
{

// ��־�¼�
class IFormat
{
public:
	virtual ~IFormat() {};

public:
	// ��־�¼�ת�ַ���
	virtual std::string EventToString(const Event& log_event) = 0;
};

}
}