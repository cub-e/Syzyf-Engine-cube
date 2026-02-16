#include "tweening/Tween.h"
#include <vector>

Tween::Tween(float initialValue, float targetValue, float duration) : timeActive(0.0f), duration(duration), initialValue(initialValue), targetValue(targetValue),
  values(
    std::vector<float*>()) {}

void Tween::TweenValue(float* value, std::function<void()> onComplete) {
  this->values.push_back(value);
  this->onComplete = onComplete;
}
