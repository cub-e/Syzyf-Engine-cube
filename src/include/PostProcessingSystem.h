#pragma once

#include <glad/glad.h>
#include <GameObjectSystem.h>
#include <PostProcessEffect.h>

class PostProcessingSystem : public GameObjectSystem<PostProcessEffect> {
private:
	GLuint postProcessColorBuffer;
public:
	PostProcessingSystem(Scene* scene);

	void UpdateBufferResolution(glm::vec2 newResolution);

	GLuint GetPostProcessBuffer();
};