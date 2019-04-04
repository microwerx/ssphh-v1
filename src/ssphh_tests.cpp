#include "stdafx.h"
#include <ssphh.hpp>

namespace SSPHH
{
	void SSPHH_Application::Test()
	{
		TestSPHLs();
	}

	void SSPHH_Application::TestSPHLs()
	{
		int &test = Interface.tests.saveSphlOBJ;
		test = 0;
		int count = 0;
		for (auto &sphl : ssg.ssphhLights) {
			ostringstream ostr;
			ostr << "tests/sphl" << count << ".obj";
			//sphl.SaveOBJ(ostr.str());
		}
	}
}