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
#include "pch.hpp"
#include <ssphhapp.hpp>
#include <ssphh_unicornfish.hpp>
#include <SphlJob.hpp>

namespace Uf
{

	void DoClient(const char * endpoint, Unicornfish * uf)
	{
		if (!uf)
			return;
		if (!ssphhPtr)
			return;
		std::shared_ptr<SSPHH::SSPHH_Application> ssphh = ssphhPtr;

		uf->SetUIMessage(Unicornfish::NodeType::Client, "started");
		Uf::Client client;
		bool result = client.ConnectToBroker(endpoint);
		int numGatheredJobs = 0;
		int numWorkingJobs = 0;
		std::map<std::string, CoronaJob> scatteredJobs;
		std::map<std::string, int64_t> sent_times;
		while (result && !uf->IsStopped()) {
			// Find out if we have scattered jobs to send out
			int numScatteredJobs = uf->PushScatteredJobs(scatteredJobs);

			auto cur_time = zclock_mono();

			// Send out scattered jobs
			if (numScatteredJobs > 0) {
				for (auto & job : scatteredJobs) {
					int64_t dt = cur_time - sent_times[job.first];
					if (!job.second.IsJobWorking() && dt >= 25000) {
						Uf::CoronaJob &coronaJob = job.second;
						SphlJob sphlJob;
						sphlJob.numChannels = 4;
						sphlJob.resizeCoefs(5);
						sphlJob.meta_position.reset(0.0f, 0.0f, 0.0f);
						sphlJob.meta_scene = coronaJob.GetName();
						sphlJob.meta_time = Hf::Log.makeDTG();
						sphlJob.meta_coronaJob = coronaJob.ToString();

						// The stack is (from bottom)
						// coronaJob: CoronaJob
						// sphlJob: SphlJob
						// sceneName: string
						// service: string [groundtruth, sph, irprobe]
						Uf::Message request(&job.second, sizeof(Uf::CoronaJob));
						request.Push(sphlJob.toJSON());
						request.Push(job.first);
						sent_times[job.first] = cur_time;
						client.SendRequest(ssphh->GetSceneName().c_str(), request);
					}
				}
			}

			// Check for replies
			while (client.PollReply()) {
				if (client.WaitReply()) {
					Uf::Message reply = client.GetReply();
					// 1. POP SERVICE STRING
					std::string jobName = reply.PopString();
					// 2. POP STATUS
					std::string status = reply.PopString();
					if (status == "finished") {
						// this is where we would "pop" the results

						// 3. POP JSON SPHL JOB
						std::string jsonSphlJob = reply.PopString();

						// 4. POP CORONA JOB
						CoronaJob job;
						reply.PopMem(&job, sizeof(CoronaJob));
						scatteredJobs[jobName] = job;
						memset(&job, 0, sizeof(CoronaJob));


						//scatteredJobs[jobName].MarkJobFinished();
					}
					if (status == "working") {
						if (scatteredJobs.find(jobName) != scatteredJobs.end())
							scatteredJobs[jobName].MarkJobWorking();
					}
				}
			}

			numWorkingJobs = 0;
			for (auto & sj : scatteredJobs) {
				if (sj.second.IsJobFinished())
					numGatheredJobs++;
				if (sj.second.IsJobWorking())
					numWorkingJobs++;
			}

			uf->PullFinishedJobs(scatteredJobs);

			std::ostringstream ostr;
			ostr << "client: scattered/working/finished: ";
			ostr << numScatteredJobs << "/";
			ostr << numWorkingJobs << "/";
			ostr << numGatheredJobs;
			uf->SetUIMessage(Unicornfish::NodeType::Client, ostr.str());
		}
		if (!result) {
			HFLOGERROR("client: error!");
		}
		HFLOGINFO("client: okay, quitting");
		client.Disconnect();
		uf->SetUIMessage(Unicornfish::NodeType::Client, "stopped");
	}
}