#pragma once

#include <glad/glad.h>

#include <GameObject.h>
#include <Mesh.h>
#include <Material.h>

class Skybox : public GameObject {
private:
	ResourceRef<Mesh> skyMesh;
  std::shared_ptr<Material> skyMaterial;

	static Skybox* currentSkybox;
public:
	Skybox(std::shared_ptr<Material> skyMaterial);

	Material* GetSkyMaterial();
	Mesh* GetSkyMesh();

	static Skybox* GetCurrentSkybox();
	void SetAsCurrentSkybox();
};
