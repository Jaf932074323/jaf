#pragma once
#include "impl/co_await_time.h"
#include "interface/communication/i_channel.h"
#include "interface/communication/i_channel_user.h"
#include "interface/communication/i_unpack.h"
#include "time_head.h"
#include "util/co_await.h"
#include "util/co_coroutine.h"
#include <string>

namespace jaf
{
namespace comm
{

// TCP�ͻ���
class TcpClient
{
public:
    TcpClient(std::string remote_ip, uint16_t remote_port, std::string local_ip = "0.0.0.0", uint16_t local_port = 0, std::shared_ptr<jaf::time::ITimer> timer = nullptr);
    virtual ~TcpClient();

public:
    // ��������ʱ��
    // connect_timeout ���ӳ�ʱʱ��
    // reconnect_wait_time �����ȴ�ʱ��
    virtual void SetConnectTime(uint64_t connect_timeout, uint64_t reconnect_wait_time);
    void SetChannelUser(std::shared_ptr<IChannelUser> user);
    virtual jaf::Coroutine<void> Run(HANDLE completion_handle);
    virtual void Stop();

private:
    void Init(void);
    jaf::Coroutine<void> Execute();

    SOCKET CreationSocket();

private:
    struct ConnectResult;
    class ConnectAwaitable;

private:
    bool run_flag_ = false;
    CoAwait await_stop_;

    std::shared_ptr<jaf::time::ITimer> timer_;
    jaf::time::CoAwaitTime await_time_;

    HANDLE completion_handle_ = nullptr;

    std::string local_ip_ = "0.0.0.0";
    uint16_t local_port_  = 0;
    std::string remote_ip_;
    uint16_t remote_port_         = 0;
    uint64_t connect_timeout_     = 5000; // ���ӵȴ�ʱ��
    uint64_t reconnect_wait_time_ = 5000; // �����ȴ�ʱ��

    std::string error_info_;

    std::shared_ptr<IChannelUser> user_ = nullptr; // ͨ��ʹ����

    std::mutex channel_mutex_;
    std::shared_ptr<IChannel> channel_ = nullptr;
};

} // namespace comm
} // namespace jaf