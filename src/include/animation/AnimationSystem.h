#pragma once

#include "SceneComponent.h"

class AnimationSystem : public SceneComponent {
public:
  AnimationSystem(Scene* scene);
  virtual ~AnimationSystem() = default;

  virtual void OnPreUpdate();
};
