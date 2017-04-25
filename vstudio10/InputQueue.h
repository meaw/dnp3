#ifndef _SAFE_IQUEUE
#define _SAFE_IQUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

class CommandsQueue
{
public:
	int type;
	int	value;
	int	index;
	CommandsQueue(int _type, int _value, int _index) {
		type = _type;
		value = _type;
		index = _type;
	}
};

// A threadsafe-queue.
template <class T>
class SafeQueue
{
private:
	int Max_Size = 100;
	
public:
	int Period = 100;   //how often to report
	int Interval = 10;  //How often shall the queue be processed
    int Lost = 0;
	int Received = 0;
	SafeQueue()
		: q()
		, m()
		, c()
	{
		Max_Size = 100;
	}

	~SafeQueue(void)
	{}

	void SetSize(int _MaxSize) {
		Max_Size = _MaxSize;
	}
	void Reset() {
		std::lock_guard<std::mutex> lock(m);
		Received = 0;
		Lost = 0;
		c.notify_one();
	}

	// Add an element to the queue.
	int Enqueue(T t)
	{
		std::lock_guard<std::mutex> lock(m);
		if (q.size() > Max_Size - 1) {
			Lost++;
			c.notify_one();
			return 0;
		}
		else{
		q.push(t);

		Received++;
		c.notify_one();
		return 1;
		}
	}

	int Size()
	{
		return q.size();
	}

	// Get the "front"-element.
	// If the queue is empty, wait till a element is avaiable.
	T Dequeue(void)
	{
		std::unique_lock<std::mutex> lock(m);
		while (q.empty())
		{
			// release lock as long as the wait and reaquire it afterwards.
			c.wait(lock);
		}
		T val = q.front();
		q.pop();
		return val;
	}

private:
	std::queue<T> q;
	mutable std::mutex m;
	std::condition_variable c;
};

#endif