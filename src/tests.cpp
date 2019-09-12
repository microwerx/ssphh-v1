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
#include <iostream>
#include <string>
#include <damselfish_json.hpp>
#include <hatchetfish.hpp>
#include <unicornfish_curl.hpp>
#include <fluxions_gte.hpp>


template <typename T = int> class Outer
{
public:
	struct Inner
	{
		float t = 0.0f;
	};

	Outer() {}
};


void test_nested_template_structs()
{
	//Outer<float> o;
	//Outer::Inner i;
	//i.t = 1.0;
	//cout << i.t;
}


void test_json()
{
	std::string test_json = "{\"projects\":[{\"id\":\"1\", \"name\" : \"Outside scene\", \"description\" : \"A scene with a floor, teapot, bunny, and dragon.\"}, { \"id\":\"2\",\"name\" : \"test1\",\"description\" : \"this is a description for test1\" }, { \"id\":\"3\",\"name\" : \"test2\",\"description\" : \"this is a description for test2\" }, { \"id\":\"4\",\"name\" : \"a project\",\"description\" : \"a description about a project\" }, { \"id\":\"32\",\"name\" : \"test\",\"description\" : \"test description\" }, { \"id\":\"114\",\"name\" : \"test8141\",\"description\" : \"this is a description for test1\" }, { \"id\":\"119\",\"name\" : \"project 1\",\"description\" : \"project 1 description\" }, { \"id\":\"120\",\"name\" : \"project 2\",\"description\" : \"project 2 description\" }, { \"id\":\"121\",\"name\" : \"project 3\",\"description\" : \"project 3 description\" }, { \"id\":\"122\",\"name\" : \"project 4\",\"description\" : \"project 4 description\" }, { \"id\":\"123\",\"name\" : \"project 5\",\"description\" : \"project 5 description\" }]}";

	Df::JSONPtr json = Df::JSON::New();
	if (json) {
		bool result = json->Deserialize(test_json);
		std::cout << __FUNCTION__ << " Deserialize() result was successful\n";
		std::string str = json->Serialize();
		std::cout << __FUNCTION__ << "\n";
		std::cout << str << "\n";
		Df::JSONPtr json2 = Df::JSON::New();
		json2->Deserialize(str);
		result = false;
		if (json == json2) {
			json->Clear();
			if (json != json2) {
				result = true;
			}
		}
		if (result) {
			std::cout << __FUNCTION__ << " == works!\n";
		}
		else {
			std::cout << __FUNCTION__ << " == does not work!\n";
		}
	}
	std::cout << __FUNCTION__ << " test is finished\n";
}


int test_curl()
{
	using namespace Uf;
	Curl curl;
	double t0 = Hf::Log.getSecondsElapsed();
	std::string result = curl.Get("http://phd.metzgar-research.com/test_get_project.php");
	double dt = Hf::Log.getSecondsElapsed() - t0;
	Hf::Log.info("%s(): curl.GET worked! took %3.6f", __FUNCTION__, dt);

	Curl::StringTimePairFuture p;
	Curl::StringTimePair results;

	t0 = Hf::Log.getSecondsElapsed();
	p = curl.AsyncGet("http://phd.metzgar-research.com/test_get_project.php");
	dt = Hf::Log.getSecondsElapsed() - t0;
	Hf::Log.info("%s(): curl.AsyncGET finished! took %3.6f", __FUNCTION__, dt);

	results = p.get();
	Hf::Log.info("%s(): curl.AsyncGET return result: %s", __FUNCTION__, results.first.c_str());
	Hf::Log.info("%s(): curl.AsyncGET worked! took %3.6f", __FUNCTION__, results.second);

	t0 = Hf::Log.getSecondsElapsed();
	p = curl.AsyncGet("http://192.168.1.164/");
	dt = Hf::Log.getSecondsElapsed() - t0;
	Hf::Log.info("%s(): curl.AsyncGET finished! took %3.6f", __FUNCTION__, dt);

	results = p.get();
	Hf::Log.info("%s(): curl.AsyncGET return result: %s", __FUNCTION__, results.first.c_str());
	Hf::Log.info("%s(): curl.AsyncGET worked! took %3.6f", __FUNCTION__, results.second);
	return 0;
}


//template <typename T, typename U>
//constexpr auto test_add(T a, U b) noexcept
//{
//	return a + b;
//}
//
//
//template <typename T, typename U>
//constexpr auto test_multiply(T a, U b) noexcept
//{
//	return a * b;
//}
//
//
//using namespace Fluxions;
//
//
//void test_linear_algebra()
//{
//	Matrix2f m2;
//
//	float a = 1.0;
//	
//	auto m2_add_result_ab = test_add<Matrix2f, float>(m2, a);
//	auto m2_add_result_ba = test_add<float, Matrix2f>(a, m2);
//}


void test_spherical_coordinate_conversions()
{
	using namespace Fluxions;

	Vector3f v1;
	Vector3f v2;

	for (int i = 0; i < 100; i++) {
		v1.reset(randomSampler(-100.0f, 100.0f), randomSampler(-100.0f, 100.0f), randomSampler(-100.0f, 100.0f));
		v1.normalize();
		float theta = v1.theta();
		float phi = v1.phi();
		v2.from_theta_phi(theta, phi);
		float epsilon = (v1 - v2).length();
		std::cout << "normal epsilon: " << epsilon << std::endl;

		theta = v1.to_math_theta();
		phi = v1.to_math_phi();
		v2.from_math_theta_phi(theta, phi);
		epsilon = (v1 - v2).length();
		std::cout << "math epsilon: " << epsilon << std::endl;

		theta = v1.to_physics_theta();
		phi = v1.to_physics_phi();
		v2.from_physics_theta_phi(theta, phi);
		epsilon = (v1 - v2).length();
		std::cout << "physics epsilon: " << epsilon << std::endl;
	}
}


void do_tests()
{
	//test_spherical_coordinate_conversions();
	//exit(0);
	//test_linear_algebra();
	// test_json();
	// test_curl();
	// exit(0);
}
