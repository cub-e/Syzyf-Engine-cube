#pragma once

#include <vector>

#include <GameObject.h>
#include <Mesh.h>
#include <Material.h>

class MeshRenderer : public GameObject {
private:
	ResourceRef<Mesh> mesh;
	std::vector<std::shared_ptr<Material>> materials;

	void ResetUniformBuffer();
public:
	MeshRenderer();
	MeshRenderer(ResourceRef<Mesh> mesh, std::shared_ptr<Material> material);
	MeshRenderer(ResourceRef<Mesh> mesh, const std::vector<std::shared_ptr<Material>>& materials);

	Mesh* GetMesh();
	void SetMesh(ResourceRef<Mesh> newMesh);

	Material* GetMaterial(int materialIndex = 0);

	void SetMaterial(std::shared_ptr<Material> newMaterial, int materialIndex = 0);

	void Render() const;
};
