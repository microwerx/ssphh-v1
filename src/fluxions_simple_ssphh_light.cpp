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
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.If not, see <https://www.gnu.org/licenses/>.
//
// For any other type of licensing, please contact me at jmetzgar@outlook.com
#include "pch.hpp"
#include <damselfish_json.hpp>
#include <fluxions_simple_geometry_mesh.hpp>
#include <fluxions_sphl_sampler.hpp>
#include <unicornfish_corona_job.hpp>
#include <cassert>

namespace Fluxions
{
	bool SaveSphlOBJ(const std::string& path, const std::string& name, const Color3f Kd, std::vector<Vector3f>& verts, std::vector<Vector3ui>& triangles);

	bool SaveSphlOBJ(const std::string& path, const std::string& name, const Color3f Kd, std::vector<Vector3f>& verts, std::vector<Vector3ui>& triangles) {
		std::vector<Vector3f> normals;
		normals.resize(verts.size());
		for (auto& n : normals) {
			n.reset();
		}
		for (auto& tri : triangles) {
			Vector3f a = verts[tri.y] - verts[tri.x];
			Vector3f b = verts[tri.z] - verts[tri.x];
			Vector3f c = a.cross(b).normalize();
			normals[tri.x] += c;
			normals[tri.y] += c;
			normals[tri.z] += c;
		}
		std::ofstream fout;
		fout.open(path + "/" + name + ".obj");
		fout << "mtllib " << name << ".mtl\n";
		// output vertices
		for (const Vector3f& v : verts) {
			fout << "v " << v.x << " " << v.y << " " << v.z << "\n";
		}
		fout << "\n";
		for (Vector3f& n : normals) {
			n.normalize();
			fout << "vn " << n.x << " " << n.y << " " << n.z << "\n";
		}
		fout << "\n\ng " << name << "\n";
		fout << "usemtl " << name << "\n";
		// output faces
		for (const Vector3ui& t : triangles) {
			fout << "f ";
			fout << (t.x + 1) << "//" << (t.x + 1) << " ";
			fout << (t.y + 1) << "//" << (t.y + 1) << " ";
			fout << (t.z + 1) << "//" << (t.z + 1) << "\n";
		}
		fout.close();

		// Write a material
		fout.open(path + "/" + name + ".mtl");
		fout << "\nnewmtl " << name << "\n\n";
		fout << "    Kd " << Kd.r << " " << Kd.g << " " << Kd.b << "\n";
		fout << "    Ks 0.1 0.1 0.1\n";
		fout << "    Ns 1.0\n";
		fout << "    Tr 1.0";
		fout << "    d 0.0";
		fout.close();
		return true;
	}

	bool SphlImageTexture::LoadLightProbe(const std::string& path) {
		FilePathInfo fpi(path);
		if (fpi.DoesNotExist())
			return false;

		if (fpi.ext == ".ppm")
			lightProbe.loadPPM(path);
		else if (fpi.ext == ".exr")
			lightProbe.loadEXR(path);
		else
			Hf::Log.errorfn(__FUNCTION__, "Path %s is not a PPM or EXR", path.c_str());
		lightProbe.convertRectToCubeMap();
		return true;
	}

	bool SphlImageTexture::SphToLightProbe(const MultispectralSph4f& sph) {
		float v_coefs[4][121];

		size_t maxDegree = sph[0].GetMaxDegree();
		size_t numCoefs = sph[0].getMaxCoefficients();
		for (size_t j = 0; j < 4; j++) {
			for (size_t lm = 0; lm < numCoefs; lm++) {
				v_coefs[j][lm] = sph[j][lm];
			}
			for (size_t lm = numCoefs; lm < 121; lm++) {
				v_coefs[j][lm] = 0.0f;
			}
		}

		lightProbe.resize(32, 32, 6);
		for (int face = 0; face < lightProbe.depth(); face++) {
			for (int s = 0; s < lightProbe.width(); s++) {
				for (int t = 0; t < lightProbe.height(); t++) {
					float _s = (float)s / (float)lightProbe.width();
					float _t = (float)t / (float)lightProbe.height();
					Vector3f v;
					MakeCubeVectorFromFaceST(face, _s, 1.0f - _t, &v.x, &v.y, &v.z);
					v.normalize();
					Color4f color;
					float theta = v.theta();
					float phi = v.phi();

					color.r = calc_spherical_harmonic<float>((int)maxDegree, v_coefs[0], theta, phi);
					color.g = calc_spherical_harmonic<float>((int)maxDegree, v_coefs[1], theta, phi);
					color.b = calc_spherical_harmonic<float>((int)maxDegree, v_coefs[2], theta, phi);
					color.a = calc_spherical_harmonic<float>((int)maxDegree, v_coefs[3], theta, phi);

					lightProbe.setPixelUnsafe(s, t, face, color);
				}
			}
		}
		return true;
	}

