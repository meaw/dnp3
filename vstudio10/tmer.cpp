#include "Tmer.h"

void Tmer::threadStart()
{
	ScopedLock lock(sync);

	while (!done)
	{
		if (queue.empty())
		{
			// Wait (forever) for work
			wakeUp.wait(lock);
		}
		else
		{
			auto firstInstance = queue.begin();
			Instance& instance = *firstInstance;
			auto now = Clock::now();
			if (now >= instance.next)
			{
				queue.erase(firstInstance);

				// Mark it as running to handle racing destroy
				instance.running = true;

				// Call the handler
				lock.unlock();
				instance.handler();
				lock.lock();

				if (done)
				{
					break;
				}
				else if (!instance.running)
				{
					// Running was set to false, destroy was called
					// for this Instance while the callback was in progress
					// (this thread was not holding the lock during the callback)
					active.erase(instance.id);
				}
				else
				{
					instance.running = false;

					// If it is periodic, schedule a new one
					if (instance.period.count() > 0)
					{
						instance.next = instance.next + instance.period;
						queue.insert(instance);
					}
					else {
						active.erase(instance.id);
					}
				}
			}
			else {
				// Wait until the Tmer is ready or a Tmer creation notifies
				wakeUp.wait_until(lock, instance.next);
			}
		}
	}
}

Tmer::Tmer()
	: nextId(1)
	, queue(comparator)
	, done(false)
{
	ScopedLock lock(sync);
	worker = std::thread(std::bind(&Tmer::threadStart, this));
}

Tmer::~Tmer()
{
	ScopedLock lock(sync);
	done = true;
	wakeUp.notify_all();
	lock.unlock();
	worker.join();
}

Tmer::Tmer_id Tmer::create(uint64_t msFromNow, uint64_t msPeriod,
	const std::function<void()> &handler)
{
	return createImpl(Instance(0,
		Clock::now() + Duration(msFromNow), Duration(msPeriod),
		handler));
}

Tmer::Tmer_id Tmer::create(uint64_t msFromNow, uint64_t msPeriod,
	std::function<void()>&& handler)
{
	return createImpl(Instance(0,
		Clock::now() + Duration(msFromNow), Duration(msPeriod),
		std::move(handler)));
}

Tmer::Tmer_id Tmer::createImpl(Instance&& item)
{
	ScopedLock lock(sync);
	item.id = nextId++;
	auto iter = active.emplace(item.id, std::move(item));
	queue.insert(iter.first->second);
	wakeUp.notify_all();
	return item.id;
}

bool Tmer::destroy(Tmer_id id)
{
	ScopedLock lock(sync);
	auto i = active.find(id);
	if (i == active.end())
		return false;
	else if (i->second.running)
	{
		// A callback is in progress for this Instance,
		// so flag it for deletion in the worker
		i->second.running = false;
	}
	else
	{
		queue.erase(std::ref(i->second));
		active.erase(i);
	}

	wakeUp.notify_all();
	return true;
}

bool Tmer::exists(Tmer_id id)
{
	ScopedLock lock(sync);
	return active.find(id) != active.end();
}