#include "pch.hpp"
#include <SphlJob.hpp>


Df::JSONPtr NewVector3(const Fluxions::Vector3f & v)
{
	Df::JSONPtr p = Df::JSON::NewObject();
	p->set({
		{ "x", Df::JSON::NewNumber(v.x) },
		{ "y", Df::JSON::NewNumber(v.y) },
		{ "z", Df::JSON::NewNumber(v.z) }
		});
	return p;
}

Df::JSONPtr NewMatrix(std::vector<std::vector<float>> & M)
{
	Df::JSONPtr m = Df::JSON::NewArray();
	for (std::vector<float> & row : M) {
		Df::JSONPtr rowElements = Df::JSON::NewArray();
		for (const float & col : row) {
			Df::JSONPtr el = Df::JSON::NewNumber(col);
			rowElements->PushBack(el);
		}
		m->PushBack(rowElements);
	}
	return m;
}

SphlJob::SphlJob()
{
	jsonObject = Df::JSON::MakeObject();
	resizeCoefs(0);
}


SphlJob::~SphlJob()
{
	jsonObject = nullptr;
}


std::string SphlJob::toJSON() noexcept
{
	auto j_meta = Df::JSON::NewObject();
	j_meta->set({
		{ "position",  NewVector3(meta_position) },
		{ "scene",     Df::JSON::NewString(meta_scene) },
		{ "time",      Df::JSON::NewString(Hf::Log.makeDTG()) },
		{ "sphlIndex", Df::JSON::NewNumber(meta_sphlIndex) },
		{ "coronaJob",   Df::JSON::NewString(meta_coronaJob) }
		});

	jsonObject = Df::JSON::NewObject();
	jsonObject->set({
		{ "numChannels", Df::JSON::NewNumber(numChannels) },
		{ "maxDegree",   Df::JSON::NewNumber(maxDegree) },
		{ "coefs",       NewMatrix(coefs) },
		{ "meta",        j_meta }
		});
	json = jsonObject->Serialize();
	return json;
}


//Fluxions::CoronaJob &SphlJob::toCoronaJob() noexcept
//{
//	return coronaJob;
//}


bool SphlJob::parseJSON(const std::string &str) noexcept
{
	// In this method j_variableName means a member of the JSON object

	bool foundError = false;
	jsonObject->Deserialize(str);
	if (!jsonObject->HasKeyOfType("numChannels", Df::JSON::Type::Number)) return false;
	if (!jsonObject->HasKeyOfType("maxDegree", Df::JSON::Type::Number)) return false;
	if (!jsonObject->HasKeyOfType("coefs", Df::JSON::Type::Array)) return false;
	if (!jsonObject->HasKeyOfType("meta", Df::JSON::Type::Object)) return false;

	numChannels = jsonObject->getMember("numChannels")->AsInt();
	maxDegree = jsonObject->getMember("maxDegree")->AsInt();
	sphl.resize(maxDegree);
	size_t numCoefs = sphl.size();

	// Copy 'coefs'
	std::vector<Df::JSONPtr> j_coefs = jsonObject->getMember("coefs")->AsArray();
	if (j_coefs.size() != sphl.size()) {
		HFLOGERROR("JSON has incorrect number of SPH degrees %i %i",
			(int)j_coefs.size(), (int)sphl.size());
	}
	coefs.resize(numCoefs);
	for (int i = 0; i < numCoefs; i++) {
		coefs[i] = j_coefs[i]->AsFloatArray();
		if (coefs[i].size() != numChannels) {
			HFLOGERROR("JSON has incorrect number of SPH channels");
			foundError = true;
			coefs[i].resize(numChannels, 0.0f);
		}
	}

	// Copy meta data
	auto j_meta = jsonObject->getMember("meta");
	if (j_meta) {
		auto j_position = j_meta->getMember("position");
		if (j_position) {
			j_position->getMemberAsFloat("x", meta_position.x);
			j_position->getMemberAsFloat("y", meta_position.y);
			j_position->getMemberAsFloat("z", meta_position.z);
		}
		j_meta->getMemberAsString("scene", meta_scene);
		j_meta->getMemberAsString("time", meta_time);
		j_meta->getMemberAsInt("sphlIndex", meta_sphlIndex);
		j_meta->getMemberAsString("coronaJob", meta_coronaJob);
	}
	return true;
}

void SphlJob::resizeCoefs(int sphMaxDegree) noexcept
{
	maxDegree = sphMaxDegree;
	sphl.resize(maxDegree);
	sphl.toVectorFormat(numChannels, coefs);
}