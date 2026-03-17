#pragma once

#include <GameObject.h>

#include <Texture.h>
#include <Debug.h>

class ReflectionProbeSystem;
class Material;
class Mesh;

class ReflectionProbe : public GameObject, public ImGuiDrawable {
	friend class ReflectionProbeSystem;
private:
  ResourceRef<Mesh> cubemapGizmoMesh;

	static constexpr unsigned int resolution = 256;

	bool dirty;
  std::shared_ptr<Cubemap> irradianceMap;
  std::shared_ptr<Cubemap> prefilterMap;

  std::shared_ptr<Material> gizmoMaterial;
public:
	ReflectionProbe();

	void Regenerate();

	Cubemap* GetIrradianceMap();
	Cubemap* GetPrefilterMap();

	void DrawGizmos();

	virtual void DrawImGui();
};
