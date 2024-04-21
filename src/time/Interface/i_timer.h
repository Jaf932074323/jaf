
#pragma once
#include <functional>
#include <memory>

namespace jaf
{
namespace time
{

// ��ʱ�������
struct STimerTask
{
	std::function<void(void)> fun; // ��ʱִ�к���
	uint64_t interval = 1000; // ��ʱʱ���������룩
};

// ������
class ITimer
{
public:
	virtual ~ITimer()
	{};

public:
	// ��Ӷ�ʱ����
	// rTask ��ʱ������Ϣ
	// ���ض�ʱ����Id������0ʱ��ʾ��Ӷ�ʱ����ʧ��
	virtual uint64_t AddTask(const STimerTask& task) = 0;
	// ������ж�ʱ����
	virtual void Clear() = 0;
	// �Ƴ�һ����ʱ����
	// nTimeId Ҫ�Ƴ��Ķ�ʱ�����Id
	virtual void Remove(uint64_t task_id) = 0;
};

}
}