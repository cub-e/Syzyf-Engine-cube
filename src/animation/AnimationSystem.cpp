#include "animation/AnimationSystem.h"
#include "animation/AnimationComponent.h"

#include "Scene.h"
#include <algorithm>
#include <glm/ext/quaternion_common.hpp>

AnimationSystem::AnimationSystem(Scene* scene) : SceneComponent(scene) {
  spdlog::info("Animation system added");
}

void AnimationSystem::OnPreUpdate() {
  const float deltaTime = GetScene()->DeltaTime();

  auto objects = GetScene()->FindObjectsOfType<AnimationComponent>();
  for (auto* object : objects) {
    for (auto& animation : object->animations) {
      if (!animation.playing)
        continue;

      animation.timeActive += deltaTime;

      spdlog::info("Animation: {}, duration: {}, timeactive: {}", animation.data.name, animation.data.duration, animation.timeActive);
      if (animation.timeActive >= animation.data.duration) {
        spdlog::info("Animation over");
        if (animation.looping) {
          animation.timeActive = 0.0f;
        } else {
          animation.playing = false;
          animation.timeActive = animation.data.duration;
        }
      }

      for (std::size_t i = 0; i < animation.data.tracks.size(); ++i) {
        auto& track = animation.data.tracks[i];

        auto upper = std::upper_bound(track.inputs.begin(), track.inputs.end(), animation.timeActive);
        std::size_t upperIndex = std::distance(track.inputs.begin(), upper);
        std::size_t lowerIndex = 0;

        // Because of how this is done this will interpolate the values for no reason sometimes but maybe it doesn't matter
        // Checks if the track hasn't started yet and sets the values to that of the first keyframe
        if (upperIndex == 0) {
          lowerIndex = 0;
        // Checks if the track ended, set's the value to the last keyframe
        } else if (upper == track.inputs.end()) {
          upperIndex = track.inputs.size() - 1;
          lowerIndex = track.inputs.size() - 1;
        } else {
          // Animation plays normally
          lowerIndex = upperIndex - 1;
        }

        const float keyFrameDuration = track.inputs[upperIndex] - track.inputs[lowerIndex];
        const float keyFrameTimeActive = animation.timeActive - track.inputs[lowerIndex];
        const float interpolationValue = keyFrameTimeActive / keyFrameDuration;

        switch (track.property) {
          case AnimationComponent::Property::ROTATION:
            {
              glm::quat rotation;

              glm::quat lower = {
                track.outputs[lowerIndex * 4 + 3],
                track.outputs[lowerIndex * 4],
                track.outputs[lowerIndex * 4 + 1],
                track.outputs[lowerIndex * 4 + 2],
              };

              // Skips interpolation if the track hasn't started yet/ended 
              if (upperIndex == lowerIndex) {
                rotation = lower;
                track.target->LocalTransform().Rotation() = rotation;
                break;
              }

              glm::quat upper = {
                track.outputs[upperIndex * 4 + 3],
                track.outputs[upperIndex * 4],
                track.outputs[upperIndex * 4 + 1],
                track.outputs[upperIndex * 4 + 2],
              };
 
              switch (track.interpolation) {
                case AnimationComponent::Interpolation::LINEAR:
                  rotation = glm::slerp(lower, upper, interpolationValue);
                  break;
                case AnimationComponent::Interpolation::STEP:
                  rotation = lower;
                  break;
                case AnimationComponent::Interpolation::CUBICSPLINE:
                  // TODO
                  rotation = lower;
                  break;
                }
              track.target->LocalTransform().Rotation() = rotation;
              break;
            }
          case AnimationComponent::Property::POSITION:
            {
              glm::vec3 lower = {
                track.outputs[lowerIndex * 3],
                track.outputs[lowerIndex * 3 + 1],
                track.outputs[lowerIndex * 3 + 2],
              };

              if (upperIndex == lowerIndex) {
                track.target->LocalTransform().Position() = lower;
                break;
              }

              glm::vec3 upper = {
                track.outputs[upperIndex * 3],
                track.outputs[upperIndex * 3 + 1],
                track.outputs[upperIndex * 3 + 2],
              };

              glm::vec3 position = glm::mix(lower, upper, interpolationValue);

              track.target->LocalTransform().Position() = position;
              break;
            }
          case AnimationComponent::Property::SCALE:
            {
              glm::vec3 lower = {
                track.outputs[lowerIndex * 3],
                track.outputs[lowerIndex * 3 + 1],
                track.outputs[lowerIndex * 3 + 2],
              };

              if (upperIndex == lowerIndex) {
                track.target->LocalTransform().Scale() = lower;
                break;
              }

              glm::vec3 upper = {
                track.outputs[upperIndex * 3],
                track.outputs[upperIndex * 3 + 1],
                track.outputs[upperIndex * 3 + 2],
              };

              glm::vec3 scale = glm::mix(lower, upper, interpolationValue);

              track.target->LocalTransform().Position() = scale;
              break;
            }
          case AnimationComponent::Property::WEIGHTS:
            // TODO
            break;
        }
      }
    }
  }
};
