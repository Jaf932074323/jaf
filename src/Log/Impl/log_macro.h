#pragma once

#define LOG_FATAL(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_FATAL, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_ERROR(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_WARNING(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_INFO(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_DEBUG(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_TRANCE(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_TRANCE, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
