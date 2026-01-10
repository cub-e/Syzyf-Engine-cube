#pragma once

#include <PostProcessEffect.h>
#include <Material.h>
#include <glad/glad.h>

class Bloom : public PostProcessEffect {
private:
	glm::vec2 savedResolution;
	GLuint bloomTexture;
	ComputeShaderProgram* downsampleShader;
	ComputeShaderProgram* upsampleShader;
	ComputeShaderProgram* finalShader;

	void UpdateTexture();
public:
	Bloom();

	virtual void OnPostProcess(const PostProcessParams* params);
};