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
#include <ssphh_unicornfish.hpp>


void DoBroker(const char * endpoint, Unicornfish * context)
{
	if (!context) return;
	Uf::Broker broker;
	context->SetMessage(Unicornfish::NodeType::Broker, "started");
	bool result = broker.Create(endpoint);
	while (result && !context->IsStopped())
	{
		result = broker.RunLoop();
		ostringstream ostr;
		ostr << "Workers Total/Waiting/Requests: ";
		ostr << broker.GetNumWorkers() << "/";
		ostr << broker.GetNumWaitingWorkers() << "/";
		ostr << broker.GetNumRequests();
		context->SetMessage(Unicornfish::NodeType::Broker, ostr.str());
	}
	if (!result)
	{
		HFLOGINFO("%s(): broker: error!");
	}
	broker.Delete();
	HFLOGINFO("broker: okay, quitting");
	context->SetMessage(Unicornfish::NodeType::Broker, "stopped");
}
