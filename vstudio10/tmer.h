#ifndef Tmer_H
#define Tmer_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <functional>
#include <chrono>
#include <unordered_map>
#include <set>
#include <cstdint>

class Tmer
{
public:
	typedef uint64_t Tmer_id;
	typedef std::function<void()> handler_type;

private:
	std::mutex sync;
	typedef std::unique_lock<std::mutex> ScopedLock;

	std::condition_variable wakeUp;

private:
	typedef std::chrono::steady_clock Clock;
	typedef std::chrono::time_point<Clock> Timestamp;
	typedef std::chrono::milliseconds Duration;

	struct Instance
	{
		Instance(Tmer_id id = 0)
			: id(id)
			, running(false)
		{
		}

		template<typename Tfunction>
		Instance(Tmer_id id, Timestamp next, Duration period, Tfunction&& handler) noexcept
			: id(id)
			, next(next)
			, period(period)
			, handler(std::forward<Tfunction>(handler))
			, running(false)
		{
		}

		Instance(Instance const& r) = delete;

		Instance(Instance&& r) noexcept
			: id(r.id)
			, next(r.next)
			, period(r.period)
			, handler(std::move(r.handler))
			, running(r.running)
		{
		}

		Instance& operator=(Instance const& r) = delete;

		Instance& operator=(Instance&& r)
		{
			if (this != &r)
			{
				id = r.id;
				next = r.next;
				period = r.period;
				handler = std::move(r.handler);
				running = r.running;
			}
			return *this;
		}

		Tmer_id id;
		Timestamp next;
		Duration period;
		handler_type handler;
		bool running;
	};

	typedef std::unordered_map<Tmer_id, Instance> InstanceMap;
	Tmer_id nextId;
	InstanceMap active;

	// Comparison functor to sort the Tmer "queue" by Instance::next
	struct NextActiveComparator
	{
		bool operator()(const Instance &a, const Instance &b) const
		{
			return a.next < b.next;
		}
	};
	NextActiveComparator comparator;

	// Queue is a set of references to Instance objects, sorted by next
	typedef std::reference_wrapper<Instance> QueueValue;
	typedef std::multiset<QueueValue, NextActiveComparator> Queue;
	Queue queue;

	// Thread and exit flag
	std::thread worker;
	bool done;
	void threadStart();

public:
	Tmer();
	~Tmer();

	Tmer_id create(uint64_t when, uint64_t period, const handler_type& handler);
	Tmer_id create(uint64_t when, uint64_t period, handler_type&& handler);

private:
	Tmer_id createImpl(Instance&& item);

public:
	bool destroy(Tmer_id id);

	bool exists(Tmer_id id);
};

#endif // Tmer_H
