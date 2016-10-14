//
//  File: DKScene.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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

namespace DKGL
{
	class DKGL_API DKScene
	{
	public:
		DKScene(void);
		virtual ~DKScene(void);

		struct DrawCallback
		{
			using VertexArray = DKArray<DKRenderer::Vertex3DColored>;
			using MeshArray = DKArray<const DKMesh*>;

			virtual ~DrawCallback(void) {}
			virtual void DrawTriangles(VertexArray&, const DKMatrix4&) = 0;
			virtual void DrawLines(VertexArray&, const DKMatrix4&) = 0;
			virtual void DrawPoints(VertexArray&, const DKMatrix4&) = 0;
			virtual void DrawMeshes(MeshArray&, DKSceneState&) = 0;
			virtual bool ObjectColors(const DKCollisionObject*, DKColor&, DKColor&) { return true; }
		};
		void Render(const DKCamera& camera, int sceneIndex, unsigned int modes, unsigned int groupFilter, bool enableCulling, DrawCallback& dc) const;

		virtual void Update(double tickDelta, DKTimeTick tick);

		enum : unsigned int
		{
			DrawMeshes				= 1,
			DrawMeshBoundSpheres	= 1 << 1,
			DrawCollisionAabb		= 1 << 2,
			DrawCollisionShapes		= 1 << 3,
			DrawConstraints			= 1 << 4,
			DrawConstraintLimits	= 1 << 5,
			DrawContactPoints		= 1 << 6,
			DrawWireframe			= 1 << 7,
			DrawSkeletalLines		= 1 << 8,
		};

		DKColor ambientColor;

		typedef DKFunctionSignature<float (DKCollisionObject*, float, const DKVector3&)> RayResultCallback;
		typedef DKFunctionSignature<float (const DKCollisionObject*, float, const DKVector3&)> ConstRayResultCallback;

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
		typedef DKFunctionSignature<bool(const DKModel*)> BEnumerator;
		typedef DKFunctionSignature<void(const DKModel*)> VEnumerator;
		void Enumerate(BEnumerator* e) const;
		void Enumerate(VEnumerator* e) const;

	protected:
		CollisionWorldContext* context;
		DKScene(CollisionWorldContext* ctxt);

		DKSet<DKObject<DKModel>> sceneObjects;
		DKSet<DKMesh*> meshes;

		void UpdateObjectKinematics(double tickDelta, DKTimeTick tick);
		void UpdateObjectSceneStates(void);

		virtual bool AddSingleObject(DKModel* obj);
		virtual void RemoveSingleObject(DKModel* obj);

		// picking out nodes to protect between update sequence.
		void PrepareUpdateNode(void);
		void CleanupUpdateNode(void);

	private:
		DKSpinLock lock;

		DKArray<DKObject<DKModel>> updatePendingObjects;

		DKScene(const DKScene&);
		DKScene& operator = (const DKScene&);

		friend class DKModel;
	};
}
