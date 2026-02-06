#include "SchnozController.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <spdlog/spdlog.h>

#include "physics/PhysicsObject.h"
#include "physics/PhysicsComponent.h"

SchnozController::SchnozController() {
  onPushSchnoz = [this](const PushSchnozEvent& e) {
    this->PushSchnoz();
  };
}

SchnozController::~SchnozController() {
  GetScene()->Unsubscribe<PushSchnozEvent>(onPushSchnoz);
}

void SchnozController::Awake() {
  GetScene()->Subscribe<PushSchnozEvent>(onPushSchnoz);
}

void SchnozController::PushSchnoz() {
  auto* object = GetObject<PhysicsObject>();
  if (!object) {
    spdlog::error("Missing `PhysicsObject` on a node trying to use `PushSchnoz()`, Node: {}", GetNode()->GetName());
    return;
  };

  JPH::BodyID bodyId = GetObject<PhysicsObject>()->getBodyId();
  if (bodyId.IsInvalid()) {
    spdlog::warn("Body id is invalid");
    return;
  }

  PhysicsComponent* physics = GetScene()->GetComponent<PhysicsComponent>();
  if (!physics) {
    spdlog::error("Trying to use `PushSchnoz` on a scene missing a `PhysicsComponent`");
    return;
  }

  JPH::BodyInterface* bodyInterface = physics->GetBodyInterface();

  spdlog::info("Body is active: {}", bodyInterface->IsActive(bodyId));
  
  JPH::Vec3 impulse(0.0f, 1000.0f, -5000.0f);
  bodyInterface->AddImpulse(bodyId, impulse);
}
