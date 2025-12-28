#pragma once

#include <GameObjectSystem.h>

#include <Light.h>

class LightSystem : public GameObjectSystem<Light> {
public:
	LightSystem(Scene* scene);
};