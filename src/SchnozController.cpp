#include "SchnozController.h"

#include "tweening/TweenSystem.h"
#include <vector>

SchnozController::SchnozController(Scene* scene) : SceneComponent(scene) {
  this->isAnimating = true;
  StartGrowing();
}

void SchnozController::OnPreUpdate() {
  if (this->schnoz == nullptr) {
    SchnozTag* schnozTag = this->GetScene()->FindObjectsOfType<SchnozTag>().front();
    if (schnozTag == nullptr) {
      spdlog::warn("SchnozController: No SchnozTag entities, returning...");
      return;
    } else {
      spdlog::info("SchnozController: Found schnoz, registering");
      this->schnoz = schnozTag;
    }
  }

  if (this->isAnimating) {
    spdlog::info("Updating scale, value: {}", this->value);
    this->schnoz->LocalTransform().Scale() = glm::vec3(this->value);
  }
}

void SchnozController::StartGrowing() {
  TweenSystem* tweenSystem = GetScene()->GetComponent<TweenSystem>();
  TweenId tween = tweenSystem->CreateTween(1.0f, 5.0f, 3.0f);
  tweenSystem->BindValue(tween, &this->value);
  tweenSystem->SetOnComplete(tween, [this]() {
    this->StartShrinking();
  });
}

void SchnozController::StartShrinking() {
  TweenSystem* tweenSystem = GetScene()->GetComponent<TweenSystem>();
  TweenId tween = tweenSystem->CreateTween(5.0f, 1.0f, 3.0f);
  tweenSystem->BindValue(tween, &this->value);
  tweenSystem->SetOnComplete(tween, [this]() {
    this->StartGrowing();
  });
}
