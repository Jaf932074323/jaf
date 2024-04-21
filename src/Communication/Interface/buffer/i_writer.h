#pragma once

namespace jaf
{
namespace buffer
{

// 数据写入器
class IWriter
{
public:
	IWriter(){}
	virtual ~IWriter() {};

public:
	// 设置总容量至少为size
	virtual void Reserve(size_t size) = 0;
	// 扩展未使用容量到至少size
	virtual void Extend(size_t size) = 0;
	// 获取总容量
	virtual size_t Capacity() = 0;
	// 获取未使用容量
	virtual size_t UnusedCapacity() = 0;

	virtual const unsigned char* GetBuffer() const = 0;
	virtual size_t GetBufferLen() const = 0;

public:
	// 写入数据
	virtual bool Write(unsigned char* src, size_t len) = 0;
};

}
}