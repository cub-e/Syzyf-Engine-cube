#include <ReflectionProbeSystem.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <ReflectionProbe.h>

#include "../res/shaders/shared/shared.h"
#include "../res/shaders/shared/uniforms.h"

ReflectionProbeSystem::ReflectionProbeSystem(Scene* scene):
GameObjectSystem<ReflectionProbe>(scene) {
	glCreateFramebuffers(1, &this->reflectionProbeFramebuffer);
	glGenTextures(1, &this->reflectionProbeDepthTexture);
	glTextureParameteri(this->reflectionProbeDepthTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(this->reflectionProbeDepthTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(this->reflectionProbeDepthTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(this->reflectionProbeDepthTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, this->reflectionProbeDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 512, 512, 0,  GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, this->reflectionProbeFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->reflectionProbeDepthTexture, 0);
}

void ReflectionProbeSystem::OnPostRender() {
	const glm::vec3 directions[] {
		{ 1,  0,  0},
		{-1,  0,  0},
		{ 0,  1,  0},
		{ 0, -1,  0},
		{ 0,  0,  1},
		{ 0,  0, -1}
	};

	for (ReflectionProbe* probe : *GetAllObjects()) {
		if (!probe->dirty) {
			continue;
		}

		probe->dirty = false;

		// return;

		glBindFramebuffer(GL_FRAMEBUFFER, this->reflectionProbeFramebuffer);
		
		ShaderGlobalUniforms globalUniforms;
		
		globalUniforms.Global_CameraWorldPos = probe->GlobalTransform().Position();
		globalUniforms.Global_Time = (float) glfwGetTime();
		globalUniforms.Global_CameraFarPlane = 0;
		globalUniforms.Global_CameraNearPlane = 0;
		globalUniforms.Global_CameraFov = glm::radians(90.0f);

		for (int face = 0; face < 6; face++) {
			globalUniforms.Global_ViewMatrix = glm::lookAt(
				probe->GlobalTransform().Position().Value(),
				probe->GlobalTransform().Position() + directions[face],
				glm::vec3(0, 1, 0)
			);
			globalUniforms.Global_ProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
			globalUniforms.Global_VPMatrix = globalUniforms.Global_ProjectionMatrix * globalUniforms.Global_ViewMatrix;

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, probe->cubemap->GetHandle(), 0);
			glClear(GL_DEPTH_BUFFER_BIT);

			GetScene()->GetGraphics()->BindGlobalUniformBuffer(globalUniforms);
			
			glViewport(0, 0, 512, 512);
			
			GetScene()->GetGraphics()->RenderObjects(globalUniforms, SceneGraphics::PassType::Color);
		}
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	}
}

int ReflectionProbeSystem::Priority() {
	return 50;
}