import _dk_core as core
from collections import namedtuple

# drawing mode bitwise constants
DRAW_MESHES = 1
DRAW_MESH_BOUNDING_SPHERES = 1 << 1
DRAW_COLLISION_AABB = 1 << 2
DRAW_COLLISION_SHAPES = 1 << 3
DRAW_CONSTRAINTS = 1 << 4
DRAW_CONSTRAINT_LIMITS = 1 << 5
DRAW_CONTACT_POINTS = 1 << 6
DRAW_WIRE_FRAMES = 1 << 7
DRAW_SKELETAL_LINES = 1 << 8

RayResult = namedtuple('RayResult', ('collisionObject', 'fraction', 'hitPoint', 'hitNormal'))

class ClosestRayResultCallback:
    def __init__(self, rayBegin, rayEnd):
        self.rayBegin = rayBegin
        self.rayEnd = rayEnd
        self.rayResult = None

    def __call__(self, col, fraction, normal):
        s = 1.0 - fraction
        hitPoint = self.rayBegin * s + self.rayEnd * fraction
        self.rayResult = RayResult(col, fraction, hitPoint, normal)
        return fraction

class AllHitsRayResultCallback:
    def __init__(self, rayBegin, rayEnd):
        self.rayBegin = rayBegin
        self.rayEnd = rayEnd
        self.rayResults = []

    def __call__(self, col, fraction, normal):
        s = 1.0 - fraction
        hitPoint = self.rayBegin * s + self.rayEnd * fraction
        self.rayResults.append(RayResult(col, fraction, hitPoint, normal))
        return 1.0


class Scene(core.Scene):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setAmbientColor(core.Color(1, 1, 1, 1))
        self.drawMode = DRAW_MESHES
        self.lights = []

    @property
    def ambientColor(self):
        return super().ambientColor()
    
    @ambientColor.setter
    def ambientColor(self, value):
        super().setAmbientColor(value)

    def updateLights(self):
        super().setLights(*self.lights)


class DynamicsScene(Scene, core.DynamicsScene):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.fixedFrameRate = 0.0
        self.setGravity(core.Vector3(0,-10,0))

    @property
    def gravity(self):
        return super().gravity()

    @gravity.setter
    def gravity(self, value):
        super().setGravity(value)

    def needCollision(self, objA, objB):
        return True

    def needResponse(self, objA, objB):
        return True


# core.setDefaultClass(core.Scene, Scene)
# core.setDefaultClass(core.DynamicsScene, DynamicsScene)
