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
// 2024-10-15 姜安富
#include "global_thread_pool/global_thread_pool.h"

namespace jaf
{

std::shared_ptr<IThreadPool> GlobalThreadPool::ThreadPool()
{
    return ThreadPoolInstance();
}

void GlobalThreadPool::SetThreadPool(std::shared_ptr<IThreadPool> thread_pool)
{
    ThreadPoolInstance() = thread_pool;
}

std::shared_ptr<IThreadPool>& GlobalThreadPool::ThreadPoolInstance()
{
    static std::shared_ptr<IThreadPool> thread_pool_ = nullptr;
    return thread_pool_;
}

} // namespace jaf
