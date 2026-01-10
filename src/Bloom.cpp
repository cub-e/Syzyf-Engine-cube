#include <Bloom.h>

#include <Resources.h>

constexpr int BLOOM_LEVEL = 6;

void Bloom::UpdateTexture() {
	glBindTexture(GL_TEXTURE_2D, this->bloomTexture);

	glm::uvec2 resolution = this->savedResolution;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, nullptr);

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

Bloom::Bloom() {
	this->savedResolution = GetScene()->GetGraphics()->GetScreenResolution();

	glGenTextures(1, &this->bloomTexture);

	glBindTexture(GL_TEXTURE_2D, this->bloomTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	UpdateTexture();

	this->downsampleShader = new ComputeShaderProgram(Resources::Get<ComputeShader>("./res/shaders/bloom/bloom_downsample.comp"));
	this->upsampleShader = new ComputeShaderProgram(Resources::Get<ComputeShader>("./res/shaders/bloom/bloom_upsample.comp"));
	this->finalShader = new ComputeShaderProgram(Resources::Get<ComputeShader>("./res/shaders/bloom/bloom_final.comp"));
}

void Bloom::OnPostProcess(const PostProcessParams* params) {
	if (this->savedResolution != GetScene()->GetGraphics()->GetScreenResolution()) {
		this->savedResolution = GetScene()->GetGraphics()->GetScreenResolution();

		UpdateTexture();
	}

	glm::uvec2 resolution = glm::ceil(this->savedResolution / 2.0f);

	glUseProgram(this->downsampleShader->GetHandle());

	for (int i = 0; i < BLOOM_LEVEL - 1; i++) {
		if (i == 0) {
			glBindImageTexture(0, params->inputTexture->GetHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
		}
		else {
			glBindImageTexture(0, this->bloomTexture, i, false, 0, GL_READ_WRITE, GL_RGBA32F);
		}
		glBindImageTexture(1, this->bloomTexture, i + 1, false, 0, GL_READ_WRITE, GL_RGBA32F);

		glUniform1i(glGetUniformLocation(this->downsampleShader->GetHandle(), "imgInput"), 0);
		glUniform1i(glGetUniformLocation(this->downsampleShader->GetHandle(), "imgOutput"), 1);

		glDispatchCompute(std::ceil(float(resolution.x) / 8), std::ceil(float(resolution.y) / 8), 1);

		resolution = glm::ceil(glm::vec2(resolution) * 0.5f);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	resolution = resolution * 4u;

	glUseProgram(this->upsampleShader->GetHandle());

	for (int i = BLOOM_LEVEL - 1; i >= 1; i--) {
		glBindImageTexture(0, this->bloomTexture, i, false, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(1, this->bloomTexture, i - 1, false, 0, GL_READ_WRITE, GL_RGBA32F);

		glUniform1i(glGetUniformLocation(this->upsampleShader->GetHandle(), "imgInput"), 0);
		glUniform1i(glGetUniformLocation(this->upsampleShader->GetHandle(), "imgOutput"), 1);

		glDispatchCompute(std::ceil(float(resolution.x) / 8), std::ceil(float(resolution.y) / 8), 1);

		resolution = resolution * 2u;

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	glUseProgram(this->finalShader->GetHandle());

	glBindImageTexture(0, params->inputTexture->GetHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, this->bloomTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, params->outputTexture->GetHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

	glDispatchCompute(std::ceil(savedResolution.x / 8), std::ceil(savedResolution.y / 8), 1);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}