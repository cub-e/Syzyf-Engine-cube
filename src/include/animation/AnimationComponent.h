#pragma once

#include "GameObject.h"
#include <vector>

class AnimationComponent : public GameObject {
public:
  enum Property {
    POSITION,
    SCALE,
    ROTATION,
    WEIGHTS
  };

  enum Interpolation {
    LINEAR,
    STEP,
    CUBICSPLINE
  };

  struct Track {
    SceneNode* target = nullptr;
    Property property;
    Interpolation interpolation;
    std::vector<float> inputs;
    std::vector<float> outputs;
  };

  struct Animation {
    std::string name = "";
    std::vector<Track> tracks;
  };

  std::vector<Animation> animations;
public:
  AnimationComponent();
  virtual ~AnimationComponent() = default;
};
