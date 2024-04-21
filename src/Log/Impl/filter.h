#pragma once
#include <memory>
#include "Interface/log/i_filter.h"

namespace jaf
{
namespace log
{

// ����̨��־�����
class Filter:public IFilter
{
public:
	// min_levelͨ���������־�ȼ�
	Filter(uint32_t min_level);
	virtual ~Filter(){}

public:
	// ɸѡ��־ trueͨ��ɸѡ��false��ͨ��ɸѡ
	virtual bool Filtration(const Event& log_event) override;

protected:
	uint32_t min_level_; // ͨ���������־�ȼ�
};

}
}