	bool SphlImageTexture::UploadLightProbe() {
		texture.Bind(0);
		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, (GLsizei)lightProbe.width(), (GLsizei)lightProbe.height(), 0, GL_RGBA, GL_FLOAT, lightProbe.getImageData(i));
		}
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		texture.Unbind();
		return true;
	}

	SimpleSSPHHLight::SimpleSSPHHLight() {
		//hierLightProbeImage.resize(32, 32, 6);
	}

	SimpleSSPHHLight::~SimpleSSPHHLight() {}

	void SimpleSSPHHLight::MakeDirty() {
		dirty = true;
	}

	void SimpleSSPHHLight::ChangeDegrees(int degrees) {
		if (degrees < 0)
			return;
		if (degrees > MaxSphlDegree)
			degrees = 9;
		maxDegree = degrees;
		dirty = true;
		for (int i = 0; i < 4; i++)
			msph[i].resize(maxDegree);

		//int i = 0;
		//for (auto & hier : hierarchies)
		//{
		//	hier.resize(maxDegree);
		//	hier.index = i;
		//	i++;
		//}
	}

	void SimpleSSPHHLight::ResizeHierarchies(int size) {
		if (size < 0 || size > MaxSphlLights)
			return;

		vizgenLightProbes.resize(size);
	}

	void SimpleSSPHHLight::Randomize(float size) {
		for (int l = 0; l <= maxDegree; l++) {
			for (int m = -l; m <= l; m++) {
				msph[0].setCoefficient(l, m, randomSampler(-size, size));
				msph[1].setCoefficient(l, m, randomSampler(-size, size));
				msph[2].setCoefficient(l, m, randomSampler(-size, size));
				msph[3].setCoefficient(l, m, randomSampler(-size, size));
			}
		}
		MakeDirty();
	}

	// Reads from a Path Traced Light Probe (a cube map stored images from left to right in a single image).
	bool SimpleSSPHHLight::ReadPtrcLightProbe(const std::string& path) {
		FilePathInfo fpi(path);
		if (fpi.DoesNotExist())
			return false;

		if (fpi.ext == ".ppm")
			ptrcLightProbeImage.loadPPM(path);
		else if (fpi.ext == ".exr")
			ptrcLightProbeImage.loadEXR(path);
		else
			Hf::Log.errorfn(__FUNCTION__, "Path %s is not a PPM or EXR", path.c_str());
		ptrcLightProbeImage.convertRectToCubeMap();
		double intensity = ptrcLightProbeImage.getIntensity();
		double numPixels = ptrcLightProbeImage.getNumPixels();
		ptrcLightProbePercent = (float)(intensity / numPixels);
		ChangeDegrees(MaxSphlDegree);

		LightProbeToSph(ptrcLightProbeImage, msph);
		SphToLightProbe(msph, msphLightProbeImage);

		UploadLightProbe(ptrcLightProbeImage, ptrcLightProbeTexture);
		UploadLightProbe(msphLightProbeImage, msphLightProbeTexture);
		// UploadLightProbe(hierLightProbeImage, hierLightProbeTexture);

		dirty = true;

		return true;
	}

	// Saves to a Path Traced Light Probe (a cube map with cube faces stored from left to right in a single image).
	bool SimpleSSPHHLight::SavePtrcLightProbe(const std::string& path) {
		FilePathInfo fpi(path);

		SphToLightProbe(msph, msphLightProbeImage);
		msphLightProbeImage.convertCubeMapToRect();
		if (fpi.ext == ".ppm")
			msphLightProbeImage.savePPMi(path, 255.99f, 0, 255, 0, false);
		else if (fpi.ext == ".exr")
			msphLightProbeImage.saveEXR(path);

		return false;
	}

	// Saves a JSON form of the multispectral (RGBI) of this SPH. I represents a monochromatic version of the RGB components. { maxDegree: (1-10), coefs : [] }
	bool SimpleSSPHHLight::SaveJsonSph(const std::string& path) {
		FilePathInfo fpi(path);
		Df::JSONPtr json = Df::JSON::MakeObject({
			{"numChannels", Df::JSON::MakeNumber(3)},
			{"maxDegree", Df::JSON::MakeNumber((int)msph[0].GetMaxDegree())},
			{"coefs", Df::JSON::MakeArray()},
			{"meta", Df::JSON::MakeObject()}
			});

		auto coefs = json->getMember("coefs");
		for (size_t lm = 0; lm < msph->getMaxCoefficients(); lm++) {
			float r = msph[0].getCoefficient(lm);
			float g = msph[1].getCoefficient(lm);
			float b = msph[2].getCoefficient(lm);
			Df::JSONPtr rgb = Df::JSON::MakeArray({ r, g, b });
			coefs->PushBack(rgb);
		}

		auto meta = json->getMember("meta");
		Df::JSONPtr m_meta = Df::JSON::MakeObject({
			{ "name", Df::JSON::MakeString(name) },
			{ "position", Df::JSON::MakeArray({
				position.x,
				position.y,
				position.z
				})
			}
			});
		meta->PushBack(m_meta);

		std::ofstream fout(fpi.path);
		fout << json->Serialize();
		fout.close();

		return true;
	}

	// Reads a JSON format of a multispectral (RGBL) of this SPH. L represents a monochromatic version of the RGB components. { maxDegree: (1-10), coefs : [] }
	bool SimpleSSPHHLight::ReadJsonSph(const std::string& path) {
		FilePathInfo fpi(path);
		if (fpi.DoesNotExist())
			return false;
		std::ifstream fin(fpi.path);
		std::string buffer;
		while (!fin) {
			std::string line;
			getline(fin, line);
			buffer += "\n";
			buffer += line;
		}
		fin.close();

		Df::JSONPtr json = Df::JSON::New();
		if (!json->Deserialize(buffer)) {
			Hf::Log.error("Unable to read JSON SPH");
			return false;
		}

		if (json->IsObject() &&
			json->getMember("numChannels")->IsNumber() &&
			json->getMember("maxDegree")->IsNumber() &&
			json->getMember("coefs")->IsArray()) {
			// int jsonMaxDegree = json->getMember("maxDegree")->AsInt();
			Df::JSONPtr coefs = json->getMember("coefs");
			for (size_t j = 0; j < 4; j++) {
				Df::JSONPtr e = coefs->getElement((int)j);
				if (e->IsArray()) {
					std::vector<float> coefJ;
					coefJ = e->AsFloatArray();
				}
			}
		}

		Df::JSONPtr meta = json->getMember("meta");
		if (meta->IsObject()) {
			auto nameMember = meta->getMember("name");
			if (nameMember->IsString()) {
				name = nameMember->AsString();
			}

			auto m_position = meta->getMember("position");
			if (m_position->IsArray()) {
				std::vector<float> v = m_position->AsFloatArray();
				if (v.size() >= 3) {
					position.reset(v[0], v[1], v[2]);
				}
			}
			else if (m_position->IsObject()) {
				Df::JSONPtr x = m_position->getMember("x");
				Df::JSONPtr y = m_position->getMember("y");
				Df::JSONPtr z = m_position->getMember("z");
				if (x->IsNumber() && y->IsNumber() && z->IsNumber()) {
					position.reset(x->AsFloat(), y->AsFloat(), z->AsFloat());
				}
			}
		}

		return false;
	}

	bool SimpleSSPHHLight::SaveOBJ(const std::string& path, const std::string& gname) {
		FilePathInfo fpi(path);
		static const char* icos_path = "resources/models/icos4.txt";
		static size_t numVertices = 0;
		static size_t numTriangles = 0;
		static std::vector<Vector3f> vertices;
		static std::vector<Vector3ui> triangles;
		static std::vector<Vector3f> rgbiy[5];

		if (!fpi.IsDirectory()) {
			Hf::Log.errorfn(__FUNCTION__, "Path %s is not a directory!", path.c_str());
			return false;
		}

		if (numTriangles == 0) {
			FilePathInfo geosphere(icos_path);

			if (!geosphere.Exists()) {
				Hf::Log.errorfn(__FUNCTION__, "Icosahedron %s does not exist", icos_path);
				return false;
			}
			std::ifstream fin(icos_path);
			fin >> numVertices;
			vertices.resize(numVertices);
			for (Vector3f& v : vertices) {
				fin >> v.x;
				fin >> v.y;
				fin >> v.z;
				v.normalize();
			}
			fin >> numTriangles;
			triangles.resize(numTriangles);
			for (Vector3ui& t : triangles) {
				fin >> t.x >> t.y >> t.z;
			}
			fin.close();
		}

		// Make sure new RGBIY vertices are the right size
		for (auto& x : rgbiy) {
			x.resize(vertices.size());
		}

		for (size_t i = 0; i < 3; i++) {
			if (msph[i].GetMaxDegree() < 5) {
				msph[i].resize(5);
				for (size_t lm = 0; lm < msph[i].getMaxCoefficients(); lm++) {
					float x = 2.0f * (float)rand() / (float)RAND_MAX - 1.0f;
					msph[i].setCoefficient(lm, x);
				}
			}
		}

		// Calculate the 3D positions for the red, green,
		// blue, intensity, and luma Y' channels
		for (size_t i = 0; i < vertices.size(); i++) {
			Vector3f& v = vertices[i];
			float theta = v.theta();
			float phi = v.phi();
			float r = this->msph[0].calc(theta, phi);
			float g = this->msph[1].calc(theta, phi);
			float b = this->msph[2].calc(theta, phi);
			float I = 0.333333f * (r + g + b);
			float Y = 0.299f * r + 0.587f * g + 0.114f * b;
			rgbiy[0][i] = (0.5f + 0.25f * r) * v;
			rgbiy[1][i] = (0.5f + 0.25f * g) * v;
			rgbiy[2][i] = (0.5f + 0.25f * b) * v;
			rgbiy[3][i] = (0.5f + 0.25f * I) * v;
			rgbiy[4][i] = (0.5f + 0.25f * Y) * v;
			if (i < 5) std::cout << r << std::endl;
		}
		const Color3f R(1.0f, 0.0f, 0.0f);
		const Color3f G(0.0f, 1.0f, 0.0f);
		const Color3f B(0.0f, 0.0f, 1.0f);
		const Color3f W(1.0f, 1.0f, 1.0f);

		// Try to save all the files; stop if one fails
		bool result = true;
		if (result) result = SaveSphlOBJ(path, gname + "_sph_ch0_R", R, rgbiy[0], triangles);
		if (result) result = SaveSphlOBJ(path, gname + "_sph_ch1_G", G, rgbiy[1], triangles);
		if (result) result = SaveSphlOBJ(path, gname + "_sph_ch2_B", B, rgbiy[2], triangles);
		if (result) result = SaveSphlOBJ(path, gname + "_sph_ch3_I", W, rgbiy[3], triangles);
		if (result) result = SaveSphlOBJ(path, gname + "_sph_ch4_Y", W, rgbiy[4], triangles);

		return result;
		//if (!__g_sphl_icos) {
		//	__g_sphl_icos = new SimpleGeometryMesh();
		//	for (size_t i = 0; i < numVertices; i++) {
		//		__g_sphl_icos->Attrib3f(0, vertices[i]);
		//	}
		//	__g_sphl_icos->BeginSurface(SimpleGeometryMesh::SurfaceType::Triangles);
		//	for (size_t i = 0; i < numTriangles; i++) {
		//		__g_sphl_icos->AddIndex(triangles[i * 3 + 0]);
		//		__g_sphl_icos->AddIndex(triangles[i * 3 + 1]);
		//		__g_sphl_icos->AddIndex(triangles[i * 3 + 2]);
		//	}
		//	__g_sphl_icos->EnableAttrib(0);
		//}

		/* TODO: Move this into the ASCM section
		const float a = 0.0f;
		const float b = 1.0f;
		const float c = ((1.0f + sqrtf(5.0f)) / 2.0f);
		const Vector3f A1(+a, +c, -b);
		const Vector3f A2(+a, -c, -b);
		const Vector3f A3(+a, -c, +b);
		const Vector3f A4(+a, +c, +b);
		const Vector3f B1(-b, +a, -c);
		const Vector3f B2(+b, +a, -c);
		const Vector3f B3(+b, +a, +c);
		const Vector3f B4(-b, +a, +c);
		const Vector3f C1(-c, +b, +a);
		const Vector3f C2(-c, -b, +a);
		const Vector3f C3(+c, -b, +a);
		const Vector3f C4(+c, +b, +a);

		std::vector<Vector3f> verts = {
			Vector3f(+0.0000,+1.0000,+0.0000),
			Vector3f(+0.8944,+0.4472,+0.0000),
			Vector3f(+0.2764,+0.4472,-0.8507),
			Vector3f(-0.7236,+0.4472,-0.5257),
			Vector3f(-0.7236,+0.4472,+0.5257),
			Vector3f(+0.2764,+0.4472,+0.8507),
			Vector3f(+0.7236,-0.4472,+0.5257),
			Vector3f(+0.7236,-0.4472,-0.5257),
			Vector3f(-0.2764,-0.4472,-0.8507),
			Vector3f(-0.8944,-0.4472,+0.0000),
			Vector3f(-0.2764,-0.4472,+0.8507),
			Vector3f(+0.0000f,-1.0000,0.0000)
		};

		std::vector<int> indcs = {
			0,1,2,
			0,2,3,
			0,3,4,
			0,4,5,
			0,5,1,
			1,6,7,
			2,7,8,
			3,8,9,
			4,9,10,
			5,10,6,
			7,2,1,
			8,3,2,
			9,4,3,
			10,5,4,
			6,1,5,
			11,8,7,
			11,9,8,
			11,10,9,
			11,6,10,
			11,7,6
		};

		// subdivide
		std::vector<Vector3f> inew;
		for (size_t i = 0; i < indcs.size(); i += 3) {
			size_t vi = indcs[i];
			size_t vj = indcs[i + 1];
			size_t vk = indcs[i + 2];
			Vector3f sidea(0.5 * (verts[vi] + verts[vj]));
			Vector3f sideb(0.5 * (verts[vj] + verts[vk]));
			Vector3f sidec(0.5 * (verts[vk] + verts[vi]));
			size_t first = inew.size();
			verts.push_back(sidea);
			verts.push_back(sideb);
			verts.push_back(sidec);
		}
		*/
	}

	bool SimpleSSPHHLight::LightProbeToSph(const Image4f& lightProbe, MultispectralSph4f& sph) {
		SphlSampler sphlSampler;
		sphlSampler.resize(64, 32);
		sphlSampler.sampleCubeMap(lightProbe, sph);
		MakeIntensityChannel4f(sph);
		return false;
	}

	bool SimpleSSPHHLight::SphToLightProbe(const MultispectralSph4f& sph, Image4f& lightProbe) {
		float v_coefs[4][121];

		for (int j = 0; j < 4; j++) {
			for (int l = 0; l <= maxDegree; l++) {
				for (int m = -l; m <= l; m++) {
					auto lm = sph[j].getCoefficientIndex(l, m);
					v_coefs[j][lm] = E0 * sph[j][lm];
				}
			}
		}

		lightProbe.resize(32, 32, 6);
		for (int face = 0; face < lightProbe.depth(); face++) {
			for (int s = 0; s < lightProbe.width(); s++) {
				for (int t = 0; t < lightProbe.height(); t++) {
					float _s = (float)s / (float)lightProbe.width();
					float _t = (float)t / (float)lightProbe.height();
					Vector3f v;
					MakeCubeVectorFromFaceST(face, _s, 1.0f - _t, &v.x, &v.y, &v.z);
					v.normalize();
					Color4f color;
					float theta = v.theta();
					float phi = v.phi();

					color.r = calc_spherical_harmonic<float>(maxDegree, v_coefs[0], theta, phi);
					color.g = calc_spherical_harmonic<float>(maxDegree, v_coefs[1], theta, phi);
					color.b = calc_spherical_harmonic<float>(maxDegree, v_coefs[2], theta, phi);
					color.a = calc_spherical_harmonic<float>(maxDegree, v_coefs[3], theta, phi);

					lightProbe.setPixelUnsafe(s, t, face, color);
				}
			}
		}
		return true;
	}

	bool SimpleSSPHHLight::SphToLightProbe(const MultispectralSph4f& sph, Image4f& lightProbe, int maxDegree_) {
		float v_coefs[4][121];

		maxDegree_ = std::min(maxDegree_, this->maxDegree);

		for (int j = 0; j < 4; j++) {
			for (int l = 0; l <= maxDegree_; l++) {
				for (int m = -l; m <= l; m++) {
					auto lm = sph[j].getCoefficientIndex(l, m);
					v_coefs[j][lm] = E0 * sph[j][lm];
				}
			}
		}

		lightProbe.resize(32, 32, 6);
		for (int face = 0; face < lightProbe.depth(); face++) {
			for (int s = 0; s < lightProbe.width(); s++) {
				for (int t = 0; t < lightProbe.height(); t++) {
					float _s = (float)s / (float)lightProbe.width();
					float _t = (float)t / (float)lightProbe.height();
					Vector3f v;
					MakeCubeVectorFromFaceST(face, _s, 1.0f - _t, &v.x, &v.y, &v.z);
					v.normalize();
					Color4f color;
					float theta = v.theta();
					float phi = v.phi();

					color.r = calc_spherical_harmonic<float>(maxDegree_, v_coefs[0], theta, phi);
					color.g = calc_spherical_harmonic<float>(maxDegree_, v_coefs[1], theta, phi);
					color.b = calc_spherical_harmonic<float>(maxDegree_, v_coefs[2], theta, phi);
					color.a = calc_spherical_harmonic<float>(maxDegree_, v_coefs[3], theta, phi);

					lightProbe.setPixelUnsafe(s, t, face, color);
				}
			}
		}
		return true;
	}

	bool SimpleSSPHHLight::UploadLightProbe(Image4f& lightProbe, SimpleGpuTexture& texture) {
		if (lightProbe.empty())
			lightProbe.resize(32, 32, 6);

		texture = SimpleGpuTexture(GL_TEXTURE_CUBE_MAP);
		texture.Bind(0);
		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, (GLsizei)lightProbe.width(), (GLsizei)lightProbe.height(), 0, GL_RGBA, GL_FLOAT, lightProbe.getImageData(i));
		}
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		texture.Unbind();
		return true;
	}

	bool SimpleSSPHHLight::UploadLightProbe(Image4f& lightProbe, GLuint& texture) {
		if (lightProbe.empty())
			lightProbe.resize(32, 32, 6);

		if (texture == 0) {
			glGenTextures(1, &texture);
		}
		//texture = SimpleGpuTexture(GL_TEXTURE_CUBE_MAP);
		//texture.Bind(0);
		FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, (GLsizei)lightProbe.width(), (GLsizei)lightProbe.height(), 0, GL_RGBA, GL_FLOAT, lightProbe.getImageData(i));
		}
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		//texture.Unbind();
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		return true;
	}

	Color3f SimpleSSPHHLight::GetCoefficientColor(int l, int m) const {
		return Color3f(
			GetCoefficient(0, l, m),
			GetCoefficient(1, l, m),
			GetCoefficient(2, l, m));
	}

	float SimpleSSPHHLight::GetCoefficient(int j, int l, int m) const {
		if (j < 0 || j > 4)
			return 0.0f;
		return msph[j].getCoefficient(l, m);
	}

	float SimpleSSPHHLight::GetCoefficient(int j, int lm) const {
		if (!within(j, 0, 3))
			return 0.0f;
		return msph[j].getCoefficient(lm);
	}

	void SimpleSSPHHLight::ZeroCoefficients() {
		SetCoefficient(-1000, -1000, -1000, 0.0f);
	}

	void SimpleSSPHHLight::IsolateCoefficient(int j, int l, int m) {
		float value = GetCoefficient(j, l, m);
		SetCoefficient(-1000, -1000, -1000, 0.0f);
		SetCoefficient(j, l, m, value);
	}

	void SimpleSSPHHLight::SetCoefficient(int j, int l, int m, float value) {
		if (j > 4)
			return;

		int minJ = 0;
		int maxJ = 0;
		int minL = 0;
		int maxL = 0;
		int minM = 0;
		int maxM = 0;

		if (j < 0) {
			minJ = 0;
			maxJ = 4;
		}
		else {
			minJ = j;
			maxJ = j + 1;
		}

		if (l < 0) {
			minL = 0;
			maxL = maxDegree;
			m = -maxL - 1;
		}
		else {
			minL = l;
			maxL = l + 1;
		}
		for (int _j = minJ; _j < maxJ; _j++) {
			for (int _l = minL; _l <= maxL; _l++) {
				if (m < -_l) {
					minM = -_l;
					maxM = _l;
				}
				else {
					minM = m;
					maxM = m;
				}
				for (int _m = minM; _m <= maxM; _m++) {
					msph[_j].setCoefficient(_l, _m, value);
				}
			}
		}
		dirty = true;
	}

	void SimpleSSPHHLight::RandomizeCoefficient(int j, int l, int m, float minValue, float maxValue) {
		if (j > 4)
			return;

		int minJ = 0;
		int maxJ = 0;
		int minL = 0;
		int maxL = 0;
		int minM = 0;
		int maxM = 0;

		if (j < 0) {
			minJ = 0;
			maxJ = 4;
		}
		else {
			minJ = j;
			maxJ = j + 1;
		}

		if (l < 0) {
			minL = 0;
			maxL = maxDegree;
		}
		else {
			minL = l;
			maxL = l + 1;
		}
		for (int _j = minJ; _j < maxJ; _j++) {
			for (int _l = minL; _l <= maxL; _l++) {
				if (m < 0) {
					minM = -_l;
					maxM = _l;
				}
				else {
					minM = m;
					maxM = m;
				}
				for (int _m = minM; _m <= maxM; _m++) {
					msph[_j].setCoefficient(_l, _m, randomSampler(minValue, maxValue));
				}
			}
		}

		dirty = true;
	}

	void SimpleSSPHHLight::SortHierarchiesByPercent() {

	}

	void SimpleSSPHHLight::SortHierarchiesByIndex() {

	}

	void SimpleSSPHHLight::CopySphlToHierarchies() {
		//if (index < 0)
		//	throw "SSPHHLight not properly initialized";
		//hierarchies[index].CopySph(msph, 1.0);
	}

	void SimpleSSPHHLight::SetHierarchyDescriptionToIndex() {
		//std::ostringstream ostr;
		//int i = 0;
		//ostr << "(" << hierarchies.size() << ") => [ ";
		//for (auto & hier : hierarchies)
		//{
		//	if (i != 0)
		//		ostr << ", ";
		//	ostr << hier.index;
		//	i++;
		//}
		//ostr << " ]";
		//hier_description = ostr.str();
	}

	void SimpleSSPHHLight::SetHierarchyDescriptionToPercent() {
		//std::ostringstream ostr;
		//int i = 0;
		//ostr << "(" << hierarchies.size() << ") => [ ";
		//for (auto & hier : hierarchies)
		//{
		//	if (i != 0)
		//		ostr << ", ";
		//	ostr << std::fixed << std::setprecision(2) << (hier.percentVisible * 100.0) << "%";
		//	i++;
		//}
		//ostr << " ]";
		//hier_description = ostr.str();
	}

	void SimpleSSPHHLight::SetHierarchyDescription() {
		//std::ostringstream ostr;
		//int i = 0;
		//ostr << "(" << index << "," << hierarchies.size() << ") => [ ";
		//for (auto & hier : hierarchies)
		//{

		//	float weight = 0.0;
		//	weight += hier.sph.sum();

		//	if (i != 0)
		//		ostr << ", ";
		//	ostr << "(" << hier.index << ", " << std::fixed << std::setprecision(2) << (hier.percentVisible * 100.0) << "%, " << weight << ")";
		//	i++;
		//}
		//ostr << " ]";
		//hier_description = ostr.str();
	}

	bool MakeStandardizedSph(SphericalHarmonicf& sph, MultispectralSph4f& msph) {
		for (size_t i = 0; i < 4; i++) {
			if (sph.GetMaxDegree() != msph[i].GetMaxDegree())
				return false;
		}
		auto lmmax = sph.getMaxCoefficients();

		for (size_t lm = 0; lm < lmmax; lm++) {
			float r = msph[0].getCoefficient(lm);
			float g = msph[1].getCoefficient(lm);
			float b = msph[2].getCoefficient(lm);
			sph.setCoefficient(lm, (r + g + b) / 3.0f);
			msph[3].setCoefficient(lm, 0.2126f * r + 0.7152f * g + 0.0722f * b);
		}
		return true;
	}

	bool MakeIntensityChannel4f(MultispectralSph4f& msph) {
		size_t lmmax = msph[0].getMaxCoefficients();

		for (size_t lm = 0; lm < lmmax; lm++) {
			float r = msph[0].getCoefficient(lm);
			float g = msph[1].getCoefficient(lm);
			float b = msph[2].getCoefficient(lm);
			msph[3].setCoefficient(lm, (r + g + b) / 3.0f);
		}
		return true;
	}

	bool MakeLuminanceChannel4f(MultispectralSph4f& msph) {
		size_t lmmax = msph[0].getMaxCoefficients();

		for (size_t lm = 0; lm < lmmax; lm++) {
			float r = msph[0].getCoefficient(lm);
			float g = msph[1].getCoefficient(lm);
			float b = msph[2].getCoefficient(lm);
			msph[3].setCoefficient(lm, 0.2126f * r + 0.7152f * g + 0.0722f * b);
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////
	// SSPHH Algorithm /////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	SimpleSSPHH::SimpleSSPHH() {}

	SimpleSSPHH::~SimpleSSPHH() {}

	void SimpleSSPHH::INIT(SimpleSceneGraph& ssg) {
		Hf::Log.infofn(__FUNCTION__, "SSPHH INIT");
		sceneName = ssg.name;
		sphls_ = &ssg.ssphhLights;
		size_ = sphls_->size();
		S.resize(size_);
		for (auto& S_i : S) {
			S_i.resize(MaxSphlDegree, 0.0f);
		}
		H.resize(size_);
		for (auto& h_i : H) {
			h_i.resize(size_);
			for (auto& h_ij : h_i) {
				h_ij.resize(MaxSphlDegree, 0.0f);
			}
		}
		P.resize(size_);
		for (auto& p : P) {
			p.resize(size_, 0.0f);
		}
		Q.resize(size_);
		for (size_t i = 0; i < size_; i++) {
			Q[i].index = (int)i;
		}
		Qsorted.resize(size_);
		Sprime.resize(size_);
		for (auto& S_i : Sprime) {
			S_i.resize(MaxSphlDegree);
		}
		self.resize(size_);
		for (auto& s : self) {
			s.resize(MaxSphlDegree);
		}
		neighbor.resize(size_);
		for (auto& n : neighbor) {
			n.resize(MaxSphlDegree);
		}
	}

	void SimpleSSPHH::GEN() {
		if (!sphls_)
			return;
		Hf::Log.infofn(__FUNCTION__, "SSPHH GEN");
		//auto &sphls = *sphls_;

		int i = 0;
		for (auto& sphl : *sphls_) {
			S[i].resize(sphl.maxDegree);
			sphl.LightProbeToSph(sphl.vizgenLightProbes[i], S[i].msph);
			H[i][i] = S[i];
			P[i][i] = 1.0f;

			float lm0, lm1, lm2, lm3;
			if (sphl.maxDegree >= 1) {
				lm0 = S[i].a().getCoefficient(0);
				lm1 = S[i].a().getCoefficient(1);
				lm2 = S[i].a().getCoefficient(2);
				lm3 = S[i].a().getCoefficient(3);
			}
			else {
				lm0 = -1.0f;
				lm1 = -1.0f;
				lm2 = -1.0f;
				lm3 = -1.0f;
			}

			std::string basename = "output/" + Uf::CoronaJob::MakeGENName(sceneName, (int)i);
			if (savePPMs) {
				Image4f lightProbe(32, 32, 6);
				sphl.SphToLightProbe(H[i][i].msph, lightProbe);
				lightProbe.convertCubeMapToRect();
				lightProbe.savePPMi(basename + "_sph.ppm", 255.99f, 0, 255);
				lightProbe.saveEXR(basename + "_sph.exr");
			}
			if (saveJSONs) {
				H[i][i].SaveJSON(basename + ".json", basename, sphl.position.xyz());
			}
			Hf::Log.info("%s(): (%d, %d) -> [ %.2f, %.2f, %.2f, %.2f ]", __FUNCTION__, i, i, lm0, lm1, lm2, lm3);
			i++;
		}
	}

	void SimpleSSPHH::VIZ() {
		if (!sphls_)
			return;
		Hf::Log.infofn(__FUNCTION__, "SSPHH VIZ");

		auto& sphls = *sphls_;

		for (size_t i = 0; i < size_; i++) {
			auto& sphl = sphls[i];
			for (size_t j = 0; j < size_; j++) {
				if (i == j)
					continue;
				if (sphl.vizgenLightProbes.empty()) {
					Hf::Log.errorfn(__FUNCTION__, "VIZ() called with no light probes!");
					continue;
				}

				H[i][j].resize(sphl.maxDegree);
				sphl.LightProbeToSph(sphl.vizgenLightProbes[j], H[i][j].msph);

				std::string basename = "output/" + Uf::CoronaJob::MakeVIZName(sceneName, (int)i, (int)j);
				if (savePPMs) {
					Image4f lightProbe(32, 32, 6);
					sphl.SphToLightProbe(H[i][j].msph, lightProbe);
					lightProbe.convertCubeMapToRect();
					lightProbe.savePPMi(basename + "_sph.ppm", 255.99f, 0, 255);
					lightProbe.saveEXR(basename + "_sph.exr");
				}
				if (saveJSONs)
					H[i][j].SaveJSON(basename + ".json", basename, sphl.position.xyz());

				// TODO: calculate visibility percentage
				P[i][j] = 0.25f;

				float lm0, lm1, lm2, lm3;
				if (sphl.maxDegree >= 1) {
					lm0 = H[i][j].a().getCoefficient(0);
					lm1 = H[i][j].a().getCoefficient(1);
					lm2 = H[i][j].a().getCoefficient(2);
					lm3 = H[i][j].a().getCoefficient(3);
				}
				else {
					lm0 = -1.0f;
					lm1 = -1.0f;
					lm2 = -1.0f;
					lm3 = -1.0f;
				}
				Hf::Log.infofn(__FUNCTION__, "(%d, %d) -> [ %.2f, %.2f, %.2f, %.2f ]", i, j, lm0, lm1, lm2, lm3);
			}
		}
		return;
	}

	void SimpleSSPHH::HIER(bool includeSelf, bool includeNeighbor, int MaxDegrees) {
		if (!sphls_)
			return;
		Hf::Log.infofn(__FUNCTION__, "SSPHH HIER");

		auto& sphls = *sphls_;
		for (size_t i = 0; i < size_; i++) {
			for (size_t j = 0; j < size_; j++) {
				Q[j].index = (int)j;
				Q[j].p = P[i][j];
			}
			Qsorted = Q;
			sort(Qsorted.begin(), Qsorted.end(), [](Qpair& a, Qpair& b) {
				if (a.p > b.p)
					return true;
				return false;
				});
			Sprime[i].reset();
			self[i].reset();
			neighbor[i].reset();
			for (size_t j = 0; j < size_; j++) {
				assert(Q[j].index >= 0);
				size_t Qj_index = (size_t)Q[j].index;
				if (includeNeighbor && Qj_index != i)
					Sprime[i].Accumulate(H[i][j], S[Qj_index], VIZmix, MaxDegrees);
				if (includeSelf && Qj_index == i)
					Sprime[i].Accumulate(S[Qj_index], Q[j].p, MaxDegrees);

				if (Qj_index == i) {
					self[i].Accumulate(S[Qj_index], Q[j].p, MaxDegrees);
				}
				else {
					neighbor[i].Accumulate(H[i][j], S[Qj_index], MaxDegrees);
				}
				float lm0, lm1, lm2, lm3;
				if (H[i][j].a().GetMaxDegree() >= 1) {
					lm0 = Sprime[i].a().getCoefficient(0);
					lm1 = Sprime[i].a().getCoefficient(1);
					lm2 = Sprime[i].a().getCoefficient(2);
					lm3 = Sprime[i].a().getCoefficient(3);
				}
				else {
					lm0 = -1.0f;
					lm1 = -1.0f;
					lm2 = -1.0f;
					lm3 = -1.0f;
				}
				// Hf::Log.info("%s(): (%d, %d) S' += H * %.2f -> [ %.2f, %.2f, %.2f, %.2f ]", __FUNCTION__, i, j, Q[j].p, lm0, lm1, lm2, lm3);
			}
		}

		for (size_t i = 0; i < size_; i++) {
			sphls[i].self = self[i];
			sphls[i].neighbor = neighbor[i];
			sphls[i].SphToLightProbe(Sprime[i].msph, sphls[i].hierLightProbeImage);
			sphls[i].UploadLightProbe(sphls[i].hierLightProbeImage, sphls[i].hierLightProbeTexture);

			std::string base = "output/" + Uf::CoronaJob::MakeHIERName(sceneName, (int)i, (int)MaxDegrees);

			if (savePPMs) {
				Image4f lightProbeSprime;
				sphls[i].SphToLightProbe(Sprime[i].msph, lightProbeSprime, MaxDegrees);
				lightProbeSprime.convertCubeMapToRect();
				lightProbeSprime.savePPMi(base + "_01_Sprime.ppm", 255.99f, 0, 0);
				lightProbeSprime.saveEXR(base + "_01_Sprime.exr");
			}
			if (saveJSONs) {
				std::string basename = base + "_01_Sprime";
				Sprime[i].SaveJSON(base + "_01_Sprime.json", basename, sphls[i].position.xyz());
			}

			if (savePPMs) {
				Image4f lightProbeSelf;
				sphls[i].SphToLightProbe(self[i].msph, lightProbeSelf, MaxDegrees);
				lightProbeSelf.convertCubeMapToRect();
				lightProbeSelf.savePPMi(base + "_02_self.ppm", 255.99f, 0, 0);
				lightProbeSelf.saveEXR(base + "_02_self.exr");
			}
			if (saveJSONs) {
				std::string basename = base + "_02_self";
				self[i].SaveJSON(base + "_02_self.json", basename, sphls[i].position.xyz());
			}

			if (savePPMs) {
				Image4f lightProbeNeighbor;
				sphls[i].SphToLightProbe(neighbor[i].msph, lightProbeNeighbor, MaxDegrees);
				lightProbeNeighbor.convertCubeMapToRect();
				lightProbeNeighbor.savePPMi(base + "_03_neighbor.ppm", 255.99f, 0, 0);
				lightProbeNeighbor.saveEXR(base + "_03_neighbor.exr");
			}
			if (saveJSONs) {
				std::string basename = base + "_03_neighbor";
				neighbor[i].SaveJSON(base + "_03_neighbor.json", basename, sphls[i].position.xyz());
			}

			float lm0, lm1, lm2, lm3;
			if (Sprime[i].a().GetMaxDegree() >= 1) {
				lm0 = Sprime[i].a().getCoefficient(0);
				lm1 = Sprime[i].a().getCoefficient(1);
				lm2 = Sprime[i].a().getCoefficient(2);
				lm3 = Sprime[i].a().getCoefficient(3);
			}
			else {
				lm0 = -1.0f;
				lm1 = -1.0f;
				lm2 = -1.0f;
				lm3 = -1.0f;
			}
			Hf::Log.infofn(__FUNCTION__, "S'_%d -> [ %.2f, %.2f, %.2f, %.2f ]", i, lm0, lm1, lm2, lm3);
		}
		return;
	}
} // namespace Fluxions