#include <Skybox.h>

#include <Resources.h>

Skybox* Skybox::currentSkybox = nullptr;

Skybox::Skybox(std::shared_ptr<Material> skyMaterial):
skyMaterial(skyMaterial) {
	if (!skyMesh.IsValid()) {
		skyMesh = Resources::Global->Get<Mesh>("./res/models/sky.obj");
	}

	SetAsCurrentSkybox();
}

Material* Skybox::GetSkyMaterial() {
	return this->skyMaterial.get();
}
Mesh* Skybox::GetSkyMesh() {
	return skyMesh.Get();
}

Skybox* Skybox::GetCurrentSkybox() {
	return currentSkybox;
}

void Skybox::SetAsCurrentSkybox() {
	currentSkybox = this;
}
