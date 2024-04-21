#pragma once
#include <memory>
#include "event.h"

namespace jaf
{
namespace log
{

// ��־�����
class IAppender
{
public:
	virtual ~IAppender() {};

public:
	// ������־�¼�
	virtual void OnLogEvent(const Event& log_event) = 0;
};

}
}