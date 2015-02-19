import _dk_core as core
import array
from .. import blendstate

class RenderContext:
    def __init__(self, renderer, transform, color, blend=blendstate.defaultOpaque):
        self.vertices = array.array('f')
        self.renderer = renderer
        self.transform = transform
        self.color = color
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


class RenderSolid3DTriangles(RenderContext):
    def add(self, p1, p2, p3):
        return super().add(p1.tuple + p2.tuple + p3.tuple, 3)

    def draw(self):
        self.renderer.renderSolid3DTriangles(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()


class RenderSolid3DTriangleStrip(RenderContext):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.last_point = (0.0, 0.0, 0.0)

    def addTriangle(self, p1, p2, p3):
        if len(self.vertices):
            super().add(self.last_point + p1.tuple, 2)
        self.last_point = p3.tuple
        return super().add(p1.tuple + p2.tuple + p3.tuple, 3)

    def add(self, p):
        self.last_point = p.tuple
        return super().add(self.last_point, 1)

    def draw(self):
        self.renderer.renderSolid3DTriangleStrip(self.vertices, self.count, self.transform, self.color, self.blend)
        self.last_point = (0.0, 0.0, 0.0)
        return super().draw()


class RenderSolid3DTriangleFan(RenderContext):
    def add(self, p):
        return super().add(p.tuple, 1)

    def draw(self):
        self.renderer.renderSolid3DTriangleFan(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()


class RenderSolid3DLines(RenderContext):
    def add(self, p1, p2):
        return super().add(p1.tuple + p2.tuple, 2)

    def draw(self):
        self.renderer.renderSolid3DLines(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()


class RenderSolid3DLineStrip(RenderContext):
    def addLine(self, p1, p2):
        if len(self.vertices):
            self.draw()
        return super().add(p1.tuple + p2.tuple, 2)

    def add(self, p):
        return super().add(p.tuple, 1)

    def draw(self):
        self.renderer.renderSolid3DLineStrip(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()


class RenderSolid3DPoints(RenderContext):
    def add(self, p):
        return super().add(p.tuple, 1)

    def draw(self):
        self.renderer.renderSolid3DPoints(self.vertices, self.count, self.transform, self.color, self.blend)
        return super().draw()
