#pragma once

#include "GameObject.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Character/Character.h>

class PhysicsCharacter : public GameObject {
public:
  // make private
  JPH::Character* character = nullptr;
private:
  JPH::Ref<JPH::CharacterSettings> characterSettings;
public:
  PhysicsCharacter();
  virtual ~PhysicsCharacter();

  void Awake();
  void Enable();
  void Disable();
};
