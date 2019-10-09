// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
// Copyright (C) 2017-2019 Jonathan Metzgar
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
#include "pch.hpp"
#include <ssphhapp.hpp>
#include <ssphh_unicornfish.hpp>
#include <SphlJob.hpp>

namespace Uf
{
	constexpr size_t CoronaJobSize = sizeof(Uf::CoronaJob);

	void DoOldWorker(const char * endpoint, const char * service, Unicornfish * context)
	{
		if (!context)
			return;
		context->SetUIMessage(Unicornfish::NodeType::Worker, "started");
		Uf::Worker worker;
		bool result = worker.ConnectToBroker(endpoint, service);
		while (result && !context->IsStopped()) {
			Uf::Message reply;
			if (worker.WaitRequest()) {
				Uf::Message request = worker.GetRequest();
				std::string jobName = request.PopString();
				Uf::CoronaJob job;
				request.PopMem(&job, CoronaJobSize);
				auto & frame = request.PopFrame();
				memcpy(&job, frame.GetData(), frame.SizeInBytes());

				reply = request;
				reply.Push("working");
				reply.Push(jobName);
				worker.SendReply(reply);

				std::string messageStr = "working ";
				messageStr += jobName;
				context->SetUIMessage(Unicornfish::NodeType::Worker, messageStr);

				// do the job!
				ssphhPtr->RunJob(job);
				job.MarkJobFinished();

				reply = request;
				reply.Push(&job, CoronaJobSize);
				reply.Push("finished");
				reply.Push(jobName);
				worker.SendReply(reply);
				memset(&job, 0, CoronaJobSize);

				context->SetUIMessage(Unicornfish::NodeType::Worker, "waiting");
			}
		}
		worker.Disconnect();
		HFLOGINFO("worker: okay, quitting -- was doing \"%s\"", service);
		context->SetUIMessage(Unicornfish::NodeType::Worker, "stopped");
	}

	void DoWorker(const char * endpoint, const char *service, Unicornfish *context)
	{
		if (!context)
			return;
		context->SetUIMessage(Unicornfish::NodeType::Worker, "started");
		Uf::Worker worker;
		bool result = worker.ConnectToBroker(endpoint, service);
		while (result && !context->IsStopped()) {
			Uf::Message reply;
			if (worker.WaitRequest()) {
				Uf::Message request = worker.GetRequest();
				std::string jobName = request.PopString();
				std::string jsonSphlJob = request.PopString();
				SphlJob sphlJob;
				sphlJob.parseJSON(jsonSphlJob);
				Uf::CoronaJob job;
				job.FromString(sphlJob.meta_coronaJob);
				Uf::CoronaJob job1;
				request.PopMem(&job1, CoronaJobSize);
				auto & frame = request.PopFrame();
				memcpy(&job, frame.GetData(), frame.SizeInBytes());

				reply = request;
				reply.Push("working");
				reply.Push(jobName);
				worker.SendReply(reply);

				std::string messageStr = "working ";
				messageStr += jobName;
				context->SetUIMessage(Unicornfish::NodeType::Worker, messageStr);

				// do the job!
				ssphhPtr->RunJob(job);
				job.MarkJobFinished();

				// report the results
				sphlJob.meta_coronaJob = job.ToString();
				job.CopySPHToSph4f(sphlJob.sphl);

				// The Stack is (from top)
				// frame
				// jobName: string
				// status: string
				// sphlJob: JSON string
				// coronaJob: CoronaJob
				reply = request;
				reply.Push(&job, CoronaJobSize);
				reply.Push(sphlJob.toJSON());
				reply.Push("finished");
				reply.Push(jobName);
				worker.SendReply(reply);
				memset(&job, 0, CoronaJobSize);

				context->SetUIMessage(Unicornfish::NodeType::Worker, "waiting");
			}
		}
		worker.Disconnect();
		HFLOGINFO("worker: okay, quitting -- was doing \"%s\"", service);
		context->SetUIMessage(Unicornfish::NodeType::Worker, "stopped");
	}
}