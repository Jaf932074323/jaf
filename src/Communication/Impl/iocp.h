#pragma once
#include "i_iocp_user.h"
#include "interface/communication/comm_struct.h"
#include "util/co_await.h"
#include "util/co_await_times.h"
#include "util/co_coroutine.h"
#include "util/i_thread_pool.h"
#include <list>
#include <map>
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

// windows平台下的完成端口
class Iocp : public std::enable_shared_from_this<Iocp>
{
public:
    Iocp(std::shared_ptr<IThreadPool> thread_pool = nullptr);
    virtual ~Iocp();

public:
    virtual jaf::Coroutine<void> Init();
    virtual jaf::Coroutine<void> Run();
    virtual void Stop();

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

    size_t work_thread_count_ = 1;
    std::shared_ptr<IThreadPool> thread_pool_;

    CoAwaitTimes await_work_thread_finish_; // 等待工作线程结束

    CoAwait await_stop_;
};

} // namespace comm
} // namespace jaf