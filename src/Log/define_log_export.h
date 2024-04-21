#pragma once

#ifdef _API_LOG_EXPORT
#define API_LOG_EXPORT __declspec(dllexport)
#else
#define API_LOG_EXPORT
#endif