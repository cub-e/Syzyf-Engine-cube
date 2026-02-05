#pragma once

#include "Jolt/Jolt.h"
#include <Jolt/Physics/Body/BodyID.h>

#include <GameObject.h>
#include <glm/fwd.hpp>
// #include <Debug.h>

using namespace JPH::literals;

class PhysicsObject : public GameObject {
    private:
        JPH::BodyID bodyId;
        bool bodyCreated = false;
        float mass = 1.0f;
        float friction = 0.5f;
    public:
    PhysicsObject();
    virtual ~PhysicsObject();

    JPH::BodyID getBodyId() const {
        return bodyId;
    }

    void Awake();
    void Enable();
    void Disable();
};
