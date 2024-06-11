#pragma once
#include "define_log_export.h"
#include <memory>
#include <thread>

namespace jaf
{
namespace log
{

constexpr uint32_t LOG_LEVEL_FATAL   = 0; // ��������
constexpr uint32_t LOG_LEVEL_ERROR   = 1; // ����
constexpr uint32_t LOG_LEVEL_WARNING = 2; // ����
constexpr uint32_t LOG_LEVEL_INFO    = 3; // ��Ϣ
constexpr uint32_t LOG_LEVEL_DEBUG   = 4; // ������Ϣ
constexpr uint32_t LOG_LEVEL_TRANCE  = 5; // ׷����Ϣ

// ��־�¼�
class API_LOG_EXPORT IEvent
{
public:
    IEvent(){};
    virtual ~IEvent() {}

public:
    virtual uint32_t Level() const       = 0;
    virtual uint32_t GroupNumber() const = 0;
    virtual uint64_t ThreadId() const    = 0;
    virtual uint64_t Time() const        = 0;
    virtual const char* FileName() const = 0;
    virtual uint32_t Line() const        = 0;
    virtual const char* Info() const     = 0;
};

} // namespace log
} // namespace jaf