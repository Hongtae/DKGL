import _dk_core as core

# interpolation type
CATMULL_ROM = 0
UNIFORM_CUBIC = 1
HERMITE = 2
BEZIER = 3


Spline = core.Spline

class Spline1(core.Spline):
    pass

class Spline2:
    '''spline for Vector2'''
    def __init__(self, p0, p1, p2, p3):
        self.p0 = p0
        self.p1 = p1
        self.p2 = p2
        self.p3 = p3

    def interpolate(self, t, type):
        x = Spline.splineInterpolate(self.p0.x, self.p1.x, self.p2.x, self.p3.x, t, type)
        y = Spline.splineInterpolate(self.p0.y, self.p1.y, self.p2.y, self.p3.y, t, type)
        return core.Vector2(x, y)

class Spline3:
    '''spline for Vector3'''
    def __init__(self, p0, p1, p2, p3):
        self.p0 = p0
        self.p1 = p1
        self.p2 = p2
        self.p3 = p3

    def interpolate(self, t, type):
        x = Spline.splineInterpolate(self.p0.x, self.p1.x, self.p2.x, self.p3.x, t, type)
        y = Spline.splineInterpolate(self.p0.y, self.p1.y, self.p2.y, self.p3.y, t, type)
        z = Spline.splineInterpolate(self.p0.z, self.p1.z, self.p2.z, self.p3.z, t, type)
        return core.Vector3(x, y, z)

class Spline4:
    '''spline for Vector4'''
    def __init__(self, p0, p1, p2, p3):
        self.p0 = p0
        self.p1 = p1
        self.p2 = p2
        self.p3 = p3

    def interpolate(self, t, type):
        x = Spline.splineInterpolate(self.p0.x, self.p1.x, self.p2.x, self.p3.x, t, type)
        y = Spline.splineInterpolate(self.p0.y, self.p1.y, self.p2.y, self.p3.y, t, type)
        z = Spline.splineInterpolate(self.p0.z, self.p1.z, self.p2.z, self.p3.z, t, type)
        w = Spline.splineInterpolate(self.p0.w, self.p1.w, self.p2.w, self.p3.w, t, type)
        return core.Vector4(x, y, z, w)


def splineInterpolate(v1, v2, v3, v4, t, type):
    return core.Spline.splineInterpolate(v1, v2, v3, v4, t, type)
