#pragma once

#include "SceneComponent.h"

#include <optional>
#include <vector>

#include <functional>
#include <vector>

class TweenSystem;

class Tween {
friend TweenSystem;
public:
  std::vector<std::function<void()>> onComplete;
private:
  float timeActive = 0.0f;
  float duration = 0.0f;
  std::vector<float*> values;
  float initialValue = 0.0f;
  float targetValue = 0.0f;
public:
  Tween(float initialValue, float targetValue, float duration) : 
    initialValue(initialValue), targetValue(targetValue), duration(duration) {}
};

struct TweenId {
  std::size_t id;
  std::size_t generation;
};

// Move this into TweenSystem
//  and allow for preallocation perhaps
class Allocator {
friend TweenSystem;
private:
  std::vector<std::size_t> free;
  std::vector<std::size_t> generations;
 
public:
  Allocator() {};

  TweenId Allocate() {
    if (this->free.empty()) {
      this->generations.push_back(0);
      return TweenId {
        this->generations.size() - 1,
        0,
      };
    };

    std::size_t id = this->free.back();
    this->free.pop_back();
    this->generations[id] += 1;
    return TweenId {
      id,
      this->generations[id],
    };
  }
};

class TweenSystem : public SceneComponent {
private:
  Allocator allocator;
  std::vector<std::optional<Tween>> tweens;

public:
  TweenSystem(Scene* scene);

  void OnPreUpdate();
  void DrawImGui();

  TweenId CreateTween(const float initialValue, const float targetValue, const float duration);

  void RemoveTween(const TweenId id);

  bool IsValid(const TweenId id) const;

  void SetOnComplete(const TweenId id, const std::function<void()> onComplete);
  void BindValue(const TweenId id, float* value);

private:
  Tween* GetTween(const TweenId id);
};
