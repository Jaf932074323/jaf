#pragma once
#include "Interface/communication/i_serial_port.h"
#include "Interface/communication/i_tcp_client.h"
#include "Interface/communication/i_tcp_server.h"
#include "Interface/communication/i_udp.h"
#include "interface/communication/comm_struct.h"
#include "iocp_head.h"
#include "time_head.h"
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
class Iocp
{
public:
    Iocp(std::shared_ptr<IThreadPool> thread_pool = nullptr, std::shared_ptr<jaf::time::ITimer> timer = nullptr);
    virtual ~Iocp();

public:
    virtual jaf::Coroutine<void> Init();
    virtual jaf::Coroutine<void> Run();
    virtual void Stop();

    HANDLE GetCompletionPort()
    {
        return m_completionPort;
    }

public:
    std::shared_ptr<ITcpServer> CreateTcpServer();
    std::shared_ptr<ITcpClient> CreateTcpClient();
    std::shared_ptr<IUdp> CreateUdp();
    std::shared_ptr<ISerialPort> CreateSerialPort();

private:
    // 创建工作线程
    void CreateWorkThread();
    // 工作线程执行
    void WorkThreadRun();

private:
    class CompletionPort : public IGetCompletionPort
    {
    public:
        CompletionPort(Iocp* iocp)
            : iocp_(iocp) {}
        ~CompletionPort() {}

    public:
        HANDLE Get()
        {
            return iocp_->GetCompletionPort();
        }

    private:
        Iocp* iocp_;
    };

private:
    HANDLE m_completionPort = 0;
    CompletionPort get_completion_port_; // 获取完成端口对象

    bool run_flag_ = false; // 运行标志

    size_t work_thread_count_ = 1;
    std::shared_ptr<IThreadPool> thread_pool_;
    std::shared_ptr<jaf::time::ITimer> timer_;

    CoAwaitTimes await_work_thread_finish_; // 等待工作线程结束

    CoAwait await_stop_;
};

} // namespace comm
} // namespace jaf