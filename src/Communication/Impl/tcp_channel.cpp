#include "tcp_channel.h"
#include "Log/log_head.h"
#include <WS2tcpip.h>
#include <assert.h>
#include <format>
#include <mswsock.h>

namespace jaf {
namespace comm {

std::string GetFormatMessage(DWORD dw);

class TcpChannel::ReadAwaitable {
public:
  ReadAwaitable(TcpChannel *tcp_channel, SOCKET socket, unsigned char *buff,
                size_t size, uint64_t timeout);
  ~ReadAwaitable();
  bool await_ready();
  bool await_suspend(std::coroutine_handle<> co_handle);
  size_t await_resume() const;
  void IoCallback(IOCP_DATA *pData);
  void TimerCallback();

private:
  TcpChannel *tcp_channel_ = nullptr;
  SOCKET socket_ = 0;   // 收发数据的套接字
  size_t recv_len_ = 0; // 接收数据长度

  IOCP_DATA iocp_data_;
  std::coroutine_handle<> handle;
  WSABUF wsbuffer_;

  uint64_t timeout_;
  uint64_t timer_id_ = 0; // 定时ID
};

class TcpChannel::WriteAwaitable {
public:
  WriteAwaitable(TcpChannel *tcp_channel, SOCKET socket,
                 const unsigned char *buff, size_t size);
  ~WriteAwaitable();
  bool await_ready();
  bool await_suspend(std::coroutine_handle<> co_handle);
  size_t await_resume() const;
  void IoCallback(IOCP_DATA *pData);

private:
  TcpChannel *tcp_channel_ = nullptr;
  SOCKET socket_ = 0;    // 收发数据的套接字
  size_t write_len_ = 0; // 接收数据长度

  IOCP_DATA iocp_data_;
  std::coroutine_handle<> handle;

  WSABUF wsbuffer_;

  DWORD dwBytes = 0;
};

TcpChannel::TcpChannel(HANDLE completion_handle, SOCKET socket,
                       std::string remote_ip, uint16_t remote_port,
                       std::string local_ip, uint16_t local_port,
                       std::shared_ptr<jaf::time::Timer> timer)
    : completion_handle_(completion_handle), socket_(socket),
      remote_ip_(remote_ip), remote_port_(remote_port), local_ip_(local_ip),
      local_port_(local_port),
      timer_(timer == nullptr ? jaf::time::CommonTimer::Timer() : timer) {}

TcpChannel::~TcpChannel() {}

Coroutine<bool> TcpChannel::Start() {
  CreateIoCompletionPort((HANDLE)socket_, completion_handle_, 0, 0);
  co_return true;
}

Coroutine<bool> TcpChannel::Read(unsigned char *buff, size_t buff_size,
                                 size_t *recv_len, uint64_t timeout) {
  assert(recv_len != nullptr);
  *recv_len = co_await ReadAwaitable{this, socket_, buff, buff_size, timeout};

  if (*recv_len == 0) {
    co_return false;
  }
  co_return true;
}

Coroutine<bool> TcpChannel::Write(const unsigned char *buff, size_t buff_size,
                                  uint64_t timeout) {
  co_await WriteAwaitable{this, socket_, buff, buff_size};
  co_return true;
}

void TcpChannel::Stop() { closesocket(socket_); }

TcpChannel::ReadAwaitable::ReadAwaitable(TcpChannel *tcp_channel, SOCKET socket,
                                         unsigned char *buff, size_t size,
                                         uint64_t timeout)
    : tcp_channel_(tcp_channel), socket_(socket),
      wsbuffer_{.len = (ULONG)size, .buf = (char *)buff}, timeout_(timeout) {}

TcpChannel::ReadAwaitable::~ReadAwaitable() {}
bool TcpChannel::ReadAwaitable::await_ready() { return false; }

bool TcpChannel::ReadAwaitable::await_suspend(
    std::coroutine_handle<> co_handle) {
  DWORD flags = 0;
  handle = co_handle;

  iocp_data_.call_ = [this](IOCP_DATA *pData) { IoCallback(pData); };

  if (SOCKET_ERROR == WSARecv(socket_, &wsbuffer_, 1, nullptr, &flags,
                              &iocp_data_.overlapped, NULL)) {
    int error = WSAGetLastError();
    if (error != ERROR_IO_PENDING) {
      std::string str =
          std::format("TcpChannel code error: {} \t  error-msg: {}\r\n", error,
                      GetFormatMessage(error));
      LOG_ERROR() << str;
      return false;
    }
  }

  jaf::time::STimerTask task{[this]() { TimerCallback(); }, timeout_};
  tcp_channel_->timer_->AddTask(task);

  return true;
}

size_t TcpChannel::ReadAwaitable::await_resume() const { return recv_len_; }

void TcpChannel::ReadAwaitable::IoCallback(IOCP_DATA *pData) {
  if (pData->success_ == 0) {
    DWORD dw = GetLastError();
    if (WAIT_TIMEOUT != dw) {
      std::string str =
          std::format("TcpChannel code error: {} \t  error-msg: {}\r\n", dw,
                      GetFormatMessage(dw));
      OutputDebugString(str.c_str());
    }

    // LINGER lingerStruct;
    // lingerStruct.l_onoff = 1;
    // lingerStruct.l_linger = 0;
    // setsockopt(socket_, SOL_SOCKET, SO_LINGER,(char*)&lingerStruct,
    // sizeof(lingerStruct));

    CancelIo((HANDLE)socket_);
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    recv_len_ = 0;
  } else {
    recv_len_ = (size_t)pData->bytesTransferred_;
  }

  handle.resume();
}

void TcpChannel::ReadAwaitable::TimerCallback() {
  CancelIoEx((HANDLE)socket_, &iocp_data_.overlapped);
}

TcpChannel::WriteAwaitable::WriteAwaitable(TcpChannel *tcp_channel,
                                           SOCKET socket,
                                           const unsigned char *buff,
                                           size_t size)
    : tcp_channel_(tcp_channel), socket_(socket),
      wsbuffer_{.len = (ULONG)size, .buf = (char *)buff}
// TODO: 写入的时候是否会修改buff，需要后续检查
{}

TcpChannel::WriteAwaitable::~WriteAwaitable() {}

bool TcpChannel::WriteAwaitable::await_ready() { return false; }

bool TcpChannel::WriteAwaitable::await_suspend(
    std::coroutine_handle<> co_handle) {
  DWORD flags = 0;
  handle = co_handle;

  iocp_data_.call_ =
      std::bind(&WriteAwaitable::IoCallback, this, std::placeholders::_1);

  int send = WSASend(socket_, &wsbuffer_, 1, nullptr, flags,
                     &iocp_data_.overlapped, NULL);

  return true;
}

size_t TcpChannel::WriteAwaitable::await_resume() const { return write_len_; }

void TcpChannel::WriteAwaitable::IoCallback(IOCP_DATA *pData) {
  if (pData->success_ == 0) {
    DWORD dw = GetLastError();
    if (WAIT_TIMEOUT != dw) {
      std::string str =
          std::format("TcpChannel code error: {} \t  error-msg: {}\r\n", dw,
                      GetFormatMessage(dw));
      OutputDebugString(str.c_str());
    }

    CancelIo((HANDLE)socket_);
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    write_len_ = 0;
  } else {
    write_len_ = (size_t)pData->bytesTransferred_;
  }

  handle.resume();
}

} // namespace comm
} // namespace jaf