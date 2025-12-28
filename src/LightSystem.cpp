#include <LightSystem.h>

#include <Light.h>

LightSystem::LightSystem(Scene* scene):
GameObjectSystem<Light>(scene) { }