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
#include <ssphh_unicornfish.hpp>


extern shared_ptr<SSPHH_Application> ssphhPtr;
SSPHHUnicornfish ssphhUf;


void DoClient(const char * endpoint, SSPHHUnicornfish * context);
void DoWorker(const char * endpoint, const char * service, SSPHHUnicornfish * context);
void DoBroker(const char * endpoint, SSPHHUnicornfish * context);


void SSPHHUnicornfish::StartClient(const string & endpoint)
{
	client_thread = thread(DoClient, endpoint.c_str(), this);
}

void SSPHHUnicornfish::StartWorker(const string & endpoint, const string & service)
{
	worker_thread = thread(DoWorker, endpoint.c_str(), service.c_str(), this);
}

void SSPHHUnicornfish::StartBroker()
{
	broker_thread = thread(DoBroker, broker_endpoint.c_str(), this);
}

void SSPHHUnicornfish::StartStandalone(bool client, bool broker, bool worker)
{
	if (!ssphhPtr)
		return;
	if (!stopped) return;
	stopped = false;
	hflog.info("%s(): Starting standalone...", __FUNCTION__);
	if (broker)
		broker_thread = thread(DoBroker, "tcp://*:9081", this);
	if (client)
		client_thread = thread(DoClient, "tcp://127.0.0.1:9081", this);
	if (worker)
		worker_thread = thread(DoWorker, "tcp://127.0.0.1:9081", ssphhPtr->GetSceneName().c_str(), this);
}

void SSPHHUnicornfish::Join()
{
	Stop();
	hflog.info("%s(): Stopping, waiting for threads to join...", __FUNCTION__);
	if (client_thread.joinable()) client_thread.join();
	if (worker_thread.joinable()) worker_thread.join();
	if (broker_thread.joinable()) broker_thread.join();
}


int SSPHHUnicornfish::GetNumScatteredJobs() const
{
	return (int)incoming_jobs.size();
}


int SSPHHUnicornfish::GetNumFinishedJobs() const
{
	return (int)finished_jobs.size();
}


void SSPHHUnicornfish::GetFinishedJobs(map<string, Fluxions::CoronaJob> & jobs)
{
	LockWrite();
	jobs = finished_jobs;
	finished_jobs.clear();
	numScattered = 0;
	UnlockWrite();
}


void SSPHHUnicornfish::ScatterJob(CoronaJob & job)
{
	LockWrite();
	incoming_jobs[job.GetName()] = job;
	UnlockWrite();
}


int SSPHHUnicornfish::PushScatteredJobs(map<string, Fluxions::CoronaJob> & jobs)
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


void SSPHHUnicornfish::PullFinishedJobs(map<string, Fluxions::CoronaJob> & jobs)
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


void SSPHHUnicornfish::SetMessage(SUFType type, const string & message)
{
	LockWrite();
	switch (type)
	{
	case SUFType::Client:
		client_message = message;
		break;
	case SUFType::Broker:
		broker_message = message;
		break;
	case SUFType::Worker:
		worker_message = message;
		break;
	}
	UnlockWrite();
}


const string & SSPHHUnicornfish::GetMessage(SUFType type)
{
	string & s = client_message;
	LockRead();
	switch (type)
	{
	case SUFType::Client:
		s = client_message;
		break;
	case SUFType::Broker:
		s = broker_message;
		break;
	case SUFType::Worker:
		s = worker_message;
		break;
	}
	UnlockRead();
	return s;
}


