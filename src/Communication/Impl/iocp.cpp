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
    : thread_pool_(thread_pool == nullptr ? std::make_shared<SimpleThreadPool>() : thread_pool) {}

Iocp::~Iocp()
{
}

void Iocp::Init()
{
}

void Iocp::Start()
{
    if (run_flag_)
    {
        return;
    }
    run_flag_ = true;

    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    CreateWorkThread();
}

void Iocp::Stop()
{
    if (!run_flag_)
    {
        return;
    }
    run_flag_ = false;
    CloseHandle(m_completionPort);
    m_completionPort = nullptr;
}

void Iocp::WaitEnd()
{
    work_threads_latch_->wait();
}

void Iocp::CreateWorkThread()
{
    SYSTEM_INFO mySysInfo;
    GetSystemInfo(&mySysInfo);
    // size_t work_thread_count = mySysInfo.dwNumberOfProcessors * 2;
    size_t work_thread_count = 1;

    work_threads_latch_ = std::make_shared<std::latch>(work_thread_count);
    for (size_t i = 0; i < work_thread_count; ++i)
    {
        thread_pool_->Post(std::bind(&Iocp::WorkThreadRun, this));
    }
}

void Iocp::WorkThreadRun()
{
    FINALLY(work_threads_latch_->count_down(););

    ULONG_PTR completionKey = 0;
    IOCP_DATA* pPerIoData   = nullptr;
    DWORD bytesTransferred  = 0;
    while (run_flag_)
    {
        BOOL success = GetQueuedCompletionStatus(m_completionPort, &bytesTransferred, (PULONG_PTR) &completionKey, (LPOVERLAPPED*) &pPerIoData, INFINITE);

        if (!success)
        {
            if (pPerIoData == nullptr)
            {
                DWORD dw = GetLastError();
                if (WAIT_TIMEOUT == dw)
                {
                    continue;
                }

                std::string str = std::format("Iocp code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
                LOG_ERROR(LOG_NAME) << str;
            }
        }

        if (pPerIoData == nullptr)
        {
            continue;
        }
        pPerIoData->success_          = success;
        pPerIoData->bytesTransferred_ = bytesTransferred;
        pPerIoData->call_(pPerIoData);
    }
}

} // namespace comm
} // namespace jaf