import _dk_core as core

UP_AXIS_LEFT = 0
UP_AXIS_TOP = 1
UP_AXIS_FORWARD = 2

TYPE_CUSTOM = 0
TYPE_EMPTY = 1
TYPE_COMPOUND = 2
TYPE_BOX = 3
TYPE_CAPSULE = 4
TYPE_CYLINDER = 5
TYPE_CONE = 6
TYPE_SPHERE = 7
TYPE_MULTI_SPHERE = 8
TYPE_CONVEX_HULL = 9
TYPE_STATIC_PLANE = 10
TYPE_STATIC_TRIANGLE_MESH = 11



CollisionShape = core.CollisionShape
CompoundShape = core.CompoundShape

ConcaveShape = core.ConcaveShape
StaticPlaneShape = core.StaticPlaneShape
StaticTriangleMeshShape = core.StaticTriangleMeshShape

ConvexShape = core.ConvexShape
CapsuleShape = core.CapsuleShape
ConeShape = core.ConeShape
CylinderShape = core.CylinderShape
MultiSphereShape = core.MultiSphereShape

PolyhedralConvexShape = core.PolyhedralConvexShape
BoxShape = core.BoxShape
ConvexHullShape = core.ConvexHullShape
SphereShape = core.SphereShape


from .convexhull import ShapeBuilder as ConvexHullBuilder
from .multisphere import ShapeBuilder as MultiSphereBuilder