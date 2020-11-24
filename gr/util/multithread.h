#pragma once

#include "namespace.h"
#include <mutex>
#include <condition_variable>
#include <memory>

NAMESPACE_BEGIN(util)

struct ConditionalMutex {
  std::unique_ptr<std::recursive_mutex> _mutex;

  ConditionalMutex(bool lock) : _mutex(lock ? std::make_unique<std::recursive_mutex>() : nullptr) {}

  void lock() { if (_mutex) _mutex->lock(); }
  void unlock() { if (_mutex) _mutex->unlock(); }
  bool try_lock() { if (_mutex) return _mutex->try_lock(); return true; }
};

template <typename Data>
struct SharedQueue {
	void PushFront(Data &&data)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_queue.push_front(std::move(data));
		_hasData.notify_one();
	}

	void PushBack(Data &&data)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_queue.push_back(std::move(data));
		_hasData.notify_one();
	}

	Data PopFront()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		while (!_queue.size()) {
			_hasData.wait(lock);
		}
		Data front = std::move(_queue.front());
		_queue.pop_front();
		return front;
	}

	Data PopBack()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		while (!_queue.size()) {
			_hasData.wait(lock);
		}
		Data back = std::move(_queue.back());
		_queue.pop_back();
		return back;
	}

	size_t GetSize()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		return _queue.size();
	}

	template <typename Consumer>
	void Consume(Consumer consume) 
	{
		std::unique_lock<std::mutex> lock(_mutex);
		while (true) {
			while (!_queue.size()) {
				_hasData.wait(lock);
			}
			while (_queue.size()) {
				bool shouldContinue = consume(_queue);
				if (!shouldContinue)
					return;
			}
		}
	}

private:
	std::mutex _mutex;
	std::condition_variable _hasData;
	std::deque<Data> _queue;
};

NAMESPACE_END(util)