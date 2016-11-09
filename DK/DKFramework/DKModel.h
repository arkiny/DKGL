﻿//
//  File: DKModel.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKTransform.h"
#include "DKAnimationController.h"

///////////////////////////////////////////////////////////////////////////////
// DKModel
// a skeletal node object for scene (DKWorld).
// this object can be structured hierarchical. (parent-children relationship)
// this is basic entry for scene.
// this object can be animated with DKAnimationController.
//
// Note:
//    On serialize, world-transform will not saved, calculated with local
//    transform after restored.
//    If you subclass, you should override Clone() with your inherited type.
//    In case of constraint (DKConstraint), not all objects can be accepted as
//    parent which is DKConstraint's reference bodies. You can overrides this
//    behavior.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKWorld;
	class DKGL_API DKModel : public DKResource
	{
	public:
		enum Type
		{
			TypeCustom = 0,
			TypeMesh,
			TypeCollision,
			TypeConstraint,
			TypeAction,
		};
		using NamedObjectMap = DKMap<DKString, DKModel*>;
		using UUIDObjectMap = DKMap<DKUuid, DKModel*>;

		using Enumerator = DKFunctionSignature<bool(DKModel*)>;
		using EnumeratorLoop = DKFunctionSignature<void(DKModel*)>;
		using ConstEnumerator = DKFunctionSignature<bool(const DKModel*)>;
		using ConstEnumeratorLoop = DKFunctionSignature<void(const DKModel*)>;

		const Type type;

		DKModel(void);
		virtual ~DKModel(void);

		DKWorld* Scene(void)					{ return scene; }
		const DKWorld* Scene(void) const		{ return scene; }
		virtual void RemoveFromScene(void);

		bool AddChild(DKModel*);
		void RemoveFromParent(void);

		DKModel* RootObject(void);
		const DKModel* RootObject(void) const;

		DKModel* Parent(void)				{ return parent; }
		const DKModel* Parent(void) const	{ return parent; }

		bool IsDescendantOf(const DKModel* obj) const;
		size_t NumberOfDescendants(void) const;
		DKModel* FindDescendant(const DKString&);
		const DKModel* FindDescendant(const DKString&) const;
		DKModel* FindCommonAncestor(DKModel*, DKModel*, Type t = TypeCustom);

		DKModel* ChildAtIndex(unsigned int i)					{ return children.Value(i); }
		const DKModel* ChildAtIndex(unsigned int i) const		{ return children.Value(i); }
		size_t NumberOfChildren(void) const						{ return children.Count(); }

		// show/hide all descendants (not self)
		void SetDescendantsHidden(bool hidden)			{ hideDescendants = hidden; }
		bool AreDescendantsHidden(void) const			{ return hideDescendants; }
		bool DidAncestorHideDescendants(void) const;

		void Enumerate(Enumerator* e)					{ EnumerateInternal(e); }
		void Enumerate(EnumeratorLoop* e)				{ EnumerateInternal(e); }
		void Enumerate(ConstEnumerator* e) const		{ EnumerateInternal(e); }
		void Enumerate(ConstEnumeratorLoop* e) const	{ EnumerateInternal(e); }

		DKAnimatedTransform* Animation(void)				{ return animation; }
		const DKAnimatedTransform* Animation(void) const	{ return animation; }
		void SetAnimation(DKAnimatedTransform*, bool recursive = true);

		virtual void SetWorldTransform(const DKNSTransform&);
		virtual void SetLocalTransform(const DKNSTransform&);
		const DKNSTransform& WorldTransform(void) const		{ return worldTransform; }
		const DKNSTransform& LocalTransform(void) const		{ return localTransform; }

		void CreateNamedObjectMap(NamedObjectMap&); // building object map for search by name.
		void CreateUUIDObjectMap(UUIDObjectMap&);   // building object map for search by UUID.

		// UpdateKinematic : before simulate
		// UpdateSceneState : after simulate, before rendering
		void UpdateKinematic(double timeDelta, DKTimeTick tick);
		void UpdateSceneState(const DKNSTransform&);

		void UpdateLocalTransform(bool recursive = true);
		void UpdateWorldTransform(bool recursive = true);

		// clone object. (internal resources are shared)
		DKObject<DKModel> Clone(void) const;
		// serializer
		DKObject<DKSerializer> Serializer(void) override;

	protected:
		virtual void OnAddedToScene(void) {}
		virtual void OnRemovedFromScene(void) {}
		virtual void OnAddedToParent(void) {}
		virtual void OnRemovedFromParent(void) {}
		virtual void OnSetAnimation(DKAnimatedTransform*) {}

		// OnUpdateTreeReferences: called on tree changed or restored.
		virtual void OnUpdateTreeReferences(NamedObjectMap&, UUIDObjectMap&) {}

		// OnUpdateKinematic: called before simulate.
		virtual void OnUpdateKinematic(double timeDelta, DKTimeTick tick);

		// OnUpdateSceneState: called after simulate, before render.
		virtual void OnUpdateSceneState(const DKNSTransform& parentWorldTransform);

		// ResolveTree: update descendants.
		// this calls OnUpdateTreeReferences() if necessary.
		void ResolveTree(bool force = false);

		// ReloadSceneContext: resetting object
		// if object be in scene, temporary removed from scene and calls given
		// operation(op) then add to scene again.
		// parameter op must have object-reset routines.
		void ReloadSceneContext(DKOperation* op);

		DKModel(Type t); // t = TypeCustom for custom subclass.

		// Clone: clone node tree. (current object's clone will be root of cloned tree)
		// clone operation does save object's UUIDs to given parameter (UUIDObjectMap&)
		// You can find cloned object with original object's UUID with UUIDObjectMap
		virtual DKObject<DKModel> Clone(UUIDObjectMap&) const;
		DKModel* Copy(UUIDObjectMap&, const DKModel*);

		// UpdateCopiedReferenceUUIDs: called on object has been cloned. (UUID has changed).
		// reference object could be replace as cloned that can be found with original
		// object's UUID.
		// parameter (UUIDObjectMap&)'s key is original object's UUID,
		// value is cloned object.
		virtual void UpdateCopiedReferenceUUIDs(UUIDObjectMap&) {}

		// CanAcceptObjectAsParent: called when object trying to add target
		// object as a child. return true to accept.
		// you can override this behavior.
		virtual bool CanAcceptObjectAsParent(DKModel*) const;

		DKNSTransform localTransform;
		DKNSTransform worldTransform;

	private:
		DKModel* parent;
		DKWorld* scene;
		DKArray<DKObject<DKModel>> children;
		DKObject<DKAnimatedTransform> animation;

		bool hideDescendants;

		// set true to call OnUpdateTreeReferences() when next update.
		bool needResolveTree;

		bool EnumerateInternal(Enumerator* e);
		void EnumerateInternal(EnumeratorLoop* e);
		bool EnumerateInternal(ConstEnumerator* e) const;
		void EnumerateInternal(ConstEnumeratorLoop* e) const;

		DKModel& operator = (const DKModel&);

		friend class DKWorld;
	};
}
