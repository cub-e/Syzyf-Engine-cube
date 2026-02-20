#include "animation/AnimationComponent.h"

AnimationComponent::AnimationComponent() {
  spdlog::info("AnimationComponent created on: {}", GetNode()->GetName());
}
