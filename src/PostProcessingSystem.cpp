#include <PostProcessingSystem.h>

PostProcessingSystem::PostProcessingSystem(Scene* scene):
GameObjectSystem<PostProcessEffect>(scene) {
	glGenTextures(1, &this->postProcessColorBuffer);
	glBindTexture(GL_TEXTURE_2D, this->postProcessColorBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void PostProcessingSystem::UpdateBufferResolution(glm::vec2 newResolution) {
	glBindTexture(GL_TEXTURE_2D, this->postProcessColorBuffer);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, newResolution.x, newResolution.y, 0,  GL_RGBA, GL_FLOAT, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint PostProcessingSystem::GetPostProcessBuffer() {
	return this->postProcessColorBuffer;
}