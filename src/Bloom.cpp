#include <Bloom.h>

#include <Resources.h>

constexpr int BLOOM_LEVEL = 6;

void Bloom::UpdateTexture() {
	glm::uvec2 resolution = this->savedResolution;

	if (resolution.x <= 0 || resolution.y <= 0) {
		return;
	}

	glTextureStorage2D(this->bloomTexture, BLOOM_LEVEL, GL_RGBA32F, resolution.x, resolution.y);
}

Bloom::Bloom() {
	this->savedResolution = GetScene()->GetGraphics()->GetScreenResolution();

	glCreateTextures(GL_TEXTURE_2D, 1, &this->bloomTexture);
	
	UpdateTexture();
	glTextureParameteri(this->bloomTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTextureParameteri(this->bloomTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(this->bloomTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(this->bloomTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	this->downsampleShader = new ComputeShaderProgram(Resources::Get<ComputeShader>("./res/shaders/bloom/bloom_downsample.comp"));
	this->upsampleShader = new ComputeShaderProgram(Resources::Get<ComputeShader>("./res/shaders/bloom/bloom_upsample.comp"));
	this->finalShader = new ComputeShaderProgram(Resources::Get<ComputeShader>("./res/shaders/bloom/bloom_final.comp"));
}

void Bloom::OnPostProcess(const PostProcessParams* params) {
	// TODO: Kick out
	const float m_threshold = 1.5f;
	const float m_knee = 0.1f;
	const float m_bloom_intensity = 1.0f;

	if (this->savedResolution != GetScene()->GetGraphics()->GetScreenResolution()) {
		this->savedResolution = GetScene()->GetGraphics()->GetScreenResolution();

		UpdateTexture();
	}

	glm::uvec2 resolution = glm::ceil(this->savedResolution / 2.0f);

	glUseProgram(this->downsampleShader->GetHandle());

	glm::vec4 tresholdVec = glm::vec4(m_threshold, m_threshold - m_knee, 2.0f * m_knee, 0.25f * m_knee);

	glUniform4fv(glGetUniformLocation(this->downsampleShader->GetHandle(), "treshold"), 1, &tresholdVec[0]);

	glBindTextureUnit(0, params->inputTexture->GetHandle());

	for (int i = 0; i < BLOOM_LEVEL - 1; i++) {
		if (i == 1) {
			glBindTextureUnit(0, this->bloomTexture);
		}

		glBindImageTexture(0, this->bloomTexture, i + 1, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
		
		glm::vec2 texelSize = 1.0f / glm::vec2(resolution);
		glUniform2fv(glGetUniformLocation(this->downsampleShader->GetHandle(), "texelSize"), 1, &texelSize[0]);
		glUniform1i(glGetUniformLocation(this->downsampleShader->GetHandle(), "mipLevel"), i);
		glUniform1i(glGetUniformLocation(this->downsampleShader->GetHandle(), "useTreshold"), i == 0);

		glDispatchCompute(std::ceil(float(resolution.x) / 8), std::ceil(float(resolution.y) / 8), 1);

		resolution = resolution / 2u;

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	glUseProgram(this->upsampleShader->GetHandle());

	glUniform1f(glGetUniformLocation(this->upsampleShader->GetHandle(), "bloomIntensity"), m_bloom_intensity);

	for (int i = BLOOM_LEVEL - 1; i >= 1; i--) {
		if (i == 1) {
			glBindImageTexture(0, params->outputTexture->GetHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
		}
		else {
			glBindImageTexture(0, this->bloomTexture, i - 1, false, 0, GL_READ_WRITE, GL_RGBA32F);
		}

		resolution.x = glm::max(1.0, glm::floor(float(savedResolution.x)  / glm::pow(2.0, i - 1)));
        resolution.y = glm::max(1.0, glm::floor(float(savedResolution.y) / glm::pow(2.0, i - 1)));

		glm::vec2 texelSize = 1.0f / glm::vec2(resolution);
		glUniform2fv(glGetUniformLocation(this->upsampleShader->GetHandle(), "texelSize"), 1, &texelSize[0]);
		glUniform1i(glGetUniformLocation(this->upsampleShader->GetHandle(), "mipLevel"), i);

		glDispatchCompute(std::ceil(float(resolution.x) / 8), std::ceil(float(resolution.y) / 8), 1);

		resolution = resolution * 2u;

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	// glUseProgram(this->finalShader->GetHandle());

	// glBindImageTexture(0, params->inputTexture->GetHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	// glBindImageTexture(1, this->bloomTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	// glBindImageTexture(2, params->outputTexture->GetHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

	// glDispatchCompute(std::ceil(savedResolution.x / 8), std::ceil(savedResolution.y / 8), 1);

	// glMemoryBarrier(GL_ALL_BARRIER_BITS);
}