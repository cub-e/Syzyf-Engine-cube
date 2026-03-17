#include <MeshRenderer.h>

#include <glad/glad.h>
#include <Scene.h>
#include <Graphics.h>

MeshRenderer::MeshRenderer():
mesh(),
materials(0) { }

MeshRenderer::MeshRenderer(ResourceRef<Mesh> mesh, std::shared_ptr<Material> material):
materials() {
	SetMesh(mesh);
	SetMaterial(material);
}

MeshRenderer::MeshRenderer(ResourceRef<Mesh> mesh, const std::vector<std::shared_ptr<Material>>& materials):
materials(materials) {
	SetMesh(mesh);
}

Mesh* MeshRenderer::GetMesh() {
	return this->mesh.Get();
}

void MeshRenderer::SetMesh(ResourceRef<Mesh> newMesh) {
	this->mesh = newMesh;
	
	if (!this->mesh.IsValid()) {
		return;
	}

	std::vector<std::shared_ptr<Material>> newMaterials{newMesh->GetMaterialsCount()};
	int materialsToCopy = std::min(newMesh->GetMaterialsCount(), (unsigned int) this->materials.size());
	for (int i = 0; i < materialsToCopy; i++) {
		newMaterials[i] = this->materials[i];
	}

	this->materials = newMaterials;
}

Material* MeshRenderer::GetMaterial(int materialIndex) {
	if (materialIndex < 0 || this->mesh->GetMaterialsCount() <= materialIndex) {
		return nullptr;
	}

	return this->materials[materialIndex].get();
}

void MeshRenderer::SetMaterial(std::shared_ptr<Material> newMaterial, int materialIndex) {
	if (materialIndex < 0 || this->mesh->GetMaterialsCount() <= materialIndex) {
		return;
	}

	this->materials[materialIndex] = newMaterial;
}

void MeshRenderer::Render() const {
	this->GetScene()->GetGraphics()->DrawMesh(const_cast<MeshRenderer*>(this));
}
