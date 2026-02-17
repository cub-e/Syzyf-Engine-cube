#include "SchnozController.h"

#include "MeshRenderer.h"
#include "TweenSystem.h"

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
    this->schnoz->LocalTransform().Position() = glm::vec3(3.0f, 0.25f + this->value * 2.0f, 0.0f);
    this->schnoz->GetObject<MeshRenderer>()->GetMaterial()->SetValue("progress", this->value * this->value);
    this->schnoz->LocalTransform().Scale() = glm::vec3(0.25f + this->value * this->value * 0.25f);
  }
}

void SchnozController::StartGrowing() {
  TweenSystem* tweenSystem = GetScene()->GetComponent<TweenSystem>();
  TweenId tween = tweenSystem->CreateTween(TweenConfig {0.0f, 1.0f, 2.0f, Easing::outBounce});
  tweenSystem->BindValue(tween, &this->value);
  tweenSystem->SetOnComplete(tween, [this]() {
    this->StartShrinking();
  });
}

void SchnozController::StartShrinking() {
  TweenSystem* tweenSystem = GetScene()->GetComponent<TweenSystem>();
  TweenId tween = tweenSystem->CreateTween(TweenConfig {1.0f, 0.0f, 5.0f, Easing::outBounce});
  tweenSystem->BindValue(tween, &this->value);
  tweenSystem->SetOnComplete(tween, [this]() {
    this->StartGrowing();
  });
}
