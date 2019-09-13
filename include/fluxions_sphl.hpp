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
#ifndef FLUXIONS_SPHL_HPP
#define FLUXIONS_SPHL_HPP

#include <vector>
#include <fluxions_opengl.hpp>
#include <fluxions_gte.hpp>
#include <fluxions_simple_geometry_mesh.hpp>
#include <fluxions_gte_image.hpp>

namespace Fluxions
{

	struct FxModel
	{
		std::vector<Vector3f> vertices;
		std::vector<Vector3i> triangles;
		size_t vertexCount = 0;
		size_t triangleCount = 0;

		void load(const char* path);
		void makeModel(SimpleGeometryMesh& model);
	};

	struct Sphl
	{
		// constants
		const static int MaxCoefficients = 121;
		const static int MaxComponents = 6;
		const static int RedComponent = 0;
		const static int GreenComponent = 1;
		const static int BlueComponent = 2;
		const static int LuminanceComponent = 3;
		const static int HierarchySelfComponent = 4;
		const static int HierarchyNeighborComponent = 5;

		// shader parameters
		bool enabled = false;
		// if w = 1, then light source is treated as a local light (V = Vertex Position - Light Position)
		// if w = 0, then light source is treated as a global light (V = Vertex Normal)
		Vector4f position;
		Color4f E0;
		// light probe texture 0 = Unsampled raw light probe
		// light probe texture 1 = SPH precomputed light probe
		// light probe texture 2 = ??? SPH precomputed Y values (degrees 0 and 1)
		GLuint lightProbeTexIds[3] = { 0, 0, 0 };
		// SPHL coefficients
		int maxDegree = 0;
		bool makeSimpleSPHLs = false;
		Vector4f coefs[MaxCoefficients];

		// other parameters
		float v_coefs[MaxComponents][MaxCoefficients];
		SimpleGeometryMesh sph_model;
		SimpleGeometryMesh lightProbeModel;
		Image4f lightProbe;

		Sphl();

		void randomize();
		void copyCoefficients(const SimpleSSPHHLight& sphl, int maxDegrees = -1, bool simpleSPHLs = false);
		void createMesh(FxModel& model);
		void createLightProbe();
	};
} // namespace Fluxions

#endif
