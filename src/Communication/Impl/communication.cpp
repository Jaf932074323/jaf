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
// 2024-6-16 ½ª°²¸»
#include "communication.h"
#include "define_constant.h"
#include "log_head.h"
#include "serial_port.h"
#include "tcp_client.h"
#include "tcp_server.h"
#include "Time/Impl/timer.h"
#include "udp.h"
#include "util/finally.h"
#include "util/simple_thread_pool.h"
#include <assert.h>
#include <format>
#include <functional>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

Communication::Communication(std::shared_ptr<IThreadPool> thread_pool, std::shared_ptr<jaf::time::ITimer> timer)
    : thread_pool_(thread_pool == nullptr ? std::make_shared<SimpleThreadPool>() : thread_pool)
    , timer_(timer == nullptr ? std::make_shared<time::Timer>() : timer)
    , get_completion_port_(this)
{
}

Communication::~Communication()
{
}

jaf::Coroutine<void> Communication::Init()
{
    co_return;
}

jaf::Coroutine<void> Communication::Run()
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    wait_stop_.Start();
    wait_work_thread_finish_.Start(work_thread_count_);

    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    CreateWorkThread();

    co_await wait_stop_.Wait();

    run_flag_ = false;

    for (size_t i = 0; i < work_thread_count_; ++i)
    {
        PostQueuedCompletionStatus(m_completionPort, 0, (DWORD) NULL, NULL);
    }
    co_await wait_work_thread_finish_;

    CloseHandle(m_completionPort);
    m_completionPort = nullptr;

    co_return;
}

void Communication::Stop()
{
    wait_stop_.Stop();
}

std::shared_ptr<ITcpServer> Communication::CreateTcpServer()
{
    std::shared_ptr<TcpServer> server = std::make_shared<TcpServer>(&get_completion_port_, timer_);
    return std::static_pointer_cast<ITcpServer>(server);
}

std::shared_ptr<ITcpClient> Communication::CreateTcpClient()
{
    std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(&get_completion_port_, timer_);
    return std::static_pointer_cast<ITcpClient>(client);
}

std::shared_ptr<IUdp> Communication::CreateUdp()
{
    std::shared_ptr<Udp> udp = std::make_shared<Udp>(&get_completion_port_, timer_);
    return std::static_pointer_cast<IUdp>(udp);
}

std::shared_ptr<ISerialPort> Communication::CreateSerialPort()
{
    std::shared_ptr<SerialPort> server = std::make_shared<SerialPort>(&get_completion_port_, timer_);
    return std::static_pointer_cast<ISerialPort>(server);
}

void Communication::CreateWorkThread()
{
    SYSTEM_INFO mySysInfo;
    GetSystemInfo(&mySysInfo);
    // size_t work_thread_count = mySysInfo.dwNumberOfProcessors * 2;

    for (size_t i = 0; i < work_thread_count_; ++i)
    {
        thread_pool_->Post(std::bind(&Communication::WorkThreadRun, this));
    }
}

void Communication::WorkThreadRun()
{
    FINALLY(wait_work_thread_finish_.Notify(););

    ULONG_PTR completionKey = 0;
    IOCP_DATA* pPerIoData   = nullptr;
    DWORD bytesTransferred  = 0;
    while (run_flag_)
    {
        BOOL success = GetQueuedCompletionStatus(m_completionPort, &bytesTransferred, (PULONG_PTR) &completionKey, (LPOVERLAPPED*) &pPerIoData, INFINITE);

        if (pPerIoData == nullptr)
        {
            if (!success)
            {
                DWORD dw = GetLastError();
                if (WAIT_TIMEOUT == dw)
                {
                    continue;
                }

                std::string str = std::format("Communication code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
                LOG_ERROR(LOG_NAME) << str;
            }

            continue;
        }
        pPerIoData->success_          = success;
        pPerIoData->bytesTransferred_ = bytesTransferred;
        pPerIoData->call_(pPerIoData);
    }
}

} // namespace comm
} // namespace jaf