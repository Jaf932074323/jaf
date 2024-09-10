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
// 2024-6-23 姜安富
#include <queue>
#include <mutex>
#include <condition_variable>

// 同步队列
template<typename Data>
class ConcurrentQueue
{
public:
	void Push(const Data& data)
	{
		{
			std::unique_lock<std::mutex> lg(data_queue_mutex_);
			data_queue_.push(data);
		}
		data_queue_cv_.notify_one();
	}

	bool Empty() const
	{
		std::unique_lock<std::mutex> lg(data_queue_mutex_);
		return data_queue_.empty();
	}

	bool TryPop(Data& data)
	{
		std::unique_lock<std::mutex> lg(data_queue_mutex_);
		if (data_queue_.empty())
		{
			return false;
		}
		
		data = data_queue_.front();
		data_queue_.pop();
		return true;
	}

	// 当没有数据时就阻塞等待 直到有数据或者被通知退出
	bool WaitAndPop(Data& data)
	{
		std::unique_lock<std::mutex> lg(data_queue_mutex_);
		while(data_queue_.empty())
		{
			if (quit_wait_)
			{
				return false;
			}
			data_queue_cv_.wait(lg);
		}

		data = data_queue_.front();
		data_queue_.pop();
		return true;		
	}
	
	// 当阻塞等待时，异步通知退出等待
	void QuitAllWait()
	{
		{
			std::unique_lock<std::mutex> lg(data_queue_mutex_);
			quit_wait_ = true;
		}
		data_queue_cv_.notify_all();
	}

	void Reset()
	{
		std::unique_lock<std::mutex> lg(data_queue_mutex_);
		quit_wait_ = false;
	}

	void Clear()
	{
		std::unique_lock<std::mutex> lg(data_queue_mutex_);
		quit_wait_ = false;
		swap(std::queue<Data>(), data_queue_); // 清空
	}

private:
	std::condition_variable data_queue_cv_; // 
	std::mutex data_queue_mutex_; // 同步互斥量
	std::queue<Data> data_queue_; // 数据队列
	bool quit_wait_ = false;
};