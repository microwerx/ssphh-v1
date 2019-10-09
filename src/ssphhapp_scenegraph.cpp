#include <ssphhapp.hpp>
#include <hatchetfish_stopwatch.hpp>

namespace SSPHH
{
	void SSPHH_Application::ReloadScenegraph() {
		ssg.Reset();
		Hf::StopWatch stopwatch;
		LoadScene();
		stopwatch.Stop();
		Hf::Log.info("%s(): SSG reload took %4.2f milliseconds", __FUNCTION__, stopwatch.GetMillisecondsElapsed());
		Interface.lastScenegraphLoadTime = stopwatch.GetMillisecondsElapsed();
	}
}