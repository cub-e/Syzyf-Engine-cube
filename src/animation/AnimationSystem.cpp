#include "animation/AnimationSystem.h"
#include "animation/AnimationComponent.h"

#include "Scene.h"
#include <algorithm>
#include <glm/ext/quaternion_common.hpp>

glm::quat cubicSpline(const glm::quat previousPoint, const glm::quat previousTangent, const glm::quat nextPoint, const glm::quat nextTangent, const float interpolationValue) {
  const float t = interpolationValue;
  const float t2 = t * t;
  const float t3 = t2 * t;

  return (2.0f * t3 - 3.0f * t2 + 1.0f)
    * previousPoint + (t3 - 2.0f * t2 + t)
    * previousTangent + (-2.0f * t3 + 3.0f * t2)
    * nextPoint + (t3 - t2) * nextTangent;
}

glm::vec3 cubicSpline(const glm::vec3 previousPoint, const glm::vec3 previousTangent, const glm::vec3 nextPoint, const glm::vec3 nextTangent, const float interpolationValue) {
  const float t = interpolationValue;
  const float t2 = t * t;
  const float t3 = t2 * t;

  return (2.0f * t3 - 3.0f * t2 + 1.0f)
    * previousPoint + (t3 - 2.0f * t2 + t)
    * previousTangent + (-2.0f * t3 + 3.0f * t2)
    * nextPoint + (t3 - t2) * nextTangent;
}

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

              // Skips interpolation if the track hasn't started yet/ended already 
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
                  glm::quat lowerValue = {
                    track.outputs[lowerIndex * 4 + 7],
                    track.outputs[lowerIndex * 4 + 4],
                    track.outputs[lowerIndex * 4 + 5],
                    track.outputs[lowerIndex * 4 + 6],
                  };
                  glm::quat lowerOutputTangent = {
                    track.outputs[lowerIndex * 4 + 11],
                    track.outputs[lowerIndex * 4 + 8],
                    track.outputs[lowerIndex * 4 + 9],
                    track.outputs[lowerIndex * 4 + 10],
                  };

                  glm::quat upperInputTangent = upper;
                  glm::quat upperValue = {
                    track.outputs[upperIndex * 4 + 7],
                    track.outputs[upperIndex * 4 + 4],
                    track.outputs[upperIndex * 4 + 5],
                    track.outputs[upperIndex * 4 + 6],
                  };

                  glm::quat previousTangent = deltaTime * lowerOutputTangent;
                  glm::quat nextTangent = deltaTime * upperInputTangent;

                  rotation = cubicSpline(lowerValue, previousTangent, upperValue, nextTangent, interpolationValue);
                  break;
                }
              track.target->LocalTransform().Rotation() = rotation;
              break;
            }
          case AnimationComponent::Property::POSITION:
            {
              glm::vec3 position;

              glm::vec3 lower = {
                track.outputs[lowerIndex * 3],
                track.outputs[lowerIndex * 3 + 1],
                track.outputs[lowerIndex * 3 + 2],
              };

              if (upperIndex == lowerIndex) {
                position = lower;
                track.target->LocalTransform().Position() = position;
                break;
              }

              glm::vec3 upper = {
                track.outputs[upperIndex * 3],
                track.outputs[upperIndex * 3 + 1],
                track.outputs[upperIndex * 3 + 2],
              };
 
              switch (track.interpolation) {
                case AnimationComponent::Interpolation::LINEAR:
                  position = glm::mix(lower, upper, interpolationValue);
                  break;
                case AnimationComponent::Interpolation::STEP:
                  position = lower;
                  break;
                case AnimationComponent::Interpolation::CUBICSPLINE:
                  glm::vec3 lowerValue = {
                    track.outputs[lowerIndex * 3 + 3],
                    track.outputs[lowerIndex * 3 + 4],
                    track.outputs[lowerIndex * 3 + 5],
                  };
                  glm::vec3 lowerOutputTangent = {
                    track.outputs[lowerIndex * 3 + 6],
                    track.outputs[lowerIndex * 3 + 7],
                    track.outputs[lowerIndex * 3 + 8],
                  };

                  glm::vec3 upperInputTangent = upper;
                  glm::vec3 upperValue = {
                    track.outputs[upperIndex * 4 + 3],
                    track.outputs[upperIndex * 4 + 4],
                    track.outputs[upperIndex * 4 + 5],
                  };

                  glm::vec3 previousTangent = deltaTime * lowerOutputTangent;
                  glm::vec3 nextTangent = deltaTime * upperInputTangent;

                  position = cubicSpline(lowerValue, previousTangent, upperValue, nextTangent, interpolationValue);
                  break;
                }
              track.target->LocalTransform().Position() = position;
              break;
            }
          case AnimationComponent::Property::SCALE:
            {
              glm::vec3 scale;

              glm::vec3 lower = {
                track.outputs[lowerIndex * 3],
                track.outputs[lowerIndex * 3 + 1],
                track.outputs[lowerIndex * 3 + 2],
              };

              if (upperIndex == lowerIndex) {
                scale = lower;
                track.target->LocalTransform().Scale() = scale;
                break;
              }

              glm::vec3 upper = {
                track.outputs[upperIndex * 3],
                track.outputs[upperIndex * 3 + 1],
                track.outputs[upperIndex * 3 + 2],
              };
 
              switch (track.interpolation) {
                case AnimationComponent::Interpolation::LINEAR:
                  scale = glm::mix(lower, upper, interpolationValue);
                  break;
                case AnimationComponent::Interpolation::STEP:
                  scale = lower;
                  break;
                case AnimationComponent::Interpolation::CUBICSPLINE:
                  glm::vec3 lowerValue = {
                    track.outputs[lowerIndex * 3 + 3],
                    track.outputs[lowerIndex * 3 + 4],
                    track.outputs[lowerIndex * 3 + 5],
                  };
                  glm::vec3 lowerOutputTangent = {
                    track.outputs[lowerIndex * 3 + 6],
                    track.outputs[lowerIndex * 3 + 7],
                    track.outputs[lowerIndex * 3 + 8],
                  };

                  glm::vec3 upperInputTangent = upper;
                  glm::vec3 upperValue = {
                    track.outputs[upperIndex * 4 + 3],
                    track.outputs[upperIndex * 4 + 4],
                    track.outputs[upperIndex * 4 + 5],
                  };

                  glm::vec3 previousTangent = deltaTime * lowerOutputTangent;
                  glm::vec3 nextTangent = deltaTime * upperInputTangent;

                  scale = cubicSpline(lowerValue, previousTangent, upperValue, nextTangent, interpolationValue);
                  break;
                }
              track.target->LocalTransform().Scale() = scale;
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
