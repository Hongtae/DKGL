import _dk_core as core


class Camera(core.Camera):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def setView(self, pos, dir, up = core.Vector3(0.0, 1.0, 0.0)):
        super().setView(pos, dir, up)

    def setPerspective(self, fov, aspect, near, far):
        assert fov > 0
        assert aspect > 0
        assert near > 0
        assert far > 0
        assert far > near
        super().setPerspective(fov, aspect, near, far)

    def setOrthographic(self, width, height, near, far):
        assert width > 0
        assert height > 0
        assert far > near
        super().setOrthographic(width, height, near, far)

    def position(self):
        return super().position()

    def direction(self):
        return super().direction()

    def up(self):
        return super().up()

    def isPerspective(self):
        return super().isPerspective()

    def isOrthographic(self):
        return super().isOrthographic()

    def isPointInside(self, point):
        return super().isPointInside(point)

    def isSphereInside(self, center, radius):
        return super().isSphereInside(center, radius)

    def viewMatrix(self):
        return super().viewMatrix()

    def setViewMatrix(self, mat):
        super().setViewMatrix(mat)

    def projectionMatrix(self):
        return super().projectionMatrix()

    def setProjectionMatrix(self, mat):
        return super().setProjectionMatrix(mat)

    def viewProjectionMatrix(self):
        return super().viewProjectionMatrix()

    def setViewProjectionMatrix(self, view, proj):
        return super().setViewProjectionMatrix(view, proj)

