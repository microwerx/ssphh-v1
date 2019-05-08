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
#include "stdafx.h"
#include <ssphh.hpp>
#include <unicornfish.hpp>


using namespace Fluxions;

extern shared_ptr<SSPHH::SSPHH_Application> ssphhPtr;
Unicornfish ssphhUf;

void Unicornfish::StartClient(const string & endpoint)
{
	client_thread = thread(DoClient, endpoint.c_str(), this);
}

void Unicornfish::StartWorker(const string & endpoint, const string & service)
{
	worker_thread = thread(DoWorker, endpoint.c_str(), service.c_str(), this);
}

void Unicornfish::StartBroker()
{
	broker_thread = thread(DoBroker, broker_endpoint.c_str(), this);
}

void Unicornfish::StartStandalone(bool client, bool broker, bool worker)
{
	if (!ssphhPtr)
		return;
	if (!stopped) return;
	stopped = false;
	HFLOGINFO("Starting standalone...");
	if (broker)
		broker_thread = thread(DoBroker, "tcp://*:9081", this);
	if (client)
		client_thread = thread(DoClient, "tcp://127.0.0.1:9081", this);
	if (worker)
		worker_thread = thread(DoWorker, "tcp://127.0.0.1:9081", ssphhPtr->GetSceneName().c_str(), this);
}

void Unicornfish::Join()
{
	Stop();
	HFLOGINFO("Stopping, waiting for threads to join...");
	if (client_thread.joinable()) client_thread.join();
	if (worker_thread.joinable()) worker_thread.join();
	if (broker_thread.joinable()) broker_thread.join();
}


int Unicornfish::GetNumScatteredJobs() const
{
	return (int)incoming_jobs.size();
}


int Unicornfish::GetNumFinishedJobs() const
{
	return (int)finished_jobs.size();
}


void Unicornfish::GetFinishedJobs(map<string, Fluxions::CoronaJob> & jobs)
{
	LockWrite();
	jobs = finished_jobs;
	finished_jobs.clear();
	numScattered = 0;
	UnlockWrite();
}


void Unicornfish::ScatterJob(CoronaJob & job)
{
	LockWrite();
	incoming_jobs[job.GetName()] = job;
	UnlockWrite();
}


int Unicornfish::PushScatteredJobs(map<string, Fluxions::CoronaJob> & jobs)
{
	LockWrite();
	int count = (int)incoming_jobs.size();
	numScattered += count;
	for (auto & job : incoming_jobs)
	{
		job.second.MarkJobUnfinished();
		jobs[job.first] = job.second;
	}
	incoming_jobs.clear();
	UnlockWrite();
	return count;
}


void Unicornfish::PullFinishedJobs(map<string, Fluxions::CoronaJob> & jobs)
{
	LockWrite();
	bool finishedJobFound;
	do
	{
		finishedJobFound = false;
		for (auto & job : jobs)
		{
			if (job.second.IsFinished())
			{
				finished_jobs[job.first] = job.second;
				finishedJobFound = true;
				jobs.erase(job.first);
				break;
			}
		}
	} while (finishedJobFound);
	UnlockWrite();
}


void Unicornfish::SetMessage(Unicornfish::NodeType type, const string & message)
{
	LockWrite();
	switch (type)
	{
	case Unicornfish::NodeType::Client:
		client_message = message;
		break;
	case Unicornfish::NodeType::Broker:
		broker_message = message;
		break;
	case Unicornfish::NodeType::Worker:
		worker_message = message;
		break;
	}
	UnlockWrite();
}


const string & Unicornfish::GetMessage(Unicornfish::NodeType type)
{
	string & s = client_message;
	LockRead();
	switch (type)
	{
	case Unicornfish::NodeType::Client:
		s = client_message;
		break;
	case Unicornfish::NodeType::Broker:
		s = broker_message;
		break;
	case Unicornfish::NodeType::Worker:
		s = worker_message;
		break;
	}
	UnlockRead();
	return s;
}


