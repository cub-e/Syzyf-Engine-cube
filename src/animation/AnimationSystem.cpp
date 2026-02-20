#include "animation/AnimationSystem.h"
#include "animation/AnimationComponent.h"

#include "Scene.h"

AnimationSystem::AnimationSystem(Scene* scene) : SceneComponent(scene) {
  spdlog::info("Animation system added");
}

void AnimationSystem::OnPreUpdate() {
  auto objects = GetScene()->FindObjectsOfType<AnimationComponent>();
  for (auto* object : objects) {
  }
};
