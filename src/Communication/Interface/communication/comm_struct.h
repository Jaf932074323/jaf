#pragma once
#include <memory>
#include <functional>
#include <winsock2.h>

namespace jaf
{
namespace comm
{

struct IOCP_DATA
{
	OVERLAPPED	overlapped = { 0 };
	int success_ = 0;
	DWORD bytesTransferred_ = 0;
	std::function<void(IOCP_DATA*)> call_;
};

// ����
struct SData
{
	unsigned char* buff = nullptr;
	size_t len = 0;
};

// �Ѷ�ȡ����
struct SConstData
{
	const unsigned char* buff = nullptr;
	size_t len = 0;
};

}
}