import _dk_core as core


TYPE_UNKNOWN = 0
TYPE_DIRECTIONAL = 1
TYPE_POINT = 2

class Light(core.Light):
    def __init__(self, type=TYPE_DIRECTIONAL, pos=core.Vector3(0, 1, 0), color=core.Color(1, 1, 1, 1)):
        core.Light.__init__(self, type, pos, color)
        self.constAttenuation = 1
        self.linearAttenuation = 0
        self.quadraticAttenuation = 0

    def direction(self):
        if self.type is TYPE_DIRECTIONAL:
            dir = -core.Vector3(self.position)
            dir.normalize()
            return dir

    def setDirection(self, dir):
        assert self.type is TYPE_DIRECTIONAL
        pos = -core.Vector3(dir)
        pos.normalize()
        self.position = pos

    def attenuation(self, pos):
        if self.type is TYPE_DIRECTIONAL:
            return 1.0
        distance = (self.position - pos).length()
        qa = self.quadraticAttenuation * distance * distance
        la = self.linearAttenuation * distance
        return 1.0 / (self.constAttenuation + la + qa)

    def setAttenuation(self, const, linear, quadratic):
        self.constAttenuation = const
        self.linearAttenuation = linear
        self.quadraticAttenuation = quadratic

    def isPointLight(self):
        return self.type is TYPE_POINT

    def isDirectionalLight(self):
        return self.type is TYPE_DIRECTIONAL


def directional(dir, color):
    return Light(TYPE_DIRECTIONAL, -dir, color)


def point(pos, color):
    return Light(TYPE_POINT, pos, color)
