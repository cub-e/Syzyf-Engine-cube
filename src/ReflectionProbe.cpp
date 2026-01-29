#include <ReflectionProbe.h>

#include <Texture.h>
#include <Resources.h>
#include <Mesh.h>
#include <Material.h>
#include <Graphics.h>

ReflectionProbe::ReflectionProbe():
dirty(true),
irradianceMap(nullptr),
prefilterMap(nullptr) {
	ShaderProgram* cubemapGizmoShader = ShaderProgram::Build()
	.WithVertexShader(Resources::Get<VertexShader>("./res/shaders/lit.vert"))
	.WithPixelShader(Resources::Get<PixelShader>("./res/shaders/cubemap.frag"))
	.Link();
	
	this->gizmoMaterial = new Material(cubemapGizmoShader);
}

void ReflectionProbe::Regenerate() {
	this->dirty = true;
}

Mesh* cubemapGizmoMesh = nullptr;

Cubemap* ReflectionProbe::GetIrradianceMap() {
	return this->irradianceMap;
}
Cubemap* ReflectionProbe::GetPrefilterMap() {
	return this->prefilterMap;
}

void ReflectionProbe::DrawGizmos() {
	if (cubemapGizmoMesh == nullptr) {
		cubemapGizmoMesh = Resources::Get<Mesh>("./res/models/sphere.obj");
	}

	if (this->dirty) {
		return;
	}

	this->gizmoMaterial->SetValue("cubemap", this->prefilterMap);

	GetScene()->GetGraphics()->DrawGizmoMesh(cubemapGizmoMesh, 0, this->gizmoMaterial, GlobalTransform().Value());
}