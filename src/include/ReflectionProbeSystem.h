#pragma once

#include <GameObjectSystem.h>
#include <ReflectionProbe.h>
#include <Framebuffer.h>
#include <Texture.h>

class ReflectionProbeSystem : public GameObjectSystem<ReflectionProbe> {
private:
	Framebuffer* reflectionProbeFramebuffer;
	Texture2D* reflectionProbeDepthTexture;

	Texture2D* brdfConvolutionMap;
public:
	ReflectionProbeSystem(Scene* scene);

	ReflectionProbe* GetClosest(glm::vec3 position);

	Texture2D* BRDFConvolutionMap();

	virtual void OnPostRender();

	virtual int Order();
};