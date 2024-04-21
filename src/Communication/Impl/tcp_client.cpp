#include "tcp_client.h"
#include <format>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <mswsock.h>
#include "tcp_channel.h"
#include "util/execute_at_end.h"
#include "Log/log_head.h"

namespace jaf
{
namespace comm
{

std::string GetFormatMessage(DWORD dw);

static LPFN_CONNECTEX GetConnectEx(SOCKET a_socket)
{
	LPFN_CONNECTEX func = nullptr;
	DWORD bytes;
	GUID guid = WSAID_CONNECTEX;
	if (WSAIoctl(a_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &func, sizeof(func), &bytes, nullptr, nullptr) == SOCKET_ERROR)
	{
		DWORD dw = GetLastError();
		std::string str = std::format("GetConnectEx code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
		LOG_ERROR() << str;
		return nullptr;
	}

	return func;
}

comm::TcpClient::TcpClient(std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port)
	: local_ip_(local_ip)
	, local_port_(local_port)
	, remote_ip_(remote_ip)
	, remote_port_(remote_port)
{
}

TcpClient::~TcpClient()
{
}

void TcpClient::SetChannelUser(std::shared_ptr<IChannelUser> user)
{
	user_ = user;
}

jaf::Coroutine<void> TcpClient::Run(HANDLE completion_handle)
{
	if (run_flag_)
	{
		co_return;
	}
	run_flag_ = true;

	completion_handle_ = completion_handle;
	Init();

	co_await Run();

	co_return;
}

void TcpClient::Stop()
{
}

void TcpClient::Init(void)
{
}

jaf::Coroutine<void> TcpClient::Run()
{
	SOCKET connect_socket = co_await ConnectAwaitable(this);
	if (INVALID_SOCKET == connect_socket)
	{
		LOG_ERROR() << std::format("TCP连接失败,本地端口 {}:{},远程端口 {}:{}", local_ip_, local_port_, remote_ip_, remote_port_);
		co_return;
	}
	EXECUTE_AT_END(	CancelIo((HANDLE)connect_socket); closesocket(connect_socket); );

	std::shared_ptr<TcpChannel> channel = std::make_shared<TcpChannel>(completion_handle_, connect_socket, remote_ip_, remote_port_, local_ip_, local_port_);
	
	if (co_await channel->Start())
	{
		co_await user_->Access(channel);
		channel->Stop();
	}

	co_return;
}

TcpClient::ConnectAwaitable::ConnectAwaitable(TcpClient* client)
	:client_(client)
{
}

TcpClient::ConnectAwaitable::~ConnectAwaitable()
{
}

bool TcpClient::ConnectAwaitable::await_ready()
{
	return false;
}

bool TcpClient::ConnectAwaitable::await_suspend(std::coroutine_handle<> co_handle)
{
	handle_ = co_handle;

	iocp_data_.call_ = std::bind(&ConnectAwaitable::IoCallback, this, std::placeholders::_1);

	socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == socket_)
	{
		DWORD dw = GetLastError();
		std::string str = std::format("TcpClient code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
		LOG_ERROR() << str;
		return false;
	}

	sockaddr_in bind_addr = {};
	inet_pton(AF_INET, client_->local_ip_.c_str(), (void*)&bind_addr.sin_addr);
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(client_->local_port_);

	//绑定套接字, 绑定到端口
	::bind(socket_, (SOCKADDR*)&bind_addr, sizeof(bind_addr));//会返回一个SOCKET_ERROR

	CreateIoCompletionPort((HANDLE)socket_, client_->completion_handle_, 0, 0);

	static LPFN_CONNECTEX func = GetConnectEx(socket_);

	sockaddr_in	connect_addr = { 0 };
	inet_pton(AF_INET, client_->remote_ip_.c_str(), (void*)&connect_addr.sin_addr);
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = htons(client_->remote_port_);
	DWORD bytes;
	bool connect_result = func(socket_, (const sockaddr*)&connect_addr, sizeof(connect_addr), nullptr, 0, &bytes, &iocp_data_.overlapped);

	if (!connect_result && WSAGetLastError() != ERROR_IO_PENDING)
	{
		DWORD dw = GetLastError();
		std::string str = std::format("TcpClient code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
		OutputDebugString(str.c_str());
		closesocket(socket_);
		socket_ = INVALID_SOCKET;
		return false;
	}

	return true;
}

SOCKET TcpClient::ConnectAwaitable::await_resume()
{
	return socket_;
}

void TcpClient::ConnectAwaitable::IoCallback(IOCP_DATA* pData)
{
	if (pData->success_ == 0)
	{
		DWORD dw = GetLastError();
		std::string str = std::format("Iocp code error: {} \t  error-msg: {}\r\n", dw, GetFormatMessage(dw));
		LOG_ERROR() << str;

		closesocket(socket_);
		socket_ = INVALID_SOCKET;
	}

	handle_.resume();
}

}
}