#include "animation/AnimationSystem.h"
#include "animation/AnimationComponent.h"

#include "Scene.h"
#include <algorithm>

AnimationSystem::AnimationSystem(Scene* scene) : SceneComponent(scene) {
  spdlog::info("Animation system added");
}

void AnimationSystem::OnPreUpdate() {
  const float deltaTime = GetScene()->DeltaTime();

  spdlog::info("AnimationSystem: delta: {}", deltaTime);

  auto objects = GetScene()->FindObjectsOfType<AnimationComponent>();
  for (auto* object : objects) {
    for (auto& animation : object->animations) {
      if (!animation.playing)
        continue;

      animation.timeActive += deltaTime;

      spdlog::info("Animation: {}, duration: {}, timeactive: {}", animation.name, animation.duration, animation.timeActive);
      if (animation.timeActive >= animation.duration) {
        spdlog::info("Animation over");
        // set to max value
        // animation.playing = false;
        // add looping
        animation.timeActive = 0.0f;
        continue;
      }

      for (std::size_t i = 0; i < animation.tracks.size(); ++i) {
        auto& track = animation.tracks[i];

        float next; 
        auto upper = std::upper_bound(track.inputs.begin(), track.inputs.end(), animation.timeActive);
        std::size_t upperIndex = std::distance(track.inputs.begin(), upper);
        std::size_t lowerIndex = upperIndex - 1;
        // shouldnt be .end() because of the previous check?

        switch (track.property) {
          case AnimationComponent::Property::ROTATION:
            {
              spdlog::info("Updating rotation");
              glm::quat rotation = {
                track.outputs[upperIndex * 4 + 3],
                track.outputs[upperIndex * 4],
                track.outputs[upperIndex * 4 + 1],
                track.outputs[upperIndex * 4 + 2],
              };
              // maybe check if its valid idk
              track.target->LocalTransform().Rotation() = rotation;
              break;
            }
          case AnimationComponent::Property::POSITION:
            {
              spdlog::info("Updating position");
              glm::vec3 position = {
                track.outputs[upperIndex * 3],
                track.outputs[upperIndex * 3 + 1],
                track.outputs[upperIndex * 3 + 2],
              };
              track.target->LocalTransform().Position() = position;
              break;
            }
          case AnimationComponent::Property::SCALE:
            {
              track.target->LocalTransform().Scale() = glm::vec3 {
                track.outputs[upperIndex * 3],
                track.outputs[upperIndex * 3 + 1],
                track.outputs[upperIndex * 3 + 2],
              };
            }
          case AnimationComponent::Property::WEIGHTS:
            break;
        }
      }
    }
  }
};
