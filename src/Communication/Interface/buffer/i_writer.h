#pragma once

namespace jaf
{
namespace buffer
{

// ����д����
class IWriter
{
public:
	IWriter(){}
	virtual ~IWriter() {};

public:
	// ��������������Ϊsize
	virtual void Reserve(size_t size) = 0;
	// ��չδʹ������������size
	virtual void Extend(size_t size) = 0;
	// ��ȡ������
	virtual size_t Capacity() = 0;
	// ��ȡδʹ������
	virtual size_t UnusedCapacity() = 0;

	virtual const unsigned char* GetBuffer() const = 0;
	virtual size_t GetBufferLen() const = 0;

public:
	// д������
	virtual bool Write(unsigned char* src, size_t len) = 0;
};

}
}