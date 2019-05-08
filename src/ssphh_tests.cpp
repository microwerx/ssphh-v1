#include "pch.h"
#include <ssphh.hpp>

namespace SSPHH
{
	void SSPHH_Application::Test()
	{
		if (Interface.tests.bTestSPHLs) {
			TestSPHLs();
			Interface.tests.bTestSPHLs = false;
		}
	}

	void SSPHH_Application::TestSPHLs()
	{
		int &test = Interface.tests.saveSphlOBJ;
		FilePathInfo fpi("output");
		if (fpi.DoesNotExist()) {
			mkdir("output");
		}
		fpi.Set("output");
		if (!fpi.IsDirectory()) {
			hflog.errorfn(__FUNCTION__, "Path 'output' is not a directory");
			return;
		}
		test = 0;
		size_t count = 0;
		for (auto &sphl : ssg.ssphhLights) {
			std::ostringstream ostr;
			ostr << ssg.name << "_sphl" << count;
			if (!sphl.SaveOBJ("output", ostr.str())) {
				hflog.warningfn(__FUNCTION__, "sphl.SaveOBJ() failed to save %s", ostr.str().c_str());
				break;
			}
			count++;
		}

		if (count != ssg.ssphhLights.size()) {
			hflog.errorfn(__FUNCTION__, "Did not save all the SPHLs as OBJ/MTL files");
			test = -1;
		}
		else {
			test = 1;
		}
	}
}
