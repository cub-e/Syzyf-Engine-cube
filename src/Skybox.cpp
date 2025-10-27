#include <Skybox.h>

Mesh* Skybox::skyMesh = nullptr;
Skybox* Skybox::currentSkybox = nullptr;

Skybox::Skybox(Material* skyMaterial):
skyMaterial(skyMaterial) {
	if (!skyMesh) {
		skyMesh = new Mesh();
		*skyMesh = Mesh::Load("./res/models/sky.obj", VertexSpec::Mesh);
	}

	SetAsCurrentSkybox();
}

Material* Skybox::GetSKyMaterial() {
	return this->skyMaterial;
}
Mesh* Skybox::GetSkyMesh() {
	return skyMesh;
}

Skybox* Skybox::GetCurrentSkybox() {
	return currentSkybox;
}

void Skybox::SetAsCurrentSkybox() {
	currentSkybox = this;
}
