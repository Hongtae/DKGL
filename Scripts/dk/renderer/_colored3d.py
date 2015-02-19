import _dk_core as core
import array
from .. import blendstate

class _RenderContext:
    def __init__(self, renderer, transform, blend=blendstate.defaultOpaque):
        self.vertices = array.array('f')
        self.renderer = renderer
        self.transform = transform
        self.blend = blend
        self.count = 0

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.draw()

    def add(self, verts, count):
        self.vertices.extend(verts)
        self.count += count

    def draw(self):
        self.vertices = array.array('f')
        self.count = 0


class RenderColored3DTriangles(_RenderContext):
    def add(self, p1, c1, p2, c2, p3, c3):
        return super().add(p1.tuple + c1.tuple + p2.tuple + c2.tuple + p3.tuple + c3.tuple, 3)

    def draw(self):
        self.renderer.renderColored3DTriangles(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()


class RenderColored3DTriangleStrip(_RenderContext):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.last_point = (0.0,) * 7   # vec3 + color = 7 floats

    def addTriangle(self, p1, c1, p2, c2, p3, c3):
        if len(self.vertices):
            super().add(self.last_point + p1.tuple + c1.tuple, 2)
        self.last_point = p3.tuple + c3.tuple
        return super().add(p1.tuple + c1.tuple + p2.tuple + c2.tuple + p3.tuple + c3.tuple, 3)

    def add(self, p, c):
        self.last_point = p.tuple + c.tuple
        return super().add(self.last_point, 1)

    def draw(self):
        self.renderer.renderColored3DTriangleStrip(self.vertices, self.count, self.transform, self.color, self.blend)
        self.last_point = (0.0,) * 7   # vec3 + color = 7 floats
        return super().draw()


class RenderColored3DTriangleFan(_RenderContext):
    def add(self, p, c):
        return super().add(p.tuple + c.tuple, 1)

    def draw(self):
        self.renderer.renderColored3DTriangleFan(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()


class RenderColored3DLines(_RenderContext):
    def add(self, p1, c1, p2, c2):
        return super().add(p1.tuple + c1.tuple + p2.tuple + c2.tuple, 2)

    def draw(self):
        self.renderer.renderColored3DLines(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()


class RenderColored3DLineStrip(_RenderContext):
    def addLine(self, p1, c1, p2, c2):
        if len(self.vertices):
            self.draw()
        return super().add(p1.tuple + c1.tuple + p2.tuple + c2.tuple, 2)

    def add(self, p, c):
        return super().add(p.tuple + c.tuple, 1)

    def draw(self):
        self.renderer.renderColored3DLineStrip(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()


class RenderColored3DPoints(_RenderContext):
    def add(self, p, c):
        return super().add(p.tuple + c.tuple, 1)

    def draw(self):
        self.renderer.renderColored3DPoints(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()
