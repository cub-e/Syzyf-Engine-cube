#pragma once

#include "GameObject.h"
#include <vector>
  
// This needs to be separated into data and a component that tracks the animation progress
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

  struct AnimationData {
    std::string name = "";
    std::vector<Track> tracks;
    float duration = 0.0f;
  };

  struct Animation {
    AnimationData data;
    
    float timeActive = 0.0f;
    bool playing = true;
    bool looping = true;
  };

  std::vector<Animation> animations;
public:
  AnimationComponent();
  virtual ~AnimationComponent() = default;
};
