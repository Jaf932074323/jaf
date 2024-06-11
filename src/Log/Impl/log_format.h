#pragma once
#include "define_log_export.h"
#include "interface/log/i_format.h"
#include <memory>

namespace jaf
{
namespace log
{

// ����̨��־�����
class API_LOG_EXPORT LogFormat : public IFormat
{
public:
    LogFormat();
    virtual ~LogFormat() {}

public:
    // ��־�¼�ת�ַ���
    virtual std::string EventToString(const IEvent& log_event) override;

protected:
};

} // namespace log
} // namespace jaf