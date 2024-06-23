#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

// ͬ������
template<typename Data>
class ConcurrentQueue
{
public:
	void Push(const Data& data)
	{
		{
			std::lock_guard<std::mutex> lg(data_queue_mutex_);
			data_queue_.push(data);
		}
		data_queue_cv_.notify_one();
	}

	bool Empty() const
	{
		std::lock_guard<std::mutex> lg(data_queue_mutex_);
		return data_queue_.empty();
	}

	bool TryPop(Data& data)
	{
		std::lock_guard<std::mutex> lg(data_queue_mutex_);
		if (data_queue_.empty())
		{
			return false;
		}
		
		data = data_queue_.front();
		data_queue_.pop();
		return true;
	}

	// ��û������ʱ�������ȴ� ֱ�������ݻ��߱�֪ͨ�˳�
	bool WaitAndPop(Data& data)
	{
		std::unique_lock<std::mutex> lg(data_queue_mutex_);
		while(data_queue_.empty())
		{
			data_queue_cv_.wait(lg);
			if (quit_wait_)
			{
				return false;
			}
		}

		data = data_queue_.front();
		data_queue_.pop();
		return true;		
	}
	
	// �������ȴ�ʱ���첽֪ͨ�˳��ȴ�
	void QuitAllWait()
	{
		{
			std::lock_guard<std::mutex> lg(data_queue_mutex_);
			quit_wait_ = true;
		}
		data_queue_cv_.notify_all();
	}

	void Reset()
	{
		std::lock_guard<std::mutex> lg(data_queue_mutex_);
		quit_wait_ = false;
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lg(data_queue_mutex_);
		quit_wait_ = false;
		swap(std::queue<Data>(), data_queue_); // ���
	}

private:
	std::condition_variable data_queue_cv_; // 
	std::mutex data_queue_mutex_; // ͬ��������
	std::queue<Data> data_queue_; // ���ݶ���
	bool quit_wait_ = false;
};