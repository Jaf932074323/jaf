#pragma once
#include "Impl/co_notify.h"
#include "util/co_coroutine.h"
#include <type_traits>

namespace jaf
{
namespace comm
{

// �ȴ�ֱ����ʱ
// WaitObj ��һ��Э����صĿɵȴ����󣬲���Ҫӵ�и���ֹͣ����Stop()
template <typename WaitObj>
struct RunUntilTimeout
{
    //using Result = decltype(std::declval<WaitObj>().await_resume());
    using WaitObjResult = std::invoke_result_t<decltype(&WaitObj::await_resume), WaitObj*>;

public:
    RunUntilTimeout(jaf::time::CoNotify& notify, WaitObj& wait_obj, uint64_t timeout)
        : notify_(notify)
        , wait_obj_(wait_obj)
        , timeout_(timeout)
    {
    }

public:
    // �Ƿ�ʱtrue��ʱ,falseδ��ʱ
    bool IsTimeout()
    {
        return timeout_flag_;
    }
    // ��ȡ�ȴ����
    const WaitObjResult& Result()
    {
        return wait_obj_result_;
    }

public:
    Coroutine<void> Run()
    {
        Coroutine<void> run_wait_obj = RunWaitObj();
        Coroutine<void> wait_timeout = WaitTimeout();

        co_await run_wait_obj;
        co_await wait_timeout;
    }

private:
    Coroutine<void> RunWaitObj()
    {
        wait_obj_result_ = co_await wait_obj_;
        notify_.Notify();
        co_return;
    };

    Coroutine<void> WaitTimeout()
    {
        timeout_flag_ = !co_await notify_.Wait(timeout_);
        wait_obj_.Stop();
        co_return;
    };


private:
    jaf::time::CoNotify& notify_;
    WaitObj& wait_obj_; // Э����صĿɵȴ��Ķ���
    uint64_t timeout_;  // ��ʱʱ�� ����

    bool timeout_flag_ = false;          // �Ƿ�ʱtrue��ʱ,falseδ��ʱ
    WaitObjResult wait_obj_result_; // �ȴ����
};

} // namespace comm
} // namespace jaf