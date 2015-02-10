//
//  File: DKScene.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKCamera.h"
#include "DKColor.h"
#include "DKSceneState.h"
#include "DKRenderer.h"
#include "DKModel.h"
#include "DKCollisionObject.h"

////////////////////////////////////////////////////////////////////////////////
// DKScene
// compose scene with DKModel tree.
// you can detect collision with DKModel nodes.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKScene
	{
	public:
		DKScene(void);
		virtual ~DKScene(void);

		struct DrawCallback
		{
			using VertexArray = DKFoundation::DKArray<DKRenderer::Vertex3DColored>;
			using MeshArray = DKFoundation::DKArray<const DKMesh*>;

			virtual ~DrawCallback(void) {}
			virtual void DrawTriangles(VertexArray&, const DKMatrix4&) = 0;
			virtual void DrawLines(VertexArray&, const DKMatrix4&) = 0;
			virtual void DrawPoints(VertexArray&, const DKMatrix4&) = 0;
			virtual void DrawMeshes(MeshArray&, DKSceneState&) = 0;
			virtual bool ObjectColors(const DKCollisionObject*, DKColor&, DKColor&) { return true; }
		};
		void Render(const DKCamera& camera, int sceneIndex, bool enableCulling, DrawCallback& dc) const;
		void Render(const DKCamera& camera, int sceneIndex, bool enableCulling, DrawCallback& dc, unsigned int modes) const;

		virtual void Update(double tickDelta, DKFoundation::DKTimeTick tick);

		enum : unsigned int
		{
			DrawMeshes				= 1,
			DrawMeshBoundSpheres	= 1 << 1,
			DrawCollisionAABB		= 1 << 2,
			DrawCollisionShapes		= 1 << 3,
			DrawConstraints			= 1 << 4,
			DrawConstraintLimits	= 1 << 5,
			DrawContactPoints		= 1 << 6,
			DrawWireframe			= 1 << 7,
			DrawSkeletalLines		= 1 << 8,
		};
		unsigned int drawMode;

		DKColor ambientColor;
		DKFoundation::DKArray<DKLight> lights;

		typedef DKFoundation::DKFunctionSignature<float (DKCollisionObject*, float, const DKVector3&)> RayResultCallback;
		typedef DKFoundation::DKFunctionSignature<float (const DKCollisionObject*, float, const DKVector3&)> ConstRayResultCallback;

		// ray-test to query all hit objects.
		size_t RayTest(const DKVector3& begin, const DKVector3& end, RayResultCallback* cb);
		size_t RayTest(const DKVector3& begin, const DKVector3& end, ConstRayResultCallback* cb) const;

		// ray-test to query closest hit.
		DKCollisionObject* RayTestClosest(const DKVector3& begin, const DKVector3& end, DKVector3* hitPoint = NULL, DKVector3* hitNormal = NULL);
		const DKCollisionObject* RayTestClosest(const DKVector3& begin, const DKVector3& end, DKVector3* hitPoint = NULL, DKVector3* hitNormal = NULL) const;

		bool AddObject(DKModel*);
		void RemoveObject(DKModel*);
		virtual void RemoveAllObjects(void);

		virtual void SetSceneState(const DKCamera& cam, DKSceneState& state) const;

		class CollisionWorldContext;

		size_t NumberOfSceneObjects(void) const;
		typedef DKFoundation::DKFunctionSignature<bool(const DKModel*)> BEnumerator;
		typedef DKFoundation::DKFunctionSignature<void(const DKModel*)> VEnumerator;
		void Enumerate(BEnumerator* e) const;
		void Enumerate(VEnumerator* e) const;

	protected:
		CollisionWorldContext* context;
		DKScene(CollisionWorldContext* ctxt);

		DKFoundation::DKSet<DKFoundation::DKObject<DKModel>> sceneObjects;
		DKFoundation::DKSet<DKMesh*> meshes;

		void UpdateObjectKinematics(double tickDelta, DKFoundation::DKTimeTick tick);
		void UpdateObjectSceneStates(void);

		virtual bool AddSingleObject(DKModel* obj);
		virtual void RemoveSingleObject(DKModel* obj);

		// picking out nodes to protect between update sequence.
		void PrepareUpdateNode(void);
		void CleanupUpdateNode(void);

	private:
		DKFoundation::DKSpinLock lock;

		DKFoundation::DKArray<DKFoundation::DKObject<DKModel>> updatePendingObjects;

		DKScene(const DKScene&);
		DKScene& operator = (const DKScene&);

		friend class DKModel;
	};
}
