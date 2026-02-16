#include "tweening/TweenSystem.h"
#include "tweening/Tween.h"
#include "Scene.h"

#include <cmath>

#include <spdlog/spdlog.h>

TweenSystem::TweenSystem(Scene* scene) : SceneComponent(scene) {}

// Move somewhere else
//  or not idk
float easeInOutBack(const float progress) {
  const float c1 = 1.70158f;
  const float c2 = c1 * 1.525f;

  if (progress < 0.5f) {
    return ((2.0f * progress) * (2.0f * progress) * ((c2 + 1.0f) * 2.0f * progress - c2)) * 0.5f;
  } else {
    return ((2.0f * progress - 2.0f) * (2.0f * progress - 2.0f) * ((c2 + 1.0f) * (progress * 2.0f - 2.0f) + c2) + 2.0f) * 0.5f;
  }
}
float easeInOutSine(const float progress) {
  return -(cos(M_PI * progress) - 1.0f) * 0.5f;
}

void TweenSystem::OnPreUpdate() {

  for (int i = tweens.size() - 1; i >= 0; i--) {
    Tween& tween = tweens[i];
    float deltaTime = GetScene()->DeltaTime();
    tween.timeActive += deltaTime;

    spdlog::info("TweenSystem: delta time: {}", deltaTime);
    spdlog::info("TweenSystem: time active: {}", tween.timeActive);

    if (tween.timeActive >= tween.duration) {
      for (auto& value : tween.values) {
        *value = tween.targetValue;
      }

      tween.onComplete();

      std::swap(tween, tweens.back());
      tweens.pop_back();
      spdlog::info("Tween system: Tween removed");
      continue;
    }

    for (auto& value : tween.values) {
      float difference = tween.targetValue - tween.initialValue;
      float progress = tween.timeActive / tween.duration;
      float easingValue = easeInOutSine(progress);
      float step = difference * easingValue;
      float newValue = tween.initialValue + step;
      *value = newValue;
      spdlog::info("Tween system: Value Changed: {}", *value);
    }
  }

  if (!pendingTweens.empty()) {
    tweens.insert(tweens.end(), pendingTweens.begin(), pendingTweens.end());
    pendingTweens.clear();
  }
}

void TweenSystem::DrawImGui() {}

Tween& TweenSystem::CreateTween(const float initialValue, const float targetValue, float duration) {
  Tween tween = Tween(initialValue, targetValue, duration);
  this->pendingTweens.push_back(tween);
  return this->pendingTweens.back();
}
