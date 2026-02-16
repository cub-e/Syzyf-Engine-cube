#pragma once

#include <functional>
#include <vector>

class TweenSystem;

class Tween {
friend TweenSystem;
public:
  std::function<void()> onComplete = nullptr;
private:
  float timeActive;
  float duration;
  std::vector<float*> values;
  float initialValue;
  float targetValue;
public:
  void TweenValue(float* value, std::function<void()> onComplete = nullptr);
private:
  Tween(float initialValue, float targetValue, float duration);
};
