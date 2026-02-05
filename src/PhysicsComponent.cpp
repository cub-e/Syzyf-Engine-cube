#include "PhysicsComponent.h"

#include "Jolt/Core/Core.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Body/BodyManager.h"
#include "PhysicsObject.h"
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <Scene.h>

#include <iostream>
#include <cstdarg>

using namespace JPH;
using namespace JPH::literals;
using namespace std;

namespace {
  /// Class that determines if two object layers can collide
  class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
  {
  public:
    virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
    {
      switch (inObject1)
      {
      case PhysicsComponent::Layers::NON_MOVING:
        return inObject2 == PhysicsComponent::Layers::MOVING; // Non moving only collides with moving
      case PhysicsComponent::Layers::MOVING:
        return true; // Moving collides with everything
      default:
        JPH_ASSERT(false);
        return false;
      }
    }
  };

  // BroadPhaseLayerInterface implementation
  // This defines a mapping between object and broadphase layers.
  class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
  {
  public:
                    BPLayerInterfaceImpl()
    {
      // Create a mapping table from object to broad phase layer
      mObjectToBroadPhase[PhysicsComponent::Layers::NON_MOVING] = PhysicsComponent::BroadPhaseLayers::NON_MOVING;
      mObjectToBroadPhase[PhysicsComponent::Layers::MOVING] = PhysicsComponent::BroadPhaseLayers::MOVING;
    }

    virtual uint					GetNumBroadPhaseLayers() const override
    {
      return PhysicsComponent::BroadPhaseLayers::NUM_LAYERS;
    }

    virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
    {
      JPH_ASSERT(inLayer < PhysicsComponent::Layers::NUM_LAYERS);
      return mObjectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char *			GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
    {
      switch ((BroadPhaseLayer::Type)inLayer)
      {
      case (BroadPhaseLayer::Type)Physics::BroadPhaseLayers::NON_MOVING.GetValue():	return "NON_MOVING";
      case (BroadPhaseLayer::Type)Physics::BroadPhaseLayers::MOVING.GetValue():		return "MOVING";
      default:													JPH_ASSERT(false); return "INVALID";
      }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

  private:
    BroadPhaseLayer					mObjectToBroadPhase[PhysicsComponent::Layers::NUM_LAYERS];
  };

  /// Class that determines if an object layer can collide with a broadphase layer
  class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
  {
  public:
    virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
    {
      switch (inLayer1)
      {
      case PhysicsComponent::Layers::NON_MOVING:
        return inLayer2 == PhysicsComponent::BroadPhaseLayers::MOVING;
      case PhysicsComponent::Layers::MOVING:
        return true;
      default:
        JPH_ASSERT(false);
        return false;
      }
    }
  };

  // An example contact listener
  class MyContactListener : public ContactListener
  {
  public:
	// See: ContactListener
	virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override
	{
		cout << "Contact validate callback" << endl;

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
		cout << "A contact was added" << endl;
	}

	virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
		cout << "A contact was persisted" << endl;
	}

	virtual void			OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
	{
		cout << "A contact was removed" << endl;
	}
  };

  // An example activation listener
  class MyBodyActivationListener : public BodyActivationListener
  {
  public:
	virtual void		OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
		cout << "A body got activated" << endl;
	}

	virtual void		OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
		cout << "A body went to sleep" << endl;
	}
  };
};

PhysicsComponent::PhysicsComponent(Scene* scene): SceneComponent(scene) {
    JPH::RegisterDefaultAllocator();
    Factory::sInstance = new Factory();
    RegisterTypes();

    tempAllocator = new TempAllocatorImpl(10 * 1024 * 1024);
    jobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

    bpLayerInterface = new BPLayerInterfaceImpl();
    objVsBPFilter = new ObjectVsBroadPhaseLayerFilterImpl();
    objVsObjFilter = new ObjectLayerPairFilterImpl();

    const uint cMaxBodies = 1024;
    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 1024;
    const uint cMaxContactConstraints = 1024;

    physicsSystem = new PhysicsSystem();
    physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
            *bpLayerInterface, *objVsBPFilter, *objVsObjFilter);
    bodyInterface = &physicsSystem->GetBodyInterface();

    BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));
    floor_shape_settings.SetEmbedded();
    ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();

    BodyCreationSettings floor_settings(floor_shape_result.Get(), RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
    bodyInterface->CreateAndAddBody(floor_settings, EActivation::DontActivate);

    // BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(0.0_r, 10.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);

    // sphere_settings.mRestitution = 0.1f;

    // schnozBodyID = bodyInterface->CreateAndAddBody(sphere_settings, EActivation::Activate);

    // bodyInterface->SetLinearVelocity(schnozBodyID, Vec3(0.0f, -5.0f, -0.1f));

    // BodyCreationSettings sphere2_settings(new SphereShape(0.5f), RVec3(0.0_r, 10.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);

    // sphere2_settings.mRestitution = 0.2f;

    // schnoz2BodyID = bodyInterface->CreateAndAddBody(sphere2_settings, EActivation::Activate);

    // bodyInterface->SetLinearVelocity(schnozBodyID, Vec3(0.0f, -5.0f, 0.1f));

    // physicsSystem->OptimizeBroadPhase();
  }

  // Should be called by the scene i think
  PhysicsComponent::~PhysicsComponent() {
      delete physicsSystem;
      delete objVsObjFilter;
      delete objVsBPFilter;
      delete bpLayerInterface;
      delete jobSystem;
      delete tempAllocator;
  }

  JPH::BodyInterface* PhysicsComponent::GetBodyInterface() {
      return bodyInterface;
  }
  JPH::PhysicsSystem* PhysicsComponent::GetSystem() {
      return physicsSystem;
  }

  void PhysicsComponent::OnPreUpdate() {
      physicsSystem->Update(cDeltaTime, 1, tempAllocator, jobSystem);

      std::vector<PhysicsObject*> objects = GetScene()->FindObjectsOfType<PhysicsObject>();

      for (auto object : objects) {
          JPH::BodyID objectId = object->getBodyId();

          RVec3 position = bodyInterface->GetCenterOfMassPosition(objectId);
          Quat rotation = bodyInterface->GetRotation(objectId);

          glm::vec3 glmPosition = glm::vec3(position.GetX(), position.GetY(), position.GetZ());
          glm::quat glmRotation = glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());

          object->GetTransform().GlobalTransform().Position() = glmPosition;
          object->GetTransform().GlobalTransform().Rotation() = glmRotation;
      }
  }

  // Callback for traces, connect this to your own trace function if you have one
  void PhysicsComponent::TraceImpl(const char *inFMT, ...) {
    // Format the message
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    // Print to the TTY
    cout << buffer << endl;
  }

#ifdef JPH_ENABLE_ASSERTS

  // Callback for asserts, connect this to your own assert handler if you have one
  bool PhysicsComponent::AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine) {
  // Print to the TTY
    cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << endl;

    // Breakpoint
    return true;
  };

#endif // JPH_ENABLE_ASSERTS
