#include "physics/PhysicsObject.h"
#include "physics/PhysicsComponent.h"
#include <spdlog/spdlog.h>

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

PhysicsObject::PhysicsObject() {};

PhysicsObject::~PhysicsObject() {
    if (bodyCreated) {
        // missing logs
        PhysicsComponent* physics = GetScene()->GetComponent<PhysicsComponent>();
        if (physics) {
            physics->GetBodyInterface()->RemoveBody(bodyId);
            physics->GetBodyInterface()->DestroyBody(bodyId);
        }
    }
}

void PhysicsObject::Awake() {
    PhysicsComponent* physics = GetScene()->GetComponent<PhysicsComponent>();
    if (!physics) {
        spdlog::warn("Tried waking up a physics object without a PhysicsComponent");
        return;
    }

    JPH::RVec3 position = JPH::RVec3(0.0_r, 0.0_r, 0.0_r);
    JPH::Quat rotation = JPH::Quat::sIdentity();

    SceneNode* node = GetNode();
    if (node) {
        SceneTransform::PositionAccess nodePosition = node->GetTransform().GlobalTransform().Position();
        SceneTransform::RotationAccess nodeRotation = node->GetTransform().GlobalTransform().Rotation();
        position = JPH::RVec3(nodePosition.x, nodePosition.y, nodePosition.z);
        rotation = JPH::Quat(nodeRotation.x, nodeRotation.y, nodeRotation.z, nodeRotation.w);
    }

    JPH::BodyCreationSettings sphereSettings(new JPH::SphereShape(0.5f),
        position, rotation,
        JPH::EMotionType::Dynamic, PhysicsComponent::Layers::MOVING);

    sphereSettings.mRestitution = 0.1f;

    bodyId = physics->GetBodyInterface()->CreateBody(sphereSettings)->GetID();
    bodyCreated = !bodyId.IsInvalid();
}

void PhysicsObject::Enable() {
    if (!bodyCreated) {
        spdlog::warn("Tried enabling a body that hasn't been created yet");
        return;
    }

    PhysicsComponent* physics = GetScene()->GetComponent<PhysicsComponent>();
    if (physics) {
        physics->GetBodyInterface()->AddBody(bodyId, JPH::EActivation::Activate);
    }
}

void PhysicsObject::Disable() {
    if (!bodyCreated) {
        spdlog::warn("Tried disabling a body that hasn't been created yet");
        return;
    }

    PhysicsComponent* physics = GetScene()->GetComponent<PhysicsComponent>();
    if (physics) {
        physics->GetBodyInterface()->RemoveBody(bodyId);
    }
}
