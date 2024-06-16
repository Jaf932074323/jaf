#include "Iocp.h"
#include "define_constant.h"
#include "log_head.h"
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

Iocp::Iocp(std::shared_ptr<IThreadPool> thread_pool)
    : thread_pool_(thread_pool == nullptr ? std::make_shared<SimpleThreadPool>() : thread_pool)
{
}

Iocp::~Iocp()
{
    await_work_thread_finish_.Stop();
}

jaf::Coroutine<void> Iocp::Init()
{
    co_return;
}

jaf::Coroutine<void> Iocp::Run()
{
    if (run_flag_)
    {
        co_return;
    }
    run_flag_ = true;

    await_stop_.Start();
    await_work_thread_finish_.Start(work_thread_count_);

    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    CreateWorkThread();

    co_await await_stop_.Wait();

    run_flag_ = false;

    for (size_t i = 0; i < work_thread_count_; ++i)
    {
        PostQueuedCompletionStatus(m_completionPort, 0, (DWORD) NULL, NULL);
    }
    co_await await_work_thread_finish_.Wait();

    CloseHandle(m_completionPort);
    m_completionPort = nullptr;

    co_return;
}

void Iocp::Stop()
{
    await_stop_.Stop();
}

void Iocp::CreateWorkThread()
{
    SYSTEM_INFO mySysInfo;
    GetSystemInfo(&mySysInfo);
    // size_t work_thread_count = mySysInfo.dwNumberOfProcessors * 2;

    for (size_t i = 0; i < work_thread_count_; ++i)
    {
        thread_pool_->Post(std::bind(&Iocp::WorkThreadRun, this));
    }
}

void Iocp::WorkThreadRun()
{
    FINALLY(await_work_thread_finish_.Notify(););

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

                std::string str = std::format("Iocp code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
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