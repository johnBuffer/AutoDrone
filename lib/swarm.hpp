#pragma once

#include <thread>
#include <mutex>
#include <list>
#include <functional>
#include <atomic>
#include <iostream>
#include <chrono>
#include <condition_variable>
#include <memory>

namespace swrm
{

class Swarm;
class ExecutionGroup;

using WorkerFunction = std::function<void(uint32_t, uint32_t)>;

class Worker
{
public:
	Worker(Swarm* swarm);

	void createThread();

	void lockReady();
	void lockDone();
	void unlockReady();
	void unlockDone();

	void setJob(uint32_t id, ExecutionGroup* group);
	void stop();
	void join();

private:
	bool m_running;
	uint32_t m_id;
	uint32_t m_group_size;

	Swarm* m_swarm;
	ExecutionGroup* m_group;
	std::thread     m_thread;
	WorkerFunction  m_job;

	std::mutex m_ready_mutex;
	std::mutex m_done_mutex;

	void run();
	void waitReady();
	void waitDone();
};

class Synchronizer
{
public:
	static void lockAtReady(std::list<Worker*>& workers)
	{
		for (Worker* worker : workers) {
			worker->lockReady();
		}
	}

	static void unlockAtReady(std::list<Worker*>& workers)
	{
		for (Worker* worker : workers) {
			worker->unlockReady();
		}
	}

	static void lockAtDone(std::list<Worker*>& workers)
	{
		for (Worker* worker : workers) {
			worker->lockDone();
		}
	}

	static void unlockAtDone(std::list<Worker*>& workers)
	{
		for (Worker* worker : workers) {
			worker->unlockDone();
		}
	}

	static void stop(std::list<Worker*>& workers)
	{
		for (Worker* worker : workers) {
			worker->stop();
		}
	}

	static void join(std::list<Worker*>& workers)
	{
		for (Worker* worker : workers) {
			worker->join();
		}
	}
};

class ExecutionGroup
{
public:
	ExecutionGroup(WorkerFunction job, uint32_t group_size, std::list<Worker*>& available_workers)
		: m_job(job)
		, m_group_size(group_size)
		, m_done_count(0U)
		, m_condition()
		, m_condition_mutex()
	{
		retrieveWorkers(available_workers);
		start();
	}

	~ExecutionGroup()
	{
	}

	void start()
	{
		m_done_count = 0U;
		Synchronizer::lockAtDone(m_workers);
		Synchronizer::unlockAtReady(m_workers);
	}

	void waitExecutionDone()
	{
		waitWorkersDone();
		Synchronizer::lockAtReady(m_workers);
		Synchronizer::unlockAtDone(m_workers);
		m_workers.clear();
	}

private:
	const uint32_t       m_group_size;
	const WorkerFunction m_job;
	std::list<Worker*>   m_workers;

	std::atomic<uint32_t>   m_done_count;
	std::condition_variable m_condition;
	std::mutex              m_condition_mutex;

	void notifyWorkerDone()
	{
		{
			std::lock_guard<std::mutex> lg(m_condition_mutex);
			++m_done_count;
		}
		m_condition.notify_one();
	}

	void waitWorkersDone()
	{
		std::unique_lock<std::mutex> ul(m_condition_mutex);
		m_condition.wait(ul, [this] { return m_done_count == m_group_size; });
	}

	void retrieveWorkers(std::list<Worker*>& available_workers)
	{
		for (uint32_t i(m_group_size); i--;) {
			Worker* worker = available_workers.front();
			available_workers.pop_front();
			worker->setJob(i, this);
			m_workers.push_back(worker);
		}
	}

	friend Worker;
};

class WorkGroup
{
public:
	WorkGroup()
		: m_group(nullptr)
	{}

	WorkGroup(std::shared_ptr<ExecutionGroup> execution_group)
		: m_group(execution_group)
	{}

	void waitExecutionDone()
	{
		if (m_group) {
			m_group->waitExecutionDone();
		}
	}

private:
	std::shared_ptr<ExecutionGroup> m_group;
};

class Swarm
{
public:
	Swarm(uint32_t thread_count)
		: m_thread_count(thread_count)
		, m_ready_count(0U)
	{
		for (uint32_t i(thread_count); i--;) {
			createWorker();
		}

		while (m_ready_count < m_thread_count) {}
	}

	~Swarm()
	{
		Synchronizer::stop(m_workers);
		Synchronizer::unlockAtReady(m_workers);
		Synchronizer::join(m_workers);
		deleteWorkers();
	}

	WorkGroup execute(WorkerFunction job, uint32_t group_size = 0)
	{
		if (!group_size) {
			group_size = m_thread_count;
		}

		if (group_size > m_available_workers.size()) {
			return WorkGroup();
		}

		return WorkGroup(std::make_unique<ExecutionGroup>(job, group_size, m_available_workers));
	}


private:
	const uint32_t m_thread_count;

	std::atomic<uint32_t> m_ready_count;
	std::list<Worker*>  m_workers;
	std::list<Worker*>  m_available_workers;
	std::mutex m_mutex;

	void createWorker()
	{
		Worker* new_worker = new Worker(this);
		new_worker->createThread();
		m_workers.push_back(new_worker);
	}

	void deleteWorkers()
	{
		for (Worker* worker : m_workers) {
			delete worker;
		}
	}

	void notifyWorkerReady(Worker* worker)
	{
		std::lock_guard<std::mutex> lg(m_mutex);
		++m_ready_count;
		m_available_workers.push_back(worker);
	}

	friend Worker;
};

Worker::Worker(Swarm* swarm)
	: m_swarm(swarm)
	, m_group(nullptr)
	, m_id(0)
	, m_group_size(0)
	, m_running(true)
	, m_ready_mutex()
	, m_done_mutex()
{
}

void Worker::createThread()
{
	lockReady();
	m_thread = std::thread(&Worker::run, this);
}

void Worker::run()
{
	while (true) {
		waitReady();

		if (!m_running) {
			break;
		}

		m_job(m_id, m_group_size);

		waitDone();
	}
}

void Worker::lockReady()
{
	m_ready_mutex.lock();
}

void Worker::unlockReady()
{
	m_ready_mutex.unlock();
}

void Worker::lockDone()
{
	m_done_mutex.lock();
}

void Worker::unlockDone()
{
	m_done_mutex.unlock();
}

void Worker::setJob(uint32_t id, ExecutionGroup* group)
{
	m_id = id;
	m_job = group->m_job;
	m_group_size = group->m_group_size;
	m_group = group;
}

void Worker::stop()
{
	m_running = false;
}

void Worker::join()
{
	m_thread.join();
}

void Worker::waitReady()
{
	m_swarm->notifyWorkerReady(this);
	lockReady();
	unlockReady();
}

void Worker::waitDone()
{
	m_group->notifyWorkerDone();
	lockDone();
	unlockDone();
}

}
