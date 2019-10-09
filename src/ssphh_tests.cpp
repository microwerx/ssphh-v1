#include "pch.hpp"
#ifdef _WIN32
#include <direct.h>		// for _mkdir
#endif
#include <ssphhapp.hpp>

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
#ifdef _WIN32
			_mkdir("output");
#else
			mkdir("output", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
		}
		fpi.Set("output");
		if (!fpi.IsDirectory()) {
			Hf::Log.errorfn(__FUNCTION__, "Path 'output' is not a directory");
			return;
		}
		test = 0;
		size_t count = 0;
		for (auto &sphl : ssgUserData->ssphhLights) {
			std::ostringstream ostr;
			ostr << ssg.name << "_sphl" << count;
			if (!sphl.SaveOBJ("output", ostr.str())) {
				Hf::Log.warningfn(__FUNCTION__, "sphl.SaveOBJ() failed to save %s", ostr.str().c_str());
				break;
			}
			count++;
		}

		if (count != ssgUserData->ssphhLights.size()) {
			Hf::Log.errorfn(__FUNCTION__, "Did not save all the SPHLs as OBJ/MTL files");
			test = -1;
		}
		else {
			test = 1;
		}
	}
}
