#pragma once

#include "SceneComponent.h"

#include <vector>

class Tween;

class TweenSystem : public SceneComponent {
private:
  std::vector<Tween> tweens;
  // Without this if a callback in a tween adds another tween it might accidentally get removed or because of the cleanup in OnPreUpdate()
  std::vector<Tween> pendingTweens;
public:
  TweenSystem(Scene* scene);

  void OnPreUpdate();
  void DrawImGui();

  // return handles instead
  Tween& CreateTween(const float initialValue, const float targetValue, const float duration);
};
