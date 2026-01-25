#include <GameObjectSystem.h>

GameObjectSystemBase::GameObjectSystemBase(Scene* scene):
SceneComponent(scene) { }

int GameObjectSystemBase::Priority() {
	return 0;
}
