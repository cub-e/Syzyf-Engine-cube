#pragma once

#include <GameObjectSystem.h>
#include <ReflectionProbe.h>

class ReflectionProbeSystem : public GameObjectSystem<ReflectionProbe> {
private:
	GLuint reflectionProbeFramebuffer;
	GLuint reflectionProbeDepthTexture;
public:
	ReflectionProbeSystem(Scene* scene);

	ReflectionProbe* GetClosest(glm::vec3 position);

	virtual void OnPostRender();

	virtual int Priority();
};