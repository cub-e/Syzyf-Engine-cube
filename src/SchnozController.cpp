#include "Jolt/Jolt.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "physics/PhysicsComponent.h"
#include "physics/PhysicsObject.h"
#include <SchnozController.h>
#include <spdlog/spdlog.h>

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
  spdlog::info("Schnoz pchniety");
  JPH::BodyID bodyId = GetObject<PhysicsObject>()->getBodyId();

  if (bodyId.IsInvalid()) {
    spdlog::warn("Body id is invalid");
    return;
  }
  PhysicsComponent* physics = GetScene()->GetComponent<PhysicsComponent>();

  JPH::BodyInterface* bodyInterface = physics->GetBodyInterface();

  spdlog::info("Body is active: {}", bodyInterface->IsActive(bodyId));
  
  JPH::Vec3 impulse(0.0f, 1000.0f, -5000.0f);
  bodyInterface->AddImpulse(bodyId, impulse);
}
