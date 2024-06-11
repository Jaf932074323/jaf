#pragma once
#include "impl/co_await_time.h"
#include "util/co_coroutine.h"
#include <type_traits>

namespace jaf
{
namespace comm
{

// ����ֱ����ʱ
// AwaitObj ��һ��Э����صĿɵȴ����󣬲���Ҫӵ��ֹͣ����Stop()
template <typename AwaitObj>
struct RunWithTimeout
{
    using AwaitObjResult = std::invoke_result_t<decltype(&AwaitObj::await_resume), AwaitObj*>;

public:
    RunWithTimeout(jaf::time::CoAwaitTime& await_time, AwaitObj& await_obj, uint64_t timeout)
        : await_time_(await_time)
        , await_obj_(await_obj)
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
    const AwaitObjResult& Result()
    {
        return await_obj_result_;
    }

public:
    Coroutine<void> Run()
    {
        Coroutine<void> run_wait_obj = RunAwaitObj();
        Coroutine<void> wait_timeout = WaitTimeout();

        co_await run_wait_obj;
        co_await wait_timeout;
    }

private:
    Coroutine<void> RunAwaitObj()
    {
        await_obj_result_ = co_await await_obj_;
        await_time_.Notify();
        co_return;
    };

    Coroutine<void> WaitTimeout()
    {
        timeout_flag_ = !co_await await_time_.Wait(timeout_);
        await_obj_.Stop();
        co_return;
    };

private:
    jaf::time::CoAwaitTime& await_time_;
    AwaitObj& await_obj_; // Э����صĿɵȴ��Ķ���
    uint64_t timeout_;    // ��ʱʱ�� ����

    bool timeout_flag_ = false;       // �Ƿ�ʱtrue��ʱ,falseδ��ʱ
    AwaitObjResult await_obj_result_; // �ȴ����
};

} // namespace comm
} // namespace jaf