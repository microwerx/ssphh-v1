#ifndef SPHL_JOB_HPP
#define SPHL_JOB_HPP

#include <fluxions_gte_spherical_harmonic.hpp>
#include <damselfish_json.hpp>

class SphlJob
{
public:
	SphlJob();
	~SphlJob();

	std::string toJSON() noexcept;
	//Fluxions::CoronaJob &toCoronaJob() noexcept;
	bool parseJSON(const std::string &str) noexcept;
	void resizeCoefs(int maxDegree) noexcept;

	std::string json;
	Df::JSONPtr jsonObject;

	int numChannels;
	int maxDegree;
	Fluxions::Sph4f sphl;
	std::vector<std::vector<float>> coefs;
	Fluxions::Vector3f meta_position;
	std::string meta_scene;
	std::string meta_time;
	std::string meta_coronaJob;
	int meta_sphlIndex;
};

#endif