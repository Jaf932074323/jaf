#pragma once
#include <memory>
#include "Interface/log/i_format.h"

namespace jaf
{
namespace log
{

// ����̨��־�����
class LogFormat:public IFormat
{
public:
	LogFormat();
	virtual ~LogFormat(){}

public:
	// ��־�¼�ת�ַ���
	virtual std::string EventToString(const Event& log_event) override;

protected:

};

}
}