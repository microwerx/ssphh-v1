#ifndef FLUXIONS_SIMPLE_SSPHH_LIGHT_HPP
#define FLUXIONS_SIMPLE_SSPHH_LIGHT_HPP

#include <vector>
#include <fluxions_gte.hpp>
#include <fluxions_gte_spherical_harmonic.hpp>
#include <fluxions_gte_image.hpp>
#include <fluxions_simple_scene_graph.hpp>

namespace Fluxions
{
	class SimpleSSPHHLight
	{
	public:
		bool enabled = true;
		float E0 = 1.0f;
		float falloffRadius = 100.0f;
		Vector4f position = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
		Quaternionf orientation;
		MultispectralSph4f msph;

		//vector<SSPHHLightHierarchy> hierarchies;

		std::vector<Image4f> vizgenLightProbes;
		Sph4f self;
		Sph4f neighbor;

		std::string hier_description;

		bool dirty = true;
		bool randomize = false;
		bool randomizePosition = false;
		std::string name;
		int index = -1;
		int maxDegree = DefaultSphlDegree;

		// This holds a cubemap representation of the path traced (ptrc) 3D light probe
		Image4f ptrcLightProbeImage;
		SimpleGpuTexture ptrcLightProbeTexture = SimpleGpuTexture(GL_TEXTURE_CUBE_MAP);
		float ptrcLightProbePercent = 0.0f;

		// This holds a cube map representation of the msph for this SPHL
		Image4f msphLightProbeImage;
		SimpleGpuTexture msphLightProbeTexture = SimpleGpuTexture(GL_TEXTURE_CUBE_MAP);

		// This holds the finalized SELF + NEIGHBOR light probe
		//SSPHHLightHierarchy hierSph;
		Image4f hierLightProbeImage;
		SimpleGpuTexture hierLightProbeTexture = SimpleGpuTexture(GL_TEXTURE_CUBE_MAP);

		SimpleCubeTexture depthSphlMap;
		SimpleCubeTexture colorSphlMap;
		SimpleCubeTexture environmentLightProbeMap;

		SimpleSSPHHLight();
		~SimpleSSPHHLight();

		// Reads from a Path Traced Light Probe (a cube map stored images from left to right in a single image).
		bool ReadPtrcLightProbe(const std::string& path);
		// Saves to a Path Traced Light Probe (a cube map with cube faces stored from left to right in a single image).
		bool SavePtrcLightProbe(const std::string& path);

		// Saves a JSON form of the multispectral (RGBL) of this SPH. L represents a monochromatic version of the RGB components. { maxDegree: (1-10), coefs : [] }
		bool SaveJsonSph(const std::string& path);
		// Reads a JSON format of a multispectral (RGBL) of this SPH. L represents a monochromatic version of the RGB components. { maxDegree: (1-10), coefs : [] }
		bool ReadJsonSph(const std::string& path);

		// Writes name.obj and name.mtl to the path
		bool SaveOBJ(const std::string& path, const std::string& name);

		bool LightProbeToSph(const Image4f& lightProbe, MultispectralSph4f& sph);
		bool SphToLightProbe(const MultispectralSph4f& sph, Image4f& lightProbe);
		bool SphToLightProbe(const MultispectralSph4f& sph, Image4f& lightProbe, int maxDegree);
		bool UploadLightProbe(Image4f& lightProbe, SimpleGpuTexture& texture);
		bool UploadLightProbe(Image4f& lightProbe, GLuint& texture);

		Color3f GetCoefficientColor(int l, int m) const;
		float GetCoefficient(int j, int l, int m) const;
		float GetCoefficient(int j, int lm) const;
		void ZeroCoefficients();
		void IsolateCoefficient(int j, int l, int m);
		void SetCoefficient(int j, int l, int m, float value);
		void RandomizeCoefficient(int j, int l, int m, float minValue, float maxValue);
		inline void Standardize() { MakeIntensityChannel4f(msph); }
		void Randomize(float size = 1.0f);

		void ChangeDegrees(int degrees);
		inline void IncDegrees() { ChangeDegrees(maxDegree + 1); }
		inline void DecDegrees() { ChangeDegrees(maxDegree - 1); }

		void ResizeHierarchies(int size);

		void MakeDirty();

		inline bool IsDirty() { return dirty; }
		inline int GetMaxCoefficients() const { return maxDegree * (maxDegree + 1) + maxDegree + 1; }
		inline int GetMaxCoefficients(int degree) { return degree * (degree + 1) + degree + 1; }
		inline void MakeLuminanceChannel() { MakeLuminanceChannel4f(msph); }
		inline void MakeIntensityChannel() { MakeIntensityChannel4f(msph); }

		// Hierarchies
		void SortHierarchiesByPercent();
		void SortHierarchiesByIndex();
		void CopySphlToHierarchies();

		void SetHierarchyDescriptionToIndex();
		void SetHierarchyDescriptionToPercent();
		void SetHierarchyDescription();
	};
}

#endif