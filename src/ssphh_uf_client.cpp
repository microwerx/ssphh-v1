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
#include "ssphh_unicornfish.hpp"


//SSPHHClient::SSPHHClient()
//{
//
//}
//
//
//SSPHHClient::~SSPHHClient()
//{
//
//}
//
//
//void SSPHHClient::Connect(UfStringQueue *readFromQueue, UfStringQueue *writeToQueue, const char *endpoint)
//{
//	if (readFromQueue == nullptr || writeToQueue == nullptr || endpoint == nullptr) return;
//	this->readFromQueue = readFromQueue;
//	this->writeToQueue = writeToQueue;
//	this->client.ConnectToBroker(endpoint);
//}
//
//
//void SSPHHClient::Send(const string &msg)
//{
//	if (writeToQueue)
//	{
//		writeToQueue->Push(msg);
//	}
//}
//
//
//const string &SSPHHClient::Recv()
//{
//	if (readFromQueue)
//	{
//		readFromQueue->Pop(recvString);
//		return recvString;
//	}
//	recvString.clear();
//	return recvString;
//}
//
//
//size_t SSPHHClient::MessagesAvailable() const
//{
//	if (readFromQueue)
//	{
//		return readFromQueue->Size();
//	}
//	return 0;
//}
