from _dk_core import MultiSphereShape
from _dk_core import Vector3
from collections import namedtuple

_Sphere = namedtuple('_Sphere', ('center','radius'))

class ShapeBuilder:
    def __init__(self):
        self.spheres = []

    def addSphere(self, center, radius):
        assert isinstance(center, Vector3)
        s = _Sphere(center, radius)
        self.spheres.append(s)

    def finalize(self):
        shape = MultiSphereShape(self.spheres)
        self.spheres = []
        return shape

