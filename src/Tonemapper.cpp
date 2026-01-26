#include <Tonemapper.h>

#include <GLFW/glfw3.h>

#include <Resources.h>
#include <Shader.h>
#include <Material.h>
#include <Graphics.h>

Tonemapper::Tonemapper() {
	this->reinhardTonemapperShader = new ComputeShaderDispatch(Resources::Get<ComputeShader>("./res/shaders/tonemapping/reinhard_tonemapper.comp"));
	this->acesTonemapperShader = new ComputeShaderDispatch(Resources::Get<ComputeShader>("./res/shaders/tonemapping/aces_tonemapper.comp"));
	this->gtTonemapperShader = new ComputeShaderDispatch(Resources::Get<ComputeShader>("./res/shaders/tonemapping/gt_tonemapper.comp"));

	this->function = TonemapperFunction::None;
}

Tonemapper::TonemapperFunction Tonemapper::GetCurrentFunction() const {
	return this->function;
}

void Tonemapper::SetFunction(Tonemapper::TonemapperFunction func) {
	this->function = func;
}

void Tonemapper::OnPostProcess(const PostProcessParams* params) {
	ComputeShaderDispatch* shader = nullptr;

	switch (this->function) {
		case TonemapperFunction::Reinhard:
			shader = this->reinhardTonemapperShader;
			break;
		case TonemapperFunction::Aces:
			shader = this->acesTonemapperShader;
			break;
		case TonemapperFunction::GranTurismo:
			shader = this->gtTonemapperShader;
			break;
		case TonemapperFunction::None:
		default:
			break;
	}

	if (shader) {
		shader->GetData()->SetValue("inputTex", params->inputTexture);
		shader->GetData()->SetValue("outputTex", params->outputTexture);
	
		glm::vec2 res = GetScene()->GetGraphics()->GetScreenResolution();
	
		shader->Dispatch(std::ceil(res.x / 8), std::ceil(res.y / 8), 1);
	}
}