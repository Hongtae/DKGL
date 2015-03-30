//
//  File: DKScene.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKMath.h"
#include "DKScene.h"
#include "DKRenderer.h"
#include "DKModel.h"
#include "DKMesh.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		class ShapeDrawer : public btIDebugDraw
		{
		public:
			struct ObjectColorCallback
			{
				virtual ~ObjectColorCallback(void) {}
				virtual bool GetObjectColors(const DKCollisionObject*, DKColor&, DKColor&) = 0;
			};

			static DKRenderer::Vertex3DColored CVertex(const btVector3& v, const btVector3& c, btScalar a)
			{
				return DKRenderer::Vertex3DColored(BulletVector3(v), DKColor(c.x(), c.y(), c.z(), a));
			}
			void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor, btScalar fromAlpha, btScalar toAlpha)
			{
				lines.Add(CVertex(from, fromColor, fromAlpha));
				lines.Add(CVertex(to, toColor, toAlpha));
			}
			void drawLine(const btVector3& from, const btVector3& to, const btVector3& color, btScalar alpha)
			{
				drawLine(from, to, color, color, alpha, alpha);
			}
			void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor) override
			{
				drawLine(from, to, fromColor, toColor, 1.0, 1.0);
			}
			void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
			{
				drawLine(from, to, color, 1.0);
			}
			void drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha) override
			{
				triangles.Add(CVertex(a, color, alpha));
				triangles.Add(CVertex(b, color, alpha));
				triangles.Add(CVertex(c, color, alpha));
			}
			void drawTriangleLine(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha)
			{
				drawLine(a, b, color, alpha);
				drawLine(b, c, color, alpha);
				drawLine(c, a, color, alpha);
			}
			void drawQuad(const btVector3& lt, const btVector3& rt, const btVector3& rb, const btVector3& lb, const btVector3& color, btScalar alpha)
			{
				drawTriangle(lt, lb, rt, color, alpha);
				drawTriangle(rt, lb, rb, color, alpha);
			}
			void drawQuadLine(const btVector3& lt, const btVector3& rt, const btVector3& rb, const btVector3& lb, const btVector3& color, btScalar alpha)
			{
				drawLine(lt, lb, color, alpha);
				drawLine(lt, rt, color, alpha);
				drawLine(rt, rb, color, alpha);
				drawLine(lb, rb, color, alpha);
			}
			void drawSphere(btScalar radius, const btTransform& transform, const btVector3& color, btScalar alpha, int lats, int longs, bool wire)
			{
				lats = Max(lats, 2);
				longs = Max(longs, 3);

				if (radius > 0)
				{
					for (int i = 0; i < lats; ++i)
					{
						float lat0 = DKL_PI * (static_cast<float>(i) / lats - 0.5f);
						float lat1 = DKL_PI * (static_cast<float>(i + 1) / lats - 0.5f);
						float y0 = radius * sin(lat0);
						float y1 = radius * sin(lat1);
						float r0 = radius * cos(lat0);
						float r1 = radius * cos(lat1);

						for (int j = 0; j < longs; ++j)
						{
							float lng0 = 2.0 * DKL_PI * static_cast<float>(j) / longs;
							float lng1 = 2.0 * DKL_PI * static_cast<float>(j + 1) / longs;
							float x0 = cos(lng0);
							float x1 = cos(lng1);
							float z0 = sin(lng0);
							float z1 = sin(lng1);

							btVector3 quad[4] = {
								transform(btVector3(x0 * r0, y0, z0 * r0)),
								transform(btVector3(x0 * r1, y1, z0 * r1)),
								transform(btVector3(x1 * r0, y0, z1 * r0)),
								transform(btVector3(x1 * r1, y1, z1 * r1))
							};

							if (wire)
							{
								drawLine(quad[0], quad[1], color, alpha);
								drawLine(quad[0], quad[2], color, alpha);
							}
							else
							{
								drawQuad(quad[0], quad[2], quad[3], quad[1], color, alpha);
							}
						}
					}
				}
			}
			void drawSphere(btScalar radius, const btTransform& transform, const btVector3& color, btScalar alpha)
			{
				drawSphere(radius, transform, color, alpha, 5, 8, true);
			}
			void drawSphere(btScalar radius, const btTransform& transform, const btVector3& color) override
			{
				drawSphere(radius, transform, color, 1.0, 5, 8, true);
			}
			void drawCone(btScalar radius, btScalar height, const btTransform& transform, const btVector3& color, btScalar alpha, int slices, bool wire)
			{
				slices = Max(slices, 3);
				if (radius > 0 && height > 0)
				{
					for (int i = 0; i < slices; ++i)
					{
						float ang0 = 2.0 * DKL_PI * (static_cast<float>(i) / slices);
						float ang1 = 2.0 * DKL_PI * (static_cast<float>(i + 1) / slices);
						float x0 = sin(ang0) * radius;
						float z0 = cos(ang0) * radius;
						float x1 = sin(ang1) * radius;
						float z1 = cos(ang1) * radius;

						if (wire)
						{
							btVector3 verts[3] = {
								transform(btVector3(x0, 0, z0)),
								transform(btVector3(x1, 0, z1)),
								transform(btVector3(0, height, 0))
							};
							drawTriangleLine(verts[1], verts[0], verts[2], color, alpha);
						}
						else
						{
							btVector3 verts[4] = {
								transform(btVector3(x0, 0, z0)),
								transform(btVector3(x1, 0, z1)),
								transform(btVector3(0, height, 0)),
								transform(btVector3(0, 0, 0))
							};
							drawTriangle(verts[0], verts[1], verts[2], color, alpha);
							drawTriangle(verts[1], verts[0], verts[3], color, alpha);
						}
					}
				}
			}
			void drawCone(btScalar radius, btScalar height, int upAxis, const btTransform& transform, const btVector3& color, btScalar alpha, int slices, bool wire)
			{
				btTransform tr2(transform);
				switch (upAxis)
				{
				case 0:    // up direction is aligned width -x axis.
					tr2 *= btTransform(btQuaternion(btVector3(0, 0, -1), SIMD_HALF_PI));
					break;
				case 1:    // up direction is aligned width y axis. (default)
					break;
				case 2:    // up direction is aligned width -z axis.
					tr2 *= btTransform(btQuaternion(btVector3(1, 0, 0), SIMD_HALF_PI));
					break;
				}
				tr2 *= btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -0.5*height, 0));

				drawCone(radius, height, tr2, color, alpha, slices, wire);
			}
			void drawCone(btScalar radius, btScalar height, int upAxis, const btTransform& transform, const btVector3& color) override
			{
				drawCone(radius, height, upAxis, transform, color, 1.0, 10, true);
			}
			void drawCapsule(btScalar radius, btScalar halfHeight, const btTransform& transform, const btVector3& color, btScalar alpha, int slices, int stacks, bool wire)
			{
				slices = Max(slices, 3);
				stacks = Max(stacks, 2);

				if (radius > 0 && halfHeight > 0)
				{
					for (int i = 0; i < slices; ++i)
					{
						// capsule total height = height + radius*2
						float ang0 = 2.0 * DKL_PI * (static_cast<float>(i) / slices);
						float ang1 = 2.0 * DKL_PI * (static_cast<float>(i + 1) / slices);
						float x0 = sin(ang0) * radius;
						float x1 = sin(ang1) * radius;
						float z0 = cos(ang0) * radius;
						float z1 = cos(ang1) * radius;

						// body of capsule
						btVector3 verts[4] = {
							transform(btVector3(x0, halfHeight, z0)),
							transform(btVector3(x1, halfHeight, z1)),
							transform(btVector3(x0, -halfHeight, z0)),
							transform(btVector3(x1, -halfHeight, z1))
						};
						if (wire)
						{
							drawLine(verts[0], verts[2], color, alpha);
						}
						else
						{
							drawQuad(verts[0], verts[1], verts[3], verts[2], color, alpha);
						}

						// capsule's upper, lower half-sphere
						for (int j = 0; j < stacks; ++j)
						{
							float sp0 = DKL_PI * 0.5 * (static_cast<float>(j) / stacks);
							float sp1 = DKL_PI * 0.5 * (static_cast<float>(j + 1) / stacks);
							float y0 = radius * sin(sp0) + halfHeight;
							float y1 = radius * sin(sp1) + halfHeight;
							float r0 = cos(sp0);
							float r1 = cos(sp1);

							btVector3 quad[8] = {
								transform(btVector3(x0 * r0, y0, z0 * r0)),
								transform(btVector3(x0 * r1, y1, z0 * r1)),
								transform(btVector3(x1 * r0, y0, z1 * r0)),
								transform(btVector3(x1 * r1, y1, z1 * r1)),
								transform(btVector3(x0 * r0, -y0, z0 * r0)),
								transform(btVector3(x0 * r1, -y1, z0 * r1)),
								transform(btVector3(x1 * r0, -y0, z1 * r0)),
								transform(btVector3(x1 * r1, -y1, z1 * r1))
							};

							if (wire)
							{
								drawLine(quad[0], quad[1], color, alpha);
								drawLine(quad[0], quad[2], color, alpha);
								drawLine(quad[4], quad[5], color, alpha);
								drawLine(quad[4], quad[6], color, alpha);
							}
							else
							{
								drawQuad(quad[0], quad[1], quad[3], quad[2], color, alpha);
								drawQuad(quad[4], quad[6], quad[7], quad[5], color, alpha);
							}
						}
					}
				}
			}
			void drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color, btScalar alpha, int slices, int stacks, bool wire)
			{
				btTransform tr2(transform);
				switch (upAxis)
				{
				case 0:    // up direction aligned width -x axis.
					tr2 *= btTransform(btQuaternion(btVector3(0, 0, -1), SIMD_HALF_PI));
					break;
				case 1:    // up direction aligned width y axis. (default)
					break;
				case 2:    // up direction aligned width -z axis.
					tr2 *= btTransform(btQuaternion(btVector3(1, 0, 0), SIMD_HALF_PI));
					break;
				}
				drawCapsule(radius, halfHeight, tr2, color, alpha, slices, stacks, wire);
			}
			void drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color) override
			{
				drawCapsule(radius, halfHeight, upAxis, transform, color, 1.0, 10, 4, true);
			}
			void drawCylinder(btScalar radius, btScalar halfHeight, const btTransform& transform, const btVector3& color, btScalar alpha, int slices, bool wire)
			{
				slices = Max(slices, 3);

				btScalar height = halfHeight * 2;

				if (radius > 0 && height > 0)
				{
					for (int i = 0; i < slices; ++i)
					{
						float ang0 = 2.0 * DKL_PI * (static_cast<float>(i) / slices);
						float ang1 = 2.0 * DKL_PI * (static_cast<float>(i + 1) / slices);
						float x0 = sin(ang0) * radius;
						float z0 = cos(ang0) * radius;
						float x1 = sin(ang1) * radius;
						float z1 = cos(ang1) * radius;

						if (wire)
						{
							btVector3 verts[4] = {
								transform(btVector3(x0, 0, z0)),
								transform(btVector3(x1, 0, z1)),
								transform(btVector3(x0, height, z0)),
								transform(btVector3(x1, height, z1))
							};
							drawQuadLine(verts[2], verts[3], verts[1], verts[0], color, alpha);
						}
						else
						{
							btVector3 verts[6] = {
								transform(btVector3(x0, 0, z0)),
								transform(btVector3(x1, 0, z1)),
								transform(btVector3(x0, height, z0)),
								transform(btVector3(x1, height, z1)),
								transform(btVector3(0, 0, 0)),
								transform(btVector3(0, height, 0))
							};
							drawQuad(verts[2], verts[3], verts[1], verts[0], color, alpha);
							drawTriangle(verts[1], verts[0], verts[4], color, alpha);
							drawTriangle(verts[2], verts[3], verts[5], color, alpha);
						}
					}
				}
			}
			void drawCylinder(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color, btScalar alpha, int slices, bool wire)
			{
				btTransform tr2(transform);
				switch (upAxis)
				{
				case 0:    // up direction aligned width -x axis.
					tr2 *= btTransform(btQuaternion(btVector3(0, 0, -1), SIMD_HALF_PI));
					break;
				case 1:    // up direction aligned width y axis. (default)
					break;
				case 2:    // up direction aligned width -z axis.
					tr2 *= btTransform(btQuaternion(btVector3(1, 0, 0), SIMD_HALF_PI));
					break;
				}
				tr2 *= btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -halfHeight, 0));
				drawCylinder(radius, halfHeight, tr2, color, alpha, slices, wire);
			}
			void drawCylinder(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color) override
			{
				drawCylinder(radius, halfHeight, upAxis, transform, color, 1.0, 10, true);
			}
			void drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override
			{
				const btVector3 to = pointOnB + normalOnB * distance;
				const btVector3& from = pointOnB;

				drawLine(from, to, color);
			}
			void reportErrorWarning(const char* warningString) override
			{
				DKLog("Physics Warning: %s\n", warningString);
			}
			void draw3dText(const btVector3& location, const char* textString) override
			{
				DKLog("Physics 3d text location:(%f,%f,%f): %s\n", location.x(), location.y(), location.z(), textString);
			}
			void drawWorld(btCollisionWorld* world, ObjectColorCallback& cb)
			{
				DKASSERT_DEBUG(world->getDebugDrawer() == this);

				world->debugDrawWorld();

				if (drawShapes)
				{
					int numObjects = world->getNumCollisionObjects();
					for (int i = 0; i < numObjects; ++i)
					{
						const btCollisionObject* col = world->getCollisionObjectArray()[i];
						btTransform tr = col->getWorldTransform();

						const btRigidBody* body = btRigidBody::upcast(col);

						if (body && body->getMotionState())
						{
							DKASSERT_DEBUG(dynamic_cast<const btDefaultMotionState*>(body->getMotionState()));
							const btDefaultMotionState* motionState = static_cast<const btDefaultMotionState*>(body->getMotionState());
							tr = motionState->m_graphicsWorldTrans;
						}

						DKCollisionObject* co = (DKCollisionObject*)col->getUserPointer();
						DKASSERT_DEBUG(co);

						DKColor faceColor = DKColor(0.5f, 0.5f, 0.5f);
						DKColor edgeColor = DKColor(0.0f, 0.0f, 0.0f);

						if (cb.GetObjectColors(co, faceColor, edgeColor))
						{
							btVector3 aabbMin, aabbMax;
							world->getBroadphase()->getBroadphaseAabb(aabbMin, aabbMax);

							aabbMin -= btVector3(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
							aabbMax += btVector3(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);

							drawCollisionShape(tr, col->getCollisionShape(), aabbMin, aabbMax,
								btVector3(faceColor.r, faceColor.g, faceColor.b), faceColor.a,
								btVector3(edgeColor.r, edgeColor.g, edgeColor.b), edgeColor.a);
						}
					}
				}
				// purge unused cache
				PurgeCache(this->purgeChcheTickCount);
			}
			void drawCollisionShape(const btTransform& tr, const btCollisionShape* shape, const btVector3& aabbMin, const btVector3& aabbMax, const btVector3& color, btScalar alpha, const btVector3& wColor, btScalar wAlpha)
			{
				int shapeType = shape->getShapeType();

				if (shapeType == CUSTOM_CONVEX_SHAPE_TYPE)
				{
					btVector3 org = tr.getOrigin();
					btVector3 dx = tr.getBasis().getRow(0);
					btVector3 dy = tr.getBasis().getRow(1);
					btVector3 dz = tr.getBasis().getRow(2);

					const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
					btVector3 halfExtent = boxShape->getHalfExtentsWithMargin();
					dx *= halfExtent[0];
					dy *= halfExtent[1];
					dz *= halfExtent[2];

					btVector3 p[4] = { org - dx - dy, org - dx + dy, org + dx + dy, org + dx - dy };
					drawLine(p[0], p[1], color);
					drawLine(p[1], p[2], color);
					drawLine(p[2], p[3], color);
					drawLine(p[3], p[0], color);
				}
				else if (shapeType == UNIFORM_SCALING_SHAPE_PROXYTYPE)
				{
					const btUniformScalingShape* scalingShape = static_cast<const btUniformScalingShape*>(shape);
					const btConvexShape* convexShape = scalingShape->getChildShape();
					float scalingFactor = (float)scalingShape->getUniformScalingFactor();

					btTransform tr2;
					tr2.setIdentity();
					tr2.getBasis()[0] = btVector3(scalingFactor, 0, 0);
					tr2.getBasis()[1] = btVector3(0, scalingFactor, 0);
					tr2.getBasis()[2] = btVector3(0, 0, scalingFactor);

					drawCollisionShape(tr * tr2, convexShape, aabbMin, aabbMax, color, alpha, wColor, wAlpha);
				}
				else if (shapeType == COMPOUND_SHAPE_PROXYTYPE)
				{
					const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(shape);
					for (int i = compoundShape->getNumChildShapes() - 1; i >= 0; i--)
					{
						btTransform tr2 = compoundShape->getChildTransform(i);
						const btCollisionShape* colShape = compoundShape->getChildShape(i);

						drawCollisionShape(tr * tr2, colShape, aabbMin, aabbMax, color, alpha, wColor, wAlpha);
					}
				}
				else
				{
					switch (shapeType)
					{
					case SPHERE_SHAPE_PROXYTYPE:
						if (true) {
							const btSphereShape* sphereShape = static_cast<const btSphereShape*>(shape);
							float radius = sphereShape->getMargin();	//radius doesn't include the margin, so draw with margin
							drawSphere(radius, tr, color, alpha, 5, 8, false);
							drawSphere(radius, tr, wColor, wAlpha, 5, 8, true);
						}
						break;
					case BOX_SHAPE_PROXYTYPE:
						if (true) {
							const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
							btVector3 halfExtent = boxShape->getHalfExtentsWithMargin();

							btVector3 vertices[8] = {
								tr(btVector3(halfExtent[0], halfExtent[1], halfExtent[2])),
								tr(btVector3(-halfExtent[0], halfExtent[1], halfExtent[2])),
								tr(btVector3(halfExtent[0], -halfExtent[1], halfExtent[2])),
								tr(btVector3(-halfExtent[0], -halfExtent[1], halfExtent[2])),
								tr(btVector3(halfExtent[0], halfExtent[1], -halfExtent[2])),
								tr(btVector3(-halfExtent[0], halfExtent[1], -halfExtent[2])),
								tr(btVector3(halfExtent[0], -halfExtent[1], -halfExtent[2])),
								tr(btVector3(-halfExtent[0], -halfExtent[1], -halfExtent[2]))
							};

							drawQuad(vertices[1], vertices[0], vertices[2], vertices[3], color, alpha);
							drawQuad(vertices[5], vertices[1], vertices[3], vertices[7], color, alpha);
							drawQuad(vertices[4], vertices[5], vertices[7], vertices[6], color, alpha);
							drawQuad(vertices[0], vertices[4], vertices[6], vertices[2], color, alpha);
							drawQuad(vertices[1], vertices[5], vertices[4], vertices[0], color, alpha);
							drawQuad(vertices[3], vertices[2], vertices[6], vertices[7], color, alpha);

							drawQuadLine(vertices[0], vertices[1], vertices[3], vertices[2], wColor, wAlpha);
							drawQuadLine(vertices[4], vertices[5], vertices[7], vertices[6], wColor, wAlpha);
							drawLine(vertices[0], vertices[4], wColor, wAlpha);
							drawLine(vertices[1], vertices[5], wColor, wAlpha);
							drawLine(vertices[2], vertices[6], wColor, wAlpha);
							drawLine(vertices[3], vertices[7], wColor, wAlpha);
						}
						break;
					case CAPSULE_SHAPE_PROXYTYPE:
						if (true) {
							const btCapsuleShape* capsuleShape = static_cast<const btCapsuleShape*>(shape);
							int upAxis = capsuleShape->getUpAxis();

							float radius = capsuleShape->getRadius();//+ capsuleShape->getMargin();
							float halfHeight = capsuleShape->getHalfHeight();//+ capsuleShape->getMargin();

							drawCapsule(radius, halfHeight, upAxis, tr, color, alpha, 10, 4, false);
							drawCapsule(radius, halfHeight, upAxis, tr, wColor, wAlpha, 10, 4, true);
						}
						break;
					case CONE_SHAPE_PROXYTYPE:
						if (true) {
							const btConeShape* coneShape = static_cast<const btConeShape*>(shape);
							int upIndex = coneShape->getConeUpIndex();
							float radius = coneShape->getRadius();//+coneShape->getMargin();
							float height = coneShape->getHeight();//+coneShape->getMargin();

							drawCone(radius, height, upIndex, tr, color, alpha, 10, false);
							drawCone(radius, height, upIndex, tr, wColor, wAlpha, 10, true);
						}
						break;
					case CYLINDER_SHAPE_PROXYTYPE:
						if (true) {
							const btCylinderShape* cylinder = static_cast<const btCylinderShape*>(shape);
							int upAxis = cylinder->getUpAxis();

							float radius = cylinder->getRadius();
							float halfHeight = cylinder->getHalfExtentsWithMargin()[upAxis];

							drawCylinder(radius, halfHeight, upAxis, tr, color, alpha, 10, false);
							drawCylinder(radius, halfHeight, upAxis, tr, wColor, wAlpha, 10, true);
						}
						break;
					case MULTI_SPHERE_SHAPE_PROXYTYPE:
						if (true) {
							const btMultiSphereShape* multiSphereShape = static_cast<const btMultiSphereShape*>(shape);

							btTransform tr2;
							tr2.setIdentity();

							for (int i = multiSphereShape->getSphereCount() - 1; i >= 0; i--)
							{
								btSphereShape sc(multiSphereShape->getSphereRadius(i));
								tr2.setOrigin(multiSphereShape->getSpherePosition(i));

								// 2012-07-08 by Hongtae Kim.
								// TODO: multiply order is not obvious. test needed.
								btTransform tr3 = tr;
								tr3 *= tr2;
								drawCollisionShape(tr3, &sc, aabbMin, aabbMax, color, alpha, wColor, wAlpha);
							}
						}
						break;
					case STATIC_PLANE_PROXYTYPE:
						if (true) {
							const btStaticPlaneShape* staticPlaneShape = static_cast<const btStaticPlaneShape*>(shape);
							btScalar planeConst = staticPlaneShape->getPlaneConstant();
							const btVector3& planeNormal = staticPlaneShape->getPlaneNormal();
							btVector3 planeOrigin = planeNormal * planeConst;
							btVector3 vec0, vec1;
							btPlaneSpace1(planeNormal, vec0, vec1);

							btScalar vecLen = 100.0f;
							{
								btVector3 pt0 = tr(planeOrigin + vec0*vecLen);
								btVector3 pt1 = tr(planeOrigin - vec0*vecLen);
								btVector3 pt2 = tr(planeOrigin + vec1*vecLen);
								btVector3 pt3 = tr(planeOrigin - vec1*vecLen);

								drawLine(pt0, pt1, wColor, wAlpha);
								drawLine(pt2, pt3, wColor, wAlpha);

								//drawQuad(pt1 + pt2, pt0 + pt2, pt0 + pt3, pt1 + pt3, color, 1.0);		// upper side
								//drawQuad(pt0 + pt2, pt1 + pt2, pt1 + pt3, pt0 + pt3, color, 1.0);		// lower side
							}
							// drawing grid lines
							int maxLines = 5;
							for (int i = 1; i <= maxLines; ++i)
							{
								btScalar s = vecLen * static_cast<float>(i) / (maxLines);
								btVector3 pt[8] = {
									tr(planeOrigin + vec0*vecLen + vec1*s),
									tr(planeOrigin - vec0*vecLen + vec1*s),
									tr(planeOrigin + vec0*vecLen - vec1*s),
									tr(planeOrigin - vec0*vecLen - vec1*s),
									tr(planeOrigin + vec1*vecLen + vec0*s),
									tr(planeOrigin - vec1*vecLen + vec0*s),
									tr(planeOrigin + vec1*vecLen - vec0*s),
									tr(planeOrigin - vec1*vecLen - vec0*s)
								};
								drawLine(pt[0], pt[1], wColor, wAlpha);
								drawLine(pt[2], pt[3], wColor, wAlpha);
								drawLine(pt[4], pt[5], wColor, wAlpha);
								drawLine(pt[6], pt[7], wColor, wAlpha);
							}
						}
						break;
					default:
						if (shape->isConvex())
						{
							const btConvexPolyhedron* poly = shape->isPolyhedral() ? ((btPolyhedralConvexShape*)shape)->getConvexPolyhedron() : NULL;
							if (poly)
							{
								for (int i = 0; i < poly->m_faces.size(); ++i)
								{
									btVector3 centroid(0, 0, 0);
									int numVerts = poly->m_faces[i].m_indices.size();
									if (numVerts > 2)
									{
										btVector3 v1 = poly->m_vertices[poly->m_faces[i].m_indices[0]];
										for (int v = 0; v < poly->m_faces[i].m_indices.size() - 2; ++v)
										{
											btVector3 v2 = poly->m_vertices[poly->m_faces[i].m_indices[v + 1]];
											btVector3 v3 = poly->m_vertices[poly->m_faces[i].m_indices[v + 2]];

											drawTriangle(v1, v2, v3, color, alpha);
											drawTriangleLine(v1, v2, v3, wColor, wAlpha);
										}
									}
								}
							}
							else
							{
								ShapeCache* sc = Cache((btConvexShape*)shape);
								btShapeHull* hull = &sc->shapehull;
								if (hull->numTriangles() > 0)
								{
									int index = 0;
									const unsigned int* idx = hull->getIndexPointer();
									const btVector3* vtx = hull->getVertexPointer();

									for (int i = 0; i < hull->numTriangles(); i++)
									{
										int i1 = index++;
										int i2 = index++;
										int i3 = index++;
										btAssert(i1 < hull->numIndices() &&
											i2 < hull->numIndices() &&
											i3 < hull->numIndices());

										int index1 = idx[i1];
										int index2 = idx[i2];
										int index3 = idx[i3];
										btAssert(index1 < hull->numVertices() &&
											index2 < hull->numVertices() &&
											index3 < hull->numVertices());

										btVector3 v1 = tr(vtx[index1]);
										btVector3 v2 = tr(vtx[index2]);
										btVector3 v3 = tr(vtx[index3]);

										drawTriangle(v1, v2, v3, color, alpha);
										drawTriangleLine(v1, v2, v3, wColor, wAlpha);
									}
								}
							}
						}
						break;
					}

					if (shape->isConcave() && !shape->isInfinite())
					{
						struct Callback : public btTriangleCallback
						{
							void processTriangle(btVector3* triangle, int partId, int triangleIndex)
							{
								btVector3 verts[3] = { transform(triangle[0]), transform(triangle[1]), transform(triangle[2]) };

								lines.Add(DKRenderer::Vertex3DColored(DKVector3(verts[0].x(), verts[0].y(), verts[0].z()), wireColor));
								lines.Add(DKRenderer::Vertex3DColored(DKVector3(verts[1].x(), verts[1].y(), verts[1].z()), wireColor));
								lines.Add(DKRenderer::Vertex3DColored(DKVector3(verts[2].x(), verts[2].y(), verts[2].z()), wireColor));
								lines.Add(DKRenderer::Vertex3DColored(DKVector3(verts[1].x(), verts[1].y(), verts[1].z()), wireColor));
								lines.Add(DKRenderer::Vertex3DColored(DKVector3(verts[2].x(), verts[2].y(), verts[2].z()), wireColor));
								lines.Add(DKRenderer::Vertex3DColored(DKVector3(verts[0].x(), verts[0].y(), verts[0].z()), wireColor));

								triangles.Add(DKRenderer::Vertex3DColored(DKVector3(verts[0].x(), verts[0].y(), verts[0].z()), surfaceColor));
								triangles.Add(DKRenderer::Vertex3DColored(DKVector3(verts[1].x(), verts[1].y(), verts[1].z()), surfaceColor));
								triangles.Add(DKRenderer::Vertex3DColored(DKVector3(verts[2].x(), verts[2].y(), verts[2].z()), surfaceColor));

								triangles.Add(DKRenderer::Vertex3DColored(DKVector3(verts[2].x(), verts[2].y(), verts[2].z()), surfaceColor));
								triangles.Add(DKRenderer::Vertex3DColored(DKVector3(verts[1].x(), verts[1].y(), verts[1].z()), surfaceColor));
								triangles.Add(DKRenderer::Vertex3DColored(DKVector3(verts[0].x(), verts[0].y(), verts[0].z()), surfaceColor));
							}

							typedef DKArray<DKRenderer::Vertex3DColored> VertexArray;
							VertexArray& points;
							VertexArray& lines;
							VertexArray& triangles;
							const btTransform& transform;
							const DKColor& surfaceColor;
							const DKColor& wireColor;

							Callback(VertexArray& pa, VertexArray& la, VertexArray& ta, const btTransform& trans, const DKColor& sc, const DKColor& wc)
								: points(pa), lines(la), triangles(ta), transform(trans), surfaceColor(sc), wireColor(wc)
							{
							}
						} callback(this->points, this->lines, this->triangles, tr,
							DKColor(color.x(), color.y(), color.z(), alpha),
							DKColor(wColor.x(), wColor.y(), wColor.z(), wAlpha));

						btConcaveShape* concaveMesh = (btConcaveShape*)shape;
						concaveMesh->processAllTriangles(&callback, aabbMin, aabbMax);
					}
				}
			}

			struct ShapeCache
			{
				struct Edge
				{
					btVector3 n[2];
					int v[2];
				};
				ShapeCache(btConvexShape* s) : shapehull(s) {}
				btShapeHull					shapehull;
				btAlignedObjectArray<Edge>	edges;
				DKTimeTick					tick;
			};

			ShapeCache*	Cache(btConvexShape* shape)
			{
				DKASSERT_DEBUG(shape);
				ShapeCache* sc = NULL;
				ShapeCacheMap::Pair* p = this->shapeCaches.Find(shape);
				if (p)
				{
					DKASSERT_DEBUG(p->value);
					sc = p->value;
				}
				else
				{
					sc = new(btAlignedAlloc(sizeof(ShapeCache), 16)) ShapeCache(shape);
					sc->shapehull.buildHull(shape->getMargin());

					bool b = this->shapeCaches.Insert(shape, sc);
					DKASSERT_DEBUG(b);

					/* Build edges	*/
					const int			ni = sc->shapehull.numIndices();
					const int			nv = sc->shapehull.numVertices();
					const unsigned int*	pi = sc->shapehull.getIndexPointer();
					const btVector3*	pv = sc->shapehull.getVertexPointer();
					btAlignedObjectArray<ShapeCache::Edge*>	edges;
					sc->edges.reserve(ni);
					edges.resize(nv*nv, 0);
					for (int i = 0; i < ni; i += 3)
					{
						const unsigned int* ti = pi + i;
						const btVector3		nrm = btCross(pv[ti[1]] - pv[ti[0]], pv[ti[2]] - pv[ti[0]]).normalized();
						for (int j = 2, k = 0; k < 3; j = k++)
						{
							const unsigned int	a = ti[j];
							const unsigned int	b = ti[k];
							ShapeCache::Edge*&	e = edges[btMin(a, b)*nv + btMax(a, b)];
							if (!e)
							{
								sc->edges.push_back(ShapeCache::Edge());
								e = &sc->edges[sc->edges.size() - 1];
								e->n[0] = nrm; e->n[1] = -nrm;
								e->v[0] = a; e->v[1] = b;
							}
							else
							{
								e->n[1] = nrm;
							}
						}
					}
				}

				DKASSERT_DEBUG(sc);
				sc->tick = this->tick;
				return sc;
			}
			void PurgeCache(int c)
			{
				if (c < 0)
					c = 0;

				expiredShapes.Clear();
				this->shapeCaches.EnumerateForward([=](ShapeCacheMap::Pair& p)
				{
					int k = this->tick - p.value->tick;
					if (k > c)
						this->expiredShapes.Add(p.key);
				});
				for (btConvexShape* s : this->expiredShapes)
				{
					ShapeCacheMap::Pair* p = this->shapeCaches.Find(s);
					DKASSERT_DEBUG(p);
					ShapeCache* sc = p->value;
					DKASSERT_DEBUG(sc);
					sc->~ShapeCache();
					btAlignedFree(sc);
					this->shapeCaches.Remove(s);
				}
				size_t num = this->expiredShapes.Count();
				this->expiredShapes.Clear();
				if (num > 0)
					DKLog("[DKScene] %u cached convex shape removed.\n", num);
			}

			void setDebugMode(int mode)		{ debugMode = mode; }
			int getDebugMode() const		{ return debugMode; }

			ShapeDrawer(void)
				: debugMode(0)
				, drawShapes(false)
				, tick(0)
				, purgeChcheTickCount(10)
			{
			}
			~ShapeDrawer(void)
			{
				this->shapeCaches.EnumerateForward([](ShapeCacheMap::Pair& p)
				{
					ShapeCache* sc = p.value;
					DKASSERT_DEBUG(sc);
					sc->~ShapeCache();
					btAlignedFree(sc);
				});
				this->shapeCaches.Clear();
			}

			DKArray<DKRenderer::Vertex3DColored> points;
			DKArray<DKRenderer::Vertex3DColored> lines;
			DKArray<DKRenderer::Vertex3DColored> triangles;

			int debugMode;
			bool drawShapes;
			int purgeChcheTickCount;

			typedef DKMap<btConvexShape*, ShapeCache*> ShapeCacheMap;
			DKArray<btConvexShape*> expiredShapes;
			ShapeCacheMap	shapeCaches;
			DKTimeTick		tick;

			// data for mesh-rendering
			struct ZSortedMesh
			{
				const DKMesh* mesh;
				float distance;      // z value of view-projection applied
				bool opaque;
			};
			DKArray<ZSortedMesh> sortedMeshes;  // for sorting
			DKArray<const DKMesh*> drawMeshes;  // for drawing
			DKSceneState sceneState;
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;


DKScene::DKScene(void)
: context(NULL)
, drawMode(DrawMeshes)
, ambientColor(0, 0, 0)
{
	context = new CollisionWorldContext();
	context->configuration = new btDefaultCollisionConfiguration();
	context->dispatcher = new btCollisionDispatcher(context->configuration);
	context->broadphase = new btDbvtBroadphase();
	context->world = new btCollisionWorld(context->dispatcher, context->broadphase, context->configuration);
	context->solver = NULL;
	context->tick = 0;
	context->internalTick = 0;

	DKASSERT_DEBUG(context);
	DKASSERT_DEBUG(context->broadphase);
	DKASSERT_DEBUG(context->dispatcher);
	DKASSERT_DEBUG(context->configuration);
	DKASSERT_DEBUG(context->world);

	context->world->setDebugDrawer(new ShapeDrawer);
	context->world->setForceUpdateAllAabbs(false);
}

DKScene::DKScene(CollisionWorldContext* ctxt)
: context(ctxt)
, drawMode(DrawMeshes)
, ambientColor(0, 0, 0)
{
	DKASSERT_DEBUG(context);
	DKASSERT_DEBUG(context->broadphase);
	DKASSERT_DEBUG(context->dispatcher);
	DKASSERT_DEBUG(context->configuration);
	DKASSERT_DEBUG(context->world);
	context->tick = 0;
	context->internalTick = 0;
	context->world->setDebugDrawer(new ShapeDrawer);
}

DKScene::~DKScene(void)
{
	DKASSERT_DEBUG(context);
	DKASSERT_DEBUG(context->world);

	this->RemoveAllObjects();

	btIDebugDraw* drawer = context->world->getDebugDrawer();
	context->world->setDebugDrawer(NULL);

	if (context->world)
		delete context->world;
	if (context->solver)
		delete context->solver;
	if (context->broadphase)
		delete context->broadphase;
	if (context->dispatcher)
		delete context->dispatcher;
	if (context->configuration)
		delete context->configuration;

	context->world = NULL;
	context->solver = NULL;
	context->broadphase = NULL;
	context->dispatcher = NULL;
	context->configuration = NULL;

	delete context;
	delete drawer;
}

void DKScene::Update(double tickDelta, DKTimeTick tick)
{
	DKASSERT_DEBUG(context);
	DKASSERT_DEBUG(context->world);

	if (true)
	{
		DKCriticalSection<DKSpinLock> guard(context->lock);
		if (tick == context->tick)
			return;

		context->tick = tick;
	}
	PrepareUpdateNode();
	UpdateObjectKinematics(tickDelta, tick);

	if (true)
	{
		DKCriticalSection<DKSpinLock> guard(context->lock);

		int updated = 0;
		bool forceAABB = context->world->getForceUpdateAllAabbs();

		btCollisionObjectArray& colArray = context->world->getCollisionObjectArray();
		for (int i = 0; i < colArray.size(); ++i)
		{
			btCollisionObject* colObj = colArray[i];

			if (colObj->isActive())
			{
				btRigidBody* body = btRigidBody::upcast(colObj);
				if (body && body->getMotionState())
				{
					btTransform t;
					body->getMotionState()->getWorldTransform(t);
					body->setWorldTransform(t);
				}
				// if forceAABB is true, update all AABBs in performDiscreteCollisionDetection()
				if (!forceAABB)
					context->world->updateSingleAabb(colObj);
				colObj->setActivationState(ISLAND_SLEEPING);
				updated++;
			}
		}
		if (updated > 0)
			context->world->performDiscreteCollisionDetection();
	}
	UpdateObjectSceneStates();
	CleanupUpdateNode();
}

void DKScene::PrepareUpdateNode(void)
{
	DKASSERT_DEBUG(context);
	DKASSERT_DEBUG(context->world);

	DKCriticalSection<DKSpinLock> guard(this->lock);

	this->updatePendingObjects.Clear();
	this->updatePendingObjects.Reserve(this->sceneObjects.Count());
	this->sceneObjects.EnumerateForward([=](const DKModel* model)
	{
		if (model->Parent() == NULL)
		{
			updatePendingObjects.Add((const_cast<DKModel*>(model)));
		}
	});
}

void DKScene::CleanupUpdateNode(void)
{
	updatePendingObjects.Clear();
}

void DKScene::UpdateObjectKinematics(double tickDelta, DKTimeTick tick)
{
	for (DKModel* m : updatePendingObjects)
	{
		m->UpdateKinematic(tickDelta, tick);
	}
}

void DKScene::UpdateObjectSceneStates(void)
{
	for (DKModel* m : updatePendingObjects)
	{
		m->UpdateSceneState(DKNSTransform::identity);
	}
}

void DKScene::Render(const DKCamera& camera, int sceneIndex, bool enableCulling, DrawCallback& dc) const
{
	return this->Render(camera, sceneIndex, enableCulling, dc, this->drawMode);
}

void DKScene::Render(const DKCamera& camera, int sceneIndex, bool enableCulling, DrawCallback& dc, unsigned int modes) const
{
	DKASSERT_DEBUG(context);
	DKASSERT_DEBUG(context->world);

	btIDebugDraw* debugDrawer = context->world->getDebugDrawer();
	DKASSERT_DEBUG(debugDrawer);
	DKASSERT_DEBUG(dynamic_cast<ShapeDrawer*>(debugDrawer));
	ShapeDrawer* drawer = static_cast<ShapeDrawer*>(debugDrawer);

	DKCriticalSection<DKSpinLock> guard(this->lock);

	bool drawCollisionWorld = (modes | DrawMeshes) != DrawMeshes;

	if (drawCollisionWorld)
	{
		drawer->debugMode = 0;
		if (modes & DrawCollisionAABB)
			drawer->debugMode |= btIDebugDraw::DBG_DrawAabb;
		if (modes & DrawConstraints)
			drawer->debugMode |= btIDebugDraw::DBG_DrawConstraints;
		if (modes & DrawConstraintLimits)
			drawer->debugMode |= btIDebugDraw::DBG_DrawConstraintLimits;
		if (modes & DrawContactPoints)
			drawer->debugMode |= btIDebugDraw::DBG_DrawContactPoints;
		if (modes & DrawWireframe)
			drawer->debugMode |= btIDebugDraw::DBG_DrawWireframe;

		drawer->drawShapes = (modes & DrawCollisionShapes) != 0;

		drawer->triangles.Clear();
		drawer->lines.Clear();
		drawer->points.Clear();

		drawer->tick++;
		drawer->purgeChcheTickCount = 120;

		if (true)
		{
			struct ObjectColor : public ShapeDrawer::ObjectColorCallback
			{
				bool GetObjectColors(const DKCollisionObject* co, DKColor& faceColor, DKColor& edgeColor)
				{
					return dc.ObjectColors(co, faceColor, edgeColor);
				}
				DrawCallback& dc;
				ObjectColor(DrawCallback& d) : dc(d) {}
			};
			ObjectColor oc(dc);

			DKCriticalSection<DKSpinLock> guard(context->lock);
			drawer->drawWorld(context->world, oc);
		}

		if (modes & DrawSkeletalLines)
		{
			const DKColor skeletalLineColor(1.0, 1.0, 1.0);
			this->sceneObjects.EnumerateForward([=](const DKModel* obj)
			{
				DKASSERT_DEBUG(obj->scene == this);

				if (obj->Parent())
				{
					const DKNSTransform& trans1 = obj->Parent()->WorldTransform();
					const DKNSTransform& trans2 = obj->WorldTransform();
					drawer->lines.Add(DKRenderer::Vertex3DColored(trans1.position, skeletalLineColor));
					drawer->lines.Add(DKRenderer::Vertex3DColored(trans2.position, skeletalLineColor));
				}
			});
		}
		if (modes & DrawMeshBoundSpheres)
		{
			this->meshes.EnumerateForward([=](const DKMesh* mesh)
			{
				DKASSERT_DEBUG(mesh->scene == this);
				DKASSERT_DEBUG(mesh->type == DKModel::TypeMesh);
				DKSphere bs = mesh->BoundingSphere();
				if (bs.radius > 0.0f)
				{
					bs.center.Transform(mesh->ScaledWorldTransformMatrix());
					btTransform t;
					t.setIdentity();
					t.setOrigin(btVector3(bs.center.x, bs.center.y, bs.center.z));
					if (camera.IsSphereInside(bs))
						drawer->drawSphere(bs.radius, t, btVector3(0, 0, 1));
					else
						drawer->drawSphere(bs.radius, t, btVector3(1, 0, 0));
				}
			});
		}

		DKMatrix4 viewProjMat = camera.ViewProjectionMatrix();

		if (drawer->triangles.Count() > 0)
		{
			dc.DrawTriangles(drawer->triangles, viewProjMat);
		}
		if (drawer->lines.Count() > 0)
		{
			dc.DrawLines(drawer->lines, viewProjMat);
		}
		if (drawer->points.Count() > 0)
		{
			dc.DrawPoints(drawer->points, viewProjMat);
		}
		drawer->triangles.Clear();
		drawer->lines.Clear();
		drawer->points.Clear();
	}

	if (modes & DrawMeshes && sceneIndex >= 0)
	{
		// draw models..
		auto extractMeshes = [&](const DKMesh* mesh)
		{
			DKASSERT_DEBUG(mesh->scene == this);
			DKASSERT_DEBUG(mesh->type == DKModel::TypeMesh);

			if (!mesh->IsHidden() && !mesh->DidAncestorHideDescendants())
			{
				const DKMaterial* mat = mesh->Material();
				if (mat && mat->renderingProperties.Count() > sceneIndex)
				{
					const DKMatrix4& nodeTM = mesh->ScaledWorldTransformMatrix();
					bool visible = true;
					if (enableCulling)
					{
						DKSphere bs = mesh->ScaledBoundingSphere();
						if (bs.radius > 0.0f)
						{
							bs.center.Transform(nodeTM);
							visible = camera.IsSphereInside(bs);	// frustum culling
						}
					}
					if (visible)
					{
						const DKSceneState& sceneState = drawer->sceneState;
						const DKBlendState* blend = &(mat->renderingProperties.Value(sceneIndex).blendState);

						ShapeDrawer::ZSortedMesh meshInfo = { mesh, 0.0f, true };
						meshInfo.distance = (nodeTM * sceneState.viewProjectionMatrix).m[3][2];
						meshInfo.opaque = blend->dstBlendRGB == DKBlendState::BlendModeZero && blend->dstBlendAlpha == DKBlendState::BlendModeZero;

						drawer->sortedMeshes.Add(meshInfo);
					}
				}
			}
		};
		auto sorter = [](const ShapeDrawer::ZSortedMesh& lhs, const ShapeDrawer::ZSortedMesh& rhs)->bool
		{
			// sort order:
			// if opaque is true, shorter distance is front.
			// if opaque is false, longer distance is front.
			// for all objects, opaque is front.

			int op1 = rhs.opaque ? 0 : 1;
			int op2 = lhs.opaque ? 0 : 1;
			int c = op1 - op2;
			if (c == 0)
			{
				if (rhs.opaque)   // opaque object
					return rhs.distance < lhs.distance; // shorter is front
				return rhs.distance > lhs.distance;     // longer is front
			}
			return c > 0;
		};

		drawer->sortedMeshes.Clear();
		drawer->sortedMeshes.Reserve(this->meshes.Count());
		this->SetSceneState(camera, drawer->sceneState);
		this->meshes.EnumerateForward(extractMeshes);
		drawer->sortedMeshes.Sort<decltype(sorter)&>(sorter);

		drawer->sceneState.sceneIndex = sceneIndex;
		drawer->drawMeshes.Clear();
		drawer->drawMeshes.Reserve(drawer->sortedMeshes.Count());

		for (ShapeDrawer::ZSortedMesh& meshInfo : drawer->sortedMeshes)
		{
			if (meshInfo.mesh)
				drawer->drawMeshes.Add(meshInfo.mesh);
		}

		drawer->sortedMeshes.Clear();

		if (drawer->drawMeshes.Count() > 0)
		{
			dc.DrawMeshes(drawer->drawMeshes, drawer->sceneState);
		}

		drawer->drawMeshes.Clear();
		drawer->sceneState.Clear();
	}
}

size_t DKScene::RayTest(const DKVector3& begin, const DKVector3& end, RayResultCallback* cb)
{
	struct Callback : public btCollisionWorld::RayResultCallback
	{
		Callback(DKScene::RayResultCallback* cb) : callback(cb), numCollisions(0) {}
		DKScene::RayResultCallback* callback;
		size_t numCollisions;

		btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
		{
			numCollisions++;

			m_collisionObject = rayResult.m_collisionObject;
			DKCollisionObject* object = (DKCollisionObject*)rayResult.m_collisionObject->getUserPointer();
			DKASSERT_DEBUG(object != NULL);

			if (callback)
			{
				btVector3 hitNormalWorld;
				if (normalInWorldSpace)
				{
					hitNormalWorld = rayResult.m_hitNormalLocal;
				} else
				{
					///need to transform normal into worldspace
					hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
				}
				hitNormalWorld.normalize();
				return callback->Invoke(object, rayResult.m_hitFraction, BulletVector3(hitNormalWorld));
			}
			return m_closestHitFraction;
		}
	};
	DKASSERT_DEBUG(context && context->world);

	btVector3 rayBegin = BulletVector3(begin);
	btVector3 rayEnd = BulletVector3(end);
	Callback rayResult(cb);
	context->world->rayTest(rayBegin, rayEnd, rayResult);
	return rayResult.numCollisions;
}

size_t DKScene::RayTest(const DKVector3& begin, const DKVector3& end, ConstRayResultCallback* cb) const
{
	struct Callback : public btCollisionWorld::RayResultCallback
	{
		Callback(DKScene::ConstRayResultCallback* cb) : callback(cb), numCollisions(0) {}
		DKScene::ConstRayResultCallback* callback;
		size_t numCollisions;

		btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
		{
			numCollisions++;

			m_collisionObject = rayResult.m_collisionObject;
			const DKCollisionObject* object = (const DKCollisionObject*)rayResult.m_collisionObject->getUserPointer();
			DKASSERT_DEBUG(object != NULL);

			if (callback)
			{
				btVector3 hitNormalWorld;
				if (normalInWorldSpace)
				{
					hitNormalWorld = rayResult.m_hitNormalLocal;
				} else
				{
					///need to transform normal into worldspace
					hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
				}
				hitNormalWorld.normalize();
				return callback->Invoke(object, rayResult.m_hitFraction, BulletVector3(hitNormalWorld));
			}
			return m_closestHitFraction;
		}
	};
	DKASSERT_DEBUG(context && context->world);

	btVector3 rayBegin = BulletVector3(begin);
	btVector3 rayEnd = BulletVector3(end);
	Callback rayResult(cb);
	context->world->rayTest(rayBegin, rayEnd, rayResult);
	return rayResult.numCollisions;
}

DKCollisionObject* DKScene::RayTestClosest(const DKVector3& begin, const DKVector3& end, DKVector3* hitPoint, DKVector3* hitNormal)
{
	return const_cast<DKCollisionObject*>( static_cast<const DKScene*>(this)->RayTestClosest(begin, end, hitPoint, hitNormal));
}

const DKCollisionObject* DKScene::RayTestClosest(const DKVector3& begin, const DKVector3& end, DKVector3* hitPoint, DKVector3* hitNormal) const
{
	DKASSERT_DEBUG(context && context->world);

	const DKCollisionObject* result = NULL;

	btVector3 rayBegin = BulletVector3(begin);
	btVector3 rayEnd = BulletVector3(end);
	btCollisionWorld::ClosestRayResultCallback rayCallback(rayBegin, rayEnd);

	context->world->rayTest(rayBegin, rayEnd, rayCallback);
	if (rayCallback.hasHit())
	{
		result = (const DKCollisionObject*)rayCallback.m_collisionObject->getUserPointer();
		DKASSERT_DEBUG(result);

		if (hitPoint)
			*hitPoint = BulletVector3(rayCallback.m_hitPointWorld);
		if (hitNormal)
		{
			rayCallback.m_hitNormalWorld.normalize();
			*hitNormal = BulletVector3(rayCallback.m_hitNormalWorld);
		}
	}
	return result;
}

void DKScene::SetSceneState(const DKCamera& cam, DKSceneState& state) const
{
	state.Clear();

	state.sceneIndex = 0;

	state.viewMatrix = cam.ViewMatrix();
	state.viewMatrixInverse = DKMatrix4(state.viewMatrix).Inverse();
	state.projectionMatrix = cam.ProjectionMatrix();
	state.projectionMatrixInverse = DKMatrix4(state.projectionMatrix).Inverse();
	state.viewProjectionMatrix = cam.ViewProjectionMatrix();
	state.viewProjectionMatrixInverse = DKMatrix4(state.viewProjectionMatrix).Inverse();

	state.directionalLightDirections.Reserve(lights.Count());
	state.directionalLightColors.Reserve(lights.Count());
	state.pointLightColors.Reserve(lights.Count());
	state.pointLightPositions.Reserve(lights.Count());
	state.pointLightAttenuations.Reserve(lights.Count());

	state.cameraPosition = cam.ViewPosition();
	state.ambientColor = this->ambientColor;

	for (const DKLight& light : this->lights)
	{
		switch (light.Type())
		{
		case DKLight::LightTypeDirectional:
			state.directionalLightDirections.Add(light.Direction());
			state.directionalLightColors.Add(DKVector3(light.color.r, light.color.g, light.color.b));
			break;
		case DKLight::LightTypePoint:
			state.pointLightPositions.Add(light.position);
			state.pointLightColors.Add(DKVector3(light.color.r, light.color.g, light.color.b));
			state.pointLightAttenuations.Add(DKVector3(light.constAttenuation, light.linearAttenuation, light.quadraticAttenuation));
			break;
		default:
			DKLog("Unknown light type(%x)!\n", light.Type());
			break;
		}
	}
}

bool DKScene::AddObject(DKModel* obj)
{
	DKASSERT_DEBUG(context && context->world);

	if (obj == NULL)
		return false;
	if (obj->Scene())
		return false;
	if (obj->Parent())
		return false;

	obj->UpdateWorldTransform();

	DKCriticalSection<DKSpinLock> guard(this->lock);

	struct InsertObject
	{
		InsertObject(DKScene* s) : target(s) {}
		DKScene* target;
		void operator () (DKModel* model)
		{
			DKASSERT_DEBUG(target->sceneObjects.Contains(model) == false);
			model->scene = target;
			target->sceneObjects.Insert(model);
			DKASSERT_DEBUG(target->sceneObjects.Contains(model));

			target->AddSingleObject(model);
			size_t num = model->NumberOfChildren();
			for (size_t i = 0; i < num; ++i)
			{
				DKModel* m = model->ChildAtIndex(i);
				this->operator()(m);
			}
			model->OnAddedToScene();
		}
	};
	InsertObject(this)(obj);
	return true;
}

void DKScene::RemoveObject(DKModel* obj)
{
	DKASSERT_DEBUG(context && context->world);

	if (obj == NULL)
		return;
	if (obj->Parent())
		return;
	if (obj->scene != this)
		return;

	DKObject<DKModel> holder(obj); // prevent to removal

	DKCriticalSection<DKSpinLock> guard(this->lock);

	struct RemoveObject
	{
		RemoveObject(DKScene* s) : target(s) {}
		DKScene* target;
		void operator () (DKModel* model)
		{
			size_t num = model->NumberOfChildren();
			for (size_t i = 0; i < num; ++i)
			{
				DKModel* m = model->ChildAtIndex(i);
				this->operator()(m);
			}
			DKASSERT_DEBUG(target->sceneObjects.Contains(model));
			DKASSERT_DEBUG(model->scene == target);
			target->RemoveSingleObject(model);
			target->sceneObjects.Remove(model);
			model->scene = NULL;   // set scene to NULL before call OnRemovedFromScene().
			model->OnRemovedFromScene();
		}
	};
	RemoveObject(this)(obj);
}

bool DKScene::AddSingleObject(DKModel* obj)
{
	DKASSERT_DEBUG(context && context->world);
	DKASSERT_DEBUG(obj->Scene() == this);

	if (obj->type == DKModel::TypeMesh)
	{
		DKASSERT_DEBUG(dynamic_cast<DKMesh*>(obj) != NULL);
		DKMesh* mesh = static_cast<DKMesh*>(obj);
		DKASSERT_DEBUG(meshes.Contains(mesh) == false);
		meshes.Insert(mesh);
		return true;
	}
	else if (obj->type == DKModel::TypeCollision)
	{
		DKASSERT_DEBUG(dynamic_cast<DKCollisionObject*>(obj) != NULL);
		DKCollisionObject* col = static_cast<DKCollisionObject*>(obj);

		btCollisionObject* co = BulletCollisionObject(col);
		DKASSERT_DEBUG(co && co->getCollisionShape());
		DKCriticalSection<DKSpinLock> guard(context->lock);
		context->world->addCollisionObject(co);
	}
	return false;
}

void DKScene::RemoveSingleObject(DKModel* obj)
{
	DKASSERT_DEBUG(context && context->world);
	DKASSERT_DEBUG(obj->Scene() == this);

	if (obj->type == DKModel::TypeMesh)
	{
		DKASSERT_DEBUG(dynamic_cast<DKMesh*>(obj) != NULL);
		DKMesh* mesh = static_cast<DKMesh*>(obj);
		meshes.Remove(static_cast<DKMesh*>(mesh));
	}
	else if (obj->type == DKModel::TypeCollision)
	{
		DKASSERT_DEBUG(dynamic_cast<DKCollisionObject*>(obj) != NULL);
		DKCollisionObject* col = static_cast<DKCollisionObject*>(obj);

		btCollisionObject* co = BulletCollisionObject(col);
		DKASSERT_DEBUG(co && co->getCollisionShape());
		DKCriticalSection<DKSpinLock> guard(context->lock);
		context->world->removeCollisionObject(co);
	}
}

void DKScene::RemoveAllObjects(void)
{
	DKASSERT_DEBUG(context && context->world);

	DKCriticalSection<DKSpinLock> guard1(this->lock);
	DKCriticalSection<DKSpinLock> guard2(context->lock);

	for (int i = context->world->getNumCollisionObjects(); i > 0; --i)
	{
		btCollisionObject* obj = context->world->getCollisionObjectArray()[i - 1];
		context->world->removeCollisionObject(obj);
	}
	this->sceneObjects.EnumerateForward([this](const DKModel* obj)
	{
		DKModel* model = const_cast<DKModel*>(obj);
		model->scene = NULL;
		model->OnRemovedFromScene();
	});
	this->sceneObjects.Clear();
	this->meshes.Clear();
}

size_t DKScene::NumberOfSceneObjects(void) const
{
	return this->sceneObjects.Count();
}

void DKScene::Enumerate(BEnumerator* e) const
{
	DKCriticalSection<DKSpinLock> guard(lock);
	this->sceneObjects.EnumerateForward([e](const DKModel* m)
	{
		return e->Invoke(m);
	});
}

void DKScene::Enumerate(VEnumerator* e) const
{
	DKCriticalSection<DKSpinLock> guard(lock);
	this->sceneObjects.EnumerateForward([e](const DKModel* m)
	{
		return e->Invoke(m);
	});
}
