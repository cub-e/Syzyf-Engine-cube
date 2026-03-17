#include <ReflectionProbe.h>

#include <Texture.h>
#include <Resources.h>
#include <Mesh.h>
#include <Material.h>
#include <Graphics.h>

#include <imgui.h>

ReflectionProbe::ReflectionProbe():
dirty(true),
irradianceMap(nullptr),
prefilterMap(nullptr) {
  std::shared_ptr<ShaderProgram> cubemapGizmoShader = ShaderProgram::Build()
	.WithVertexShader(ResourceDatabase::Global->Get<VertexShader>("./res/shaders/lit.vert"))
	.WithPixelShader(ResourceDatabase::Global->Get<PixelShader>("./res/shaders/cubemap.frag"))
	.Link();
	
	this->gizmoMaterial = std::make_shared<Material>(cubemapGizmoShader);
}

void ReflectionProbe::Regenerate() {
	this->dirty = true;
}

Cubemap* ReflectionProbe::GetIrradianceMap() {
	return this->irradianceMap.get();
}
Cubemap* ReflectionProbe::GetPrefilterMap() {
	return this->prefilterMap.get();
}

void ReflectionProbe::DrawGizmos() {
	if (!cubemapGizmoMesh.IsValid()) {
		cubemapGizmoMesh = ResourceDatabase::Global->Get<Mesh>("./res/models/sphere.obj");
	}

	if (this->dirty) {
		return;
	}

	this->gizmoMaterial->SetValue("cubemap", this->prefilterMap.get());

	GetScene()->GetGraphics()->DrawGizmoMesh(cubemapGizmoMesh, 0, this->gizmoMaterial, GlobalTransform().Value());
}

void ReflectionProbe::DrawImGui() {
	ImGui::Text("Status: %s", this->dirty ? "Dirty" : "Clean");

	if (ImGui::Button("Recalculate")) {
		this->Regenerate();
	}
}