void DoClient(const char * endpoint, SSPHHUnicornfish * context)
{
	if (!context)
		return;
	if (!ssphhPtr)
		return;
	shared_ptr<SSPHH_Application> ssphh = ssphhPtr;

	context->SetMessage(SUFType::Client, "started");
	Uf::Client client;
	bool result = client.ConnectToBroker(endpoint);
	int numGatheredJobs = 0;
	int numWorkingJobs = 0;
	map<string, CoronaJob> scatteredJobs;
	map<string, int64_t> sent_times;
	while (result && !context->IsStopped())
	{
		// Find out if we have scattered jobs to send out
		int numScatteredJobs = context->PushScatteredJobs(scatteredJobs);

		auto cur_time = zclock_mono();

		// Send out scattered jobs
		if (numScatteredJobs > 0)
		{
			for (auto & job : scatteredJobs)
			{
				int64_t dt = cur_time - sent_times[job.first];
				if (!job.second.IsJobWorking() && dt >= 25000)
				{
					Uf::Message request(&job.second, sizeof(CoronaJob));
					request.Push(job.first);
					sent_times[job.first] = cur_time;
					client.SendRequest(ssphh->GetSceneName().c_str(), request);
				}
			}
		}

		// Check for replies
		while (client.PollReply())
		{
			if (client.WaitReply())
			{
				Uf::Message reply = client.GetReply();
				string jobName = reply.PopString();
				string status = reply.PopString();
				if (status == "finished")
				{
					// this is where we would "pop" the results
					CoronaJob job;
					reply.PopMem(&job, sizeof(CoronaJob));
					scatteredJobs[jobName] = job;
					memset(&job, 0, sizeof(CoronaJob));
					//scatteredJobs[jobName].MarkJobFinished();
				}
				if (status == "working")
				{
					if (scatteredJobs.find(jobName) != scatteredJobs.end())
						scatteredJobs[jobName].MarkJobWorking();
				}
			}
		}

		numWorkingJobs = 0;
		for (auto & sj : scatteredJobs)
		{
			if (sj.second.IsJobFinished())
				numGatheredJobs++;
			if (sj.second.IsJobWorking())
				numWorkingJobs++;
		}

		context->PullFinishedJobs(scatteredJobs);

		ostringstream ostr;
		ostr << "client: scattered/working/finished: ";
		ostr << numScatteredJobs << "/";
		ostr << numWorkingJobs << "/";
		ostr << numGatheredJobs;
		context->SetMessage(SUFType::Client, ostr.str());
	}
	if (!result) hflog.error("%s(): client: error!", __FUNCTION__);
	hflog.info("%s(): client: okay, quitting", __FUNCTION__);
	client.Disconnect();
	context->SetMessage(SUFType::Client, "stopped");
}


void DoWorker(const char * endpoint, const char * service, SSPHHUnicornfish * context)
{
	if (!context)
		return;
	context->SetMessage(SUFType::Worker, "started");
	Uf::Worker worker;
	bool result = worker.ConnectToBroker(endpoint, service);
	while (result && !context->IsStopped())
	{		
		Uf::Message reply;
		if (worker.WaitRequest())
		{
			Uf::Message request = worker.GetRequest();
			string jobName = request.PopString();
			CoronaJob job;
			request.PopMem(&job, sizeof(CoronaJob));
			auto & frame = request.PopFrame();
			memcpy(&job, frame.GetData(), frame.SizeInBytes());

			reply = request;
			reply.Push("working");
			reply.Push(jobName);
			worker.SendReply(reply);

			string messageStr = "working ";
			messageStr += jobName;
			context->SetMessage(SUFType::Worker, messageStr);

			// do the job!
			ssphhPtr->RunJob(job);
			job.MarkJobFinished();

			reply = request;
			reply.Push(&job, sizeof(CoronaJob));
			reply.Push("finished");
			reply.Push(jobName);
			worker.SendReply(reply);
			memset(&job, 0, sizeof(CoronaJob));

			context->SetMessage(SUFType::Worker, "waiting");
		}
	}
	worker.Disconnect();
	hflog.info("%s(): worker: okay, quitting -- was doing \"%s\"", __FUNCTION__, service);
	context->SetMessage(SUFType::Worker, "stopped");
}


void DoBroker(const char * endpoint, SSPHHUnicornfish * context)
{
	if (!context) return;
	Uf::Broker broker;
	context->SetMessage(SUFType::Broker, "started");
	bool result = broker.Create(endpoint);
	while (result && !context->IsStopped())
	{
		result = broker.RunLoop();
		ostringstream ostr;
		ostr << "Workers Total/Waiting/Requests: ";
		ostr << broker.GetNumWorkers() << "/";
		ostr << broker.GetNumWaitingWorkers() << "/";
		ostr << broker.GetNumRequests();
		context->SetMessage(SUFType::Broker, ostr.str());
	}
	if (!result) hflog.info("%s(): broker: error!", __FUNCTION__);
	broker.Delete();
	hflog.info("%s(): broker: okay, quitting", __FUNCTION__);
	context->SetMessage(SUFType::Broker, "stopped");
}
