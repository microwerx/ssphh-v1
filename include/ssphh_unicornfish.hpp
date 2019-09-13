// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
// Copyright (C) 2017 Jonathan Metzgar
// All rights reserved.
//
// This program is free software : you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.If not, see <https://www.gnu.org/licenses/>.
//
// For any other type of licensing, please contact me at jmetzgar@outlook.com
#ifndef SSPHH_UNICORNFISH_HPP
#define SSPHH_UNICORNFISH_HPP

#include <mutex>
#include <unicornfish.hpp>
#include <unicornfish_broker.hpp>
#include <unicornfish_client.hpp>
#include <unicornfish_worker.hpp>
#include <unicornfish_corona_job.hpp>
#include <unicornfish_corona_scene_file.hpp>

enum class UfType
{
	None,
	Broker,
	ClientGLES20,
	Client,
	WorkerEcho,
	Worker
};

//class SSPHHWorker : public Uf::Worker
//{
//public:
//	SSPHHWorker() : Uf::Worker() { }
//	SSPHHWorker(const char *endpoint, const char *service) : Uf::Worker(endpoint, service) { }
//
//	void OnProcessRequest(const Uf::Message &request, Uf::Message &reply);
//};
//
//class UfStringQueue
//{
//public:
//	UfStringQueue();
//	~UfStringQueue();
//
//	void Push(const string &msg);
//	void Pop(string &msg);
//	size_t Size() const;
//private:
//	vector<string> messages;
//	mutable mutex messagesMutex;
//};
//
//UfStringQueue::UfStringQueue()
//{
//
//}
//
//UfStringQueue::~UfStringQueue()
//{
//
//}
//
//void UfStringQueue::Push(const string &msg)
//{
//	lock_guard<mutex> guard(messagesMutex);
//	messages.push_back(msg);
//}
//
//void UfStringQueue::Pop(string &msg)
//{
//	lock_guard<mutex> guard(messagesMutex);
//	msg = messages.back();
//	messages.pop_back();
//}
//
//size_t UfStringQueue::Size() const
//{
//	lock_guard<mutex> guard(messagesMutex);
//	return messages.size();
//}
//
//class SSPHHClient
//{
//public:
//	SSPHHClient();
//	~SSPHHClient();
//
//	void Connect(UfStringQueue *readFromQueue, UfStringQueue *writeToQueue, const char *endpoint);
//	void Send(const string &msg);
//	size_t MessagesAvailable() const;
//	const string &Recv();
//private:
//	Uf::Client client;
//	string recvString;
//	UfStringQueue *readFromQueue = nullptr;
//	UfStringQueue *writeToQueue = nullptr;
//};

// SUF = SSPHH Unicornfish

class Unicornfish
{
public:
	enum class NodeType
	{
		Client,
		Worker,
		Broker
	};

	Unicornfish() {}

	const int port = 9081;
	std::string broker_endpoint = "tcp://*:9081";
	std::string worker_endpoint = "tcp://127.0.0.1:9081";
	std::string client_endpoint = "tcp://127.0.0.1:9081";

	void Lock() { uf_mutex.lock(); }
	bool TryLock() { return uf_mutex.try_lock(); }
	void Unlock() { uf_mutex.unlock(); }

	void LockRead() { Lock(); }
	bool TryLockRead() { return TryLock(); }
	void UnlockRead() { Unlock(); }
	void LockWrite() { Lock(); }
	bool TryLockWrite() { return TryLock(); }
	void UnlockWrite() { Unlock(); }

	//void LockRead() { uf_read_mutex.lock(); }
	//bool TryLockRead() { return uf_read_mutex.try_lock(); }
	//void UnlockRead() { uf_read_mutex.unlock(); }
	//void LockWrite() { uf_write_mutex.lock(); }
	//bool TryLockWrite() { return uf_write_mutex.try_lock(); }
	//void UnlockWrite() { uf_write_mutex.unlock(); }

	bool IsStopped() const { return stopped; }

	void StartClient(const std::string &endpoint);
	void StartWorker(const std::string &endpoint, const std::string &service);
	void StartBroker();
	void StartStandalone(bool client = true, bool broker = true, bool worker = true);
	void Join();
	void Stop() { stopped = true; }

	void SetUIMessage(Unicornfish::NodeType type, const std::string &message);
	const std::string &GetUIMessage(Unicornfish::NodeType type);

	void ScatterJob(Uf::CoronaJob &job);
	// Move the queue of scattered jobs out. Uf::CoronaJob::IsFinished() returns false
	int PushScatteredJobs(std::map<std::string, Uf::CoronaJob> &jobs);
	// Move the queue of gathered jobs back. Uf::CoronaJob::IsFinished() returns true
	void PullFinishedJobs(std::map<std::string, Uf::CoronaJob> &jobs);
	int GetNumScatteredJobs() const;
	int GetNumFinishedJobs() const;

	void GetFinishedJobs(std::map<std::string, Uf::CoronaJob> &finished_jobs);

private:
	std::mutex uf_mutex;
	std::mutex uf_read_mutex;
	std::mutex uf_write_mutex;

	bool stopped = true;

	std::thread broker_thread;
	std::thread worker_thread;
	std::thread client_thread;

	int numScattered = 0;
	std::map<std::string, Uf::CoronaJob> incoming_jobs;
	std::map<std::string, Uf::CoronaJob> finished_jobs;

	std::string broker_message;
	std::string worker_message;
	std::string client_message;
};

namespace Uf
{
	void DoClient(const char *endpoint, Unicornfish *context);
	void DoBroker(const char *endpoint, Unicornfish *context);
	void DoWorker(const char *endpoint, const char *service, Unicornfish *context);
	void DoOldWorker(const char *endpoint, const char *service, Unicornfish * context);
}

extern Unicornfish ssphhUf;

#endif