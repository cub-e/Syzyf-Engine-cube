#include "tweening/TweenSystem.h"
#include "Scene.h"

#include <tweening/EasingFunctions.h>

#include <spdlog/spdlog.h>

// Add an option to pre-allocate the vectors perhaps
//  if it matters
TweenSystem::TweenSystem(Scene* scene) : SceneComponent(scene) {
  this->allocator = Allocator();
}

void TweenSystem::OnPreUpdate() {
  const float deltaTime = GetScene()->DeltaTime();

  for (std::size_t i = 0; i < this->tweens.size(); ++i) {
    if (!this->tweens[i].has_value())
      continue;

    Tween& tween = this->tweens[i].value();
    tween.timeActive += deltaTime;

    spdlog::info("TweenSystem: delta time: {}", deltaTime);
    spdlog::info("TweenSystem: time active: {}", tween.timeActive);

    if (tween.timeActive >= tween.duration) {
      for (auto& value : tween.values) {
        *value = tween.targetValue;
      }

      for (auto& onComplete : tween.onComplete) {
        onComplete();
      }

      this->allocator.free.push_back(i);
      this->tweens[i].reset();

      spdlog::info("Tween system: Tween removed");

      continue;
    }

    for (auto& value : tween.values) {
      float difference = tween.targetValue - tween.initialValue;
      float progress = tween.timeActive / tween.duration;
      float easingValue = Easing::inOutSine(progress);
      float step = difference * easingValue;
      float newValue = tween.initialValue + step;
      *value = newValue;
      spdlog::info("Tween system: Value Changed: {}", *value);
    }
  }
}

void TweenSystem::DrawImGui() {}

TweenId TweenSystem::CreateTween(const float initialValue, const float targetValue, const float duration) {
  TweenId id = this->allocator.Allocate();
  
  if (this->tweens.size() <= id.id)
    this->tweens.resize(id.id + 1);

  this->tweens[id.id].emplace(initialValue, targetValue, duration);
  
  return id;
}

void TweenSystem::RemoveTween(const TweenId id) {
  if (this->tweens[id.id].has_value() && id.generation == this->allocator.generations[id.id]) {
    this->allocator.free.push_back(id.id);
    this->tweens[id.id].reset();
  }
}

bool TweenSystem::IsValid(const TweenId id) const {
  if (id.id >= this->tweens.size()) return false;
  return this->tweens[id.id].has_value() && id.generation == this->allocator.generations[id.id];
}

void TweenSystem::SetOnComplete(const TweenId id, const std::function<void()> onComplete) {
  if (!this->IsValid(id)) {
    spdlog::warn("TweenSystem: Tried setting an onComplete callback on an invalid handle");
    return;
  }
  if (onComplete == nullptr) {
    spdlog::warn("TweenSystem: Tried setting an invalid function as an 'onComplete' callback");
    return;
  }

  this->tweens[id.id]->onComplete.push_back(onComplete);
}

void TweenSystem::BindValue(const TweenId id, float* value) {
  if (!this->IsValid(id)) {
    spdlog::warn("TweenSystem: Tried binding a value on an invalid tween handle");
    return;
  }

  this->tweens[id.id]->values.push_back(value);
}


Tween* TweenSystem::GetTween(const TweenId id) {
  if (IsValid(id)) {
    return &this->tweens[id.id].value();
  }

  return nullptr;
}

