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
#include <functional>

namespace jaf
{
namespace time
{

enum class ETimerResultType
{
    TRT_SUCCESS    = 0, // 定时成功
    TRT_TASK_STOP  = 1, // 这个定时任务停止
    TRT_TIMER_STOP = 2, // 定时器停止
};

// 定时参数
struct STimerTask
{
    std::function<void(ETimerResultType result_type, STimerTask* task)> fun; // 定时执行函数
    uint64_t interval = 1000;                              // 定时时间间隔（毫秒）
    void* timer_data_ = nullptr;                           // 定时器维护的数据，由定时器记录其自身需要的数据，只能定时器修改
};

// 定时器接口
class ITimer
{
public:
    virtual ~ITimer(){};

public:
    // 启动定时器
    virtual void Start() = 0;
    // 停止定时器
    virtual void Stop() = 0;
    // 启动一个定时任务
    // task 定时任务信息，在定时任务结束之前，需要保证task有效，否则在定时回调时，可能会因为task无效而崩溃
    // 返回是否成功
    // 当StartTask成功后，para中的定时执行函数一定要执行一次，且只会执行一次
    virtual bool StartTask(STimerTask* task) = 0;
    // 停止一个定时任务
    // task 定时任务信息
    virtual void StopTask(STimerTask* task) = 0;
};

} // namespace time
} // namespace jaf