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
#include "define_global_thread_pool_export.h"
#include "util/i_thread_pool.h"
#include <memory>

namespace jaf
{

// 公用线程池管理
class API_GLOBAL_THREAD_POOL_EXPORT GlobalThreadPool
{
private:
    GlobalThreadPool() {}
public:
    virtual ~GlobalThreadPool() {}

public:
    static std::shared_ptr<IThreadPool> ThreadPool();
    static void SetThreadPool(std::shared_ptr<IThreadPool> thread_pool);

private:
    static std::shared_ptr<IThreadPool>& ThreadPoolInstance();
};

} // namespace jaf
