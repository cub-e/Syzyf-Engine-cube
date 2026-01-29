#pragma once

#include <GameObject.h>

#include <Texture.h>

class ReflectionProbeSystem;
class Material;

class ReflectionProbe : public GameObject {
	friend class ReflectionProbeSystem;
private:
	static constexpr unsigned int resolution = 256;

	bool dirty;
	Cubemap* irradianceMap;
	Cubemap* prefilterMap;

	Material* gizmoMaterial;
public:
	ReflectionProbe();

	void Regenerate();

	Cubemap* GetIrradianceMap();
	Cubemap* GetPrefilterMap();

	void DrawGizmos();
};