#pragma once
// MIT License
//
// Copyright(c) 2021 Jaf932074323
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 2024-12-28 姜安富

#ifdef _WIN32
#elif defined(__linux__)

#include "Interface/communication/comm_struct.h"
#include "Interface/communication/i_communication.h"
#include "i_get_completion_port.h"
#include "time_head.h"
#include "util/co_wait_notices.h"
#include "util/co_wait_util_stop.h"
#include "util/i_thread_pool.h"

namespace jaf
{
namespace comm
{

// linux平台下的通讯类
class Communication : public ICommunication
{
public:
    Communication(std::shared_ptr<IThreadPool> thread_pool = nullptr, std::shared_ptr<jaf::time::ITimer> timer = nullptr);
    virtual ~Communication();

public:
    virtual jaf::Coroutine<void> Init() override;
    virtual jaf::Coroutine<void> Run() override;
    virtual void Stop() override;

public:
    virtual std::shared_ptr<ITcpServer> CreateTcpServer() override;
    virtual std::shared_ptr<ITcpClient> CreateTcpClient() override;
    virtual std::shared_ptr<IUdp> CreateUdp() override;
    virtual std::shared_ptr<ISerialPort> CreateSerialPort() override;

private:
    // 创建工作线程
    void CreateWorkThread();
    // 工作线程执行
    void WorkThreadRun();

private:
    HANDLE GetCompletionPort()
    {
        return m_completionPort;
    }

    class CompletionPort : public IGetCompletionPort
    {
    public:
        CompletionPort(Communication* communication)
            : communication_(communication) {}
        ~CompletionPort() {}

    public:
        HANDLE Get()
        {
            return communication_->GetCompletionPort();
        }

    private:
        Communication* communication_;
    };

private:
    HANDLE m_completionPort = 0;
    CompletionPort get_completion_port_; // 获取完成端口对象

    bool run_flag_ = false; // 运行标志

    size_t work_thread_count_ = 1;
    std::shared_ptr<IThreadPool> thread_pool_;

    std::shared_ptr<jaf::time::ITimer> timer_;

    CoWaitNotices wait_work_thread_finish_; // 等待工作线程结束

    CoWaitUtilStop wait_stop_;
};

} // namespace comm
} // namespace jaf

#endif