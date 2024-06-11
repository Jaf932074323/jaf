#pragma once
#include "Interface/log/i_filter.h"
#include "define_log_export.h"
#include <memory>

namespace jaf
{
namespace log
{

// ����̨��־�����
class API_LOG_EXPORT Filter : public IFilter
{
public:
    // min_levelͨ���������־�ȼ�
    Filter(uint32_t min_level);
    virtual ~Filter() {}

public:
    // ɸѡ��־ trueͨ��ɸѡ��false��ͨ��ɸѡ
    virtual bool Filtration(const IEvent& log_event) override;

protected:
    uint32_t min_level_; // ͨ���������־�ȼ�
};

} // namespace log
} // namespace jaf