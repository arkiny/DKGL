//
//  File: DKDynamicsWorld.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKWorld.h"
#include "DKRigidBody.h"
#include "DKSoftBody.h"
#include "DKConstraint.h"
#include "DKActionController.h"

////////////////////////////////////////////////////////////////////////////////
// DKDynamicsWorld
// a scene with dynamics.
// You can extend physical behavior with DKActionController.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKDynamicsWorld : public DKWorld
	{
	public:
		DKDynamicsWorld(void);
		virtual ~DKDynamicsWorld(void);

		void SetGravity(const DKVector3& g);
		DKVector3 Gravity(void) const;

		void Update(double tickDelta, DKTimeTick tick) override;

		// if FixedFrameRate is greater than 0,
		// it works as fixed-frame rate in physics. (determinism)
		// You need to use fixed frame rate when sharing scene with
		// other simulator. (ex: connected via network or subprocesses)
		void SetFixedFrameRate(double fps);
		double FixedFrameRate(void) const;

		void RemoveAllObjects(void) override;

	protected:
		bool AddSingleObject(DKModel* obj) override;
		void RemoveSingleObject(DKModel* obj) override;

		typedef DKSet<DKRigidBody*> RigidBodySet;
		typedef DKSet<DKSoftBody*> SoftBodySet;
		typedef DKSet<DKConstraint*> ConstraintSet;
		typedef DKSet<DKActionController*> ActionSet;

		RigidBodySet rigidBodies;
		SoftBodySet softBodies;
		ConstraintSet constraints;
		ActionSet actions;

		virtual void UpdateActions(double tickDelta);

		virtual bool NeedCollision(DKCollisionObject* objA, DKCollisionObject* objB);
		virtual bool NeedResponse(DKCollisionObject* objA, DKCollisionObject* objB);

	private:
		double dynamicsFixedFPS; // fixed time stepping unit.
		static void PreTickCallback(void*, float);
		static void PostTickCallback(void*, float);
		class btActionInterface* actionInterface;

		friend class DKActionController;
		friend class DKConstraint;
	};
}