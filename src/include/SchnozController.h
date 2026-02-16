#pragma once

#include "Scene.h"
#include "SceneComponent.h"

class SchnozTag : public GameObject {};

class SchnozController : public SceneComponent {
private:
  // if isAnimating is set to true it will update the value before the tween system runs
  //  needs to be set to run after or do fixed some other way
  float value = 1.0f;
  SchnozTag* schnoz = nullptr;
  bool isAnimating = false;
public:
  SchnozController(Scene* scene); 
  virtual ~SchnozController() = default;
 
	virtual void OnPreUpdate();
private:
  void StartGrowing();
  void StartShrinking();
};
