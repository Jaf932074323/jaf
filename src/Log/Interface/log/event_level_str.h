#pragma once
#include <map>

namespace jaf
{
namespace log
{

class LogLevelManage
{
	// 单例模式
	LogLevelManage()
		:event_levels_
		{
			{LOG_LEVEL_FATAL,"fatal"}
			, {LOG_LEVEL_ERROR,"error"}
			, {LOG_LEVEL_WARNING,"warning"}
			, {LOG_LEVEL_INFO,"info"}
			, {LOG_LEVEL_DEBUG,"debug"}
			, {LOG_LEVEL_TRANCE,"trance"}
		}
	{}
	LogLevelManage(const LogLevelManage&) = delete;
	LogLevelManage& operator=(const LogLevelManage&) = delete;
public:
	~LogLevelManage() {}
	static LogLevelManage& Instance()
	{
		static LogLevelManage instance; // 单例实例
		return instance;
	}

	static void Set(uint32_t level, const char* str)
	{
		Instance().event_levels_.insert(std::make_pair(level, str));
	}

	static const char* Get(uint32_t level)
	{
		auto it = Instance().event_levels_.find(level);
		if (it == Instance().event_levels_.end())
		{
			return "";
		}
		return it->second;;
	}

private:
	std::map<uint32_t, const char*> event_levels_; // key为日志等级，value为对应的字符串
};


}
}