#pragma once
// MIT License
//
// Copyright(c) 2021 Jaf932074323
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 2024-6-16 姜安富

#define LOG_FATAL(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_FATAL, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_ERROR(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_WARNING(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_INFO(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_DEBUG(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
#define LOG_TRANCE(...) jaf::log::CreateLogEvent(jaf::log::LOG_LEVEL_TRANCE, __FILE__, __LINE__, __FUNCTION__, jaf::log::GetLog(__VA_ARGS__))
