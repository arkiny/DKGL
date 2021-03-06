//
//  File: DKRigidBody.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKCollisionObject.h"
#include "DKCollisionShape.h"

namespace DKFramework
{
	/// @brief
	/// a rigid body. a node object which can be used by DKScene.
	/// @details
	/// object should have CollisionShape to interact.
	/// @note
	/// this class not designed for subclassing.
	class DKGL_API DKRigidBody : public DKCollisionObject
	{
		friend class DKConstraint;
	public:
		/// internal object properties
		struct DKGL_API ObjectData
		{
			ObjectData(float mass = 0.0f, const DKVector3& localInertia = DKVector3::zero);
			float mass;
			DKVector3 localInertia;
			float linearDamping;
			float angularDamping;
			float friction;
			float rollingFriction;
			float restitution;
			float linearSleepingThreshold;
			float angularSleepingThreshold;
			bool additionalDamping;
			float additionalDampingFactor;
			float additionalLinearDampingThresholdSqr;
			float additionalAngularDampingThresholdSqr;
			float additionalAngularDampingFactor;
		};

		DKRigidBody(const DKString& name = L"");
		DKRigidBody(DKCollisionShape* shape, float mass = 0.0f);
		DKRigidBody(DKCollisionShape* shape, float mass, const DKVector3& localInertia);
		DKRigidBody(DKCollisionShape* shape, const ObjectData& data);
		~DKRigidBody();

		void SetWorldTransform(const DKNSTransform& t) override;
		void SetLocalTransform(const DKNSTransform& t) override;
		DKNSTransform CenterOfMassTransform() const;
		DKVector3 CenterOfMassPosition() const;

		/// mass (0 for static object)
		void SetMass(float mass);
		float Mass() const;
		float InverseMass() const;

		/// local inertia, can be calculated by CollisionShape
		void SetLocalInertia(const DKVector3& inertia);
		DKVector3 LocalInertia() const;
		DKVector3 InverseDiagLocalInertia() const;
		DKMatrix3 InverseWorldInertiaTensor() const;

		// linear velocity
		void SetLinearVelocity(const DKVector3&);
		DKVector3 LinearVelocity() const;

		// angular velocity
		void SetAngularVelocity(const DKVector3&);
		DKVector3 AngularVelocity() const;

		void SetLinearFactor(const DKVector3&);
		DKVector3 LinearFactor() const;

		void SetAngularFactor(const DKVector3&);
		DKVector3 AngularFactor() const;

		void SetLinearDamping(float);
		float LinearDamping() const;

		void SetAngularDamping(float);
		float AngularDamping() const;

		DKVector3 TotalForce() const;
		DKVector3 TotalTorque() const;

		DKVector3 VelocityInLocalPoint(const DKVector3& pos) const;

		float ComputeImpulseDenominator(const DKVector3& pos, const DKVector3& normal) const;
		float ComputeAngularImpulseDenominator(const DKVector3& axis) const;
		DKVector3 ComputeGyroscopicForce(float maxGyroscopicForce) const;

		void ApplyForce(const DKVector3& force, const DKVector3& relpos);
		void ApplyImpulse(const DKVector3& impulse, const DKVector3& relpos);

		void ApplyTorque(const DKVector3& torque);
		void ApplyCentralForce(const DKVector3& force);
		void ApplyCentralImpulse(const DKVector3& impulse);
		void ApplyTorqueImpulse(const DKVector3& torque);

		void ClearForces();

		void SetLinearSleepingThreshold(float);
		void SetAngularSleepingThreshold(float);
		float LinearSleepingThreshold() const;
		float AngularSleepingThreshold() const;

		DKObject<DKSerializer> Serializer() override;  

	protected:
		DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKRigidBody* Copy(UUIDObjectMap&, const DKRigidBody*);

		bool ResetObject(DKCollisionShape* shape, const ObjectData& data);
		bool GetObjectData(ObjectData& data) const;

		void OnAddedToParent() override;
		void OnSetAnimation(DKAnimatedTransform*) override;
		void OnUpdateSceneState(const DKNSTransform& parentWorldTransform) override;

	private:
		class btMotionState* motionState;
	};
}
