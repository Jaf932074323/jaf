#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>
#include <latch>
#include "Interface/communication/comm_struct.h"
#include "util/co_coroutine.h"
#include "i_iocp_user.h"
#include "util/i_thread_pool.h"

namespace jaf
{
namespace comm
{

// windows平台下的完成端口
class Iocp: public std::enable_shared_from_this<Iocp>
{
public:
	Iocp(std::shared_ptr<IThreadPool> thread_pool = nullptr);
	virtual ~Iocp();

public:	
	virtual void Init();
	virtual void Start();
	virtual void Stop();
	virtual void WaitEnd();

	HANDLE GetCompletionPort()
	{
		return m_completionPort;
	}

private:
	// 创建工作线程
	void CreateWorkThread();
	// 工作线程执行
	void WorkThreadRun();
		
private:
	HANDLE m_completionPort = 0;
	
	bool run_flag_ = false; // 运行标志
	
	std::shared_ptr<std::latch> work_threads_latch_ = std::make_shared<std::latch>(0);

	std::shared_ptr<IThreadPool> thread_pool_;
};

}
}