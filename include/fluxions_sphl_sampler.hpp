// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
// Copyright (C) 2017-2019 Jonathan Metzgar
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
#ifndef FLUXIONS_SPHL_SAMPLER_HPP
#define FLUXIONS_SPHL_SAMPLER_HPP

#include <vector>
#include <fluxions_gte.hpp>
#include <fluxions_simple_scene_graph.hpp>
#include <fluxions_sphl.hpp>

namespace Fluxions
{
	class SphlSampler
	{
	public:
		SphlSampler();

		void resize(size_t samplesX, size_t samplesY);
		void saveSampleMap(const std::string& path, int pxscale);
		void sampleCubeMap(const Image4f& cubeMap, MultispectralSph4f& msph);

	private:
		const size_t pxscale = 4;
		Image3i sampleMap;
		size_t ix = 128;
		size_t iy = 64;
		size_t numSamples = 0;
		std::vector<Vector3f> randomVectors;
		std::vector<double> theta;
		std::vector<double> phi;
		std::vector<double> sph;
		std::vector<size_t> debug_sides;
	};
} // namespace Fluxions

#endif
