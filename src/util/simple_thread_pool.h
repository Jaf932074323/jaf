#pragma once
#include "i_thread_pool.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace jaf
{

// 简易线程池
class SimpleThreadPool :public IThreadPool
{
public:
	SimpleThreadPool(size_t thread_count = 10)
		: thread_count_(thread_count)
	{
		Start();
	}
	~SimpleThreadPool()
	{
		Stop();
	}

	virtual void Post(std::function<void(void)> fun)
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		tasks_.emplace(std::move(fun));
		condition_.notify_one();
	}

private:
	void Start()
	{
		for (size_t i = 0; i < thread_count_; ++i)
		{
			workers_.emplace_back([this]() { WorkerRun(); });
		}
	}
	void Stop()
	{
		stop_ = true;

		{
			std::unique_lock<std::mutex> lock(queue_mutex_);
			condition_.notify_all();
		}

		for (auto& worker : workers_)
		{
			worker.join();
		}
	}
	void WorkerRun()
	{
		while (!stop_)
		{
			std::function<void()> task;

			{
				std::unique_lock<std::mutex> lock(queue_mutex_);
				condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
				if (stop_)
				{
					return;
				}
				if (tasks_.empty())
				{
					continue;
				}

				task = std::move(tasks_.front());
				tasks_.pop();
			}

			task();
		}


	}

private:
	bool stop_ = false;

	size_t thread_count_; // 线程个数
	std::vector< std::thread > workers_;

	std::mutex queue_mutex_;
	std::condition_variable condition_;
	std::queue<std::function<void(void)>> tasks_;
};

}
