#ifndef FLUXIONS_SIMPLE_SSPHH_HPP
#define FLUXIONS_SIMPLE_SSPHH_HPP

#include <string>
#include <vector>
#include <fluxions_gte_spherical_harmonic.hpp>
#include <fluxions_simple_ssphh_light.hpp>

namespace Fluxions
{
	class SimpleSceneGraph;

	class SimpleSSPHH
	{
	public:
		SimpleSSPHH();
		~SimpleSSPHH();

		void INIT(SimpleSceneGraph& ssg);
		void GEN();
		void VIZ();
		void HIER(bool includeSelf = true, bool includeNeighbor = true, int maxDegrees = -1);

		bool saveJSONs = false;
		bool savePPMs = false;
		bool convToPNG = false;

		float VIZmix = 1.0;

	private:
		/// A pair (int index, float p)
		struct Qpair
		{
			int index = -1;
			float p = 0.0f;
		};

		std::string sceneName;
		std::vector<SimpleSSPHHLight>* sphls_ = nullptr;
		size_t size_ = 0; // The number of SPHLs

		// GEN creates this light probe
		std::vector<Sph4f> S;
		// VIZ generates these visibility spherical harmonics. Index i is S
		std::vector<std::vector<Sph4f>> H;
		// VIZ generates visibility probability for each H.
		std::vector<std::vector<float>> P;
		// HIER creates Q from index and P;
		std::vector<Qpair> Q;
		// HIER sorts Q
		std::vector<Qpair> Qsorted;
		// HIER generates this finalized light probe
		std::vector<Sph4f> Sprime;

		std::vector<Sph4f> self;
		std::vector<Sph4f> neighbor;
	};
}

#endif