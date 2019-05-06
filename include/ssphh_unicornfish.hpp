// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/KASL/GLUT Extensions
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
#include <fluxions_corona_job.hpp>
#include <fluxions_corona_scene_file.hpp>

using namespace std;

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
enum class SUFType
{
	Client,
	Worker,
	Broker
};

class SSPHHUnicornfish
{
public:
	SSPHHUnicornfish() {}

	const int port = 9081;
	string broker_endpoint = "tcp://*:9081";
	string worker_endpoint = "tcp://127.0.0.1:9081";
	string client_endpoint = "tcp://127.0.0.1:9081";

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

	void StartClient(const string &endpoint);
	void StartWorker(const string &endpoint, const string &service);
	void StartBroker();
	void StartStandalone(bool client = true, bool broker = true, bool worker = true);
	void Join();
	void Stop() { stopped = true; }

	void SetMessage(SUFType type, const string &message);
	const string &GetMessage(SUFType type);

	void ScatterJob(Fluxions::CoronaJob &job);
	// Move the queue of scattered jobs out. CoronaJob::IsFinished() returns false
	int PushScatteredJobs(map<string, Fluxions::CoronaJob> &jobs);
	// Move the queue of gathered jobs back. CoronaJob::IsFinished() returns true
	void PullFinishedJobs(map<string, Fluxions::CoronaJob> &jobs);
	int GetNumScatteredJobs() const;
	int GetNumFinishedJobs() const;

	void GetFinishedJobs(map<string, Fluxions::CoronaJob> &finished_jobs);

private:
	mutex uf_mutex;
	mutex uf_read_mutex;
	mutex uf_write_mutex;

	bool stopped = true;

	thread broker_thread;
	thread worker_thread;
	thread client_thread;

	int numScattered = 0;
	map<string, Fluxions::CoronaJob> incoming_jobs;
	map<string, Fluxions::CoronaJob> finished_jobs;

	string broker_message;
	string worker_message;
	string client_message;
};

extern SSPHHUnicornfish ssphhUf;

#endif