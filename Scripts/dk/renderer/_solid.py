import _dk_core as core
import array
from .. import blendstate

class _RenderContext:
    def __init__(self, renderer, color, blend=blendstate.defaultOpaque):
        self.vertices = array.array('f')
        self.renderer = renderer
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

# solid color drawing
class RenderSolidRects(_RenderContext):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.transforms = array.array('f')

    def add(self, rect, transform=core.Matrix3()):
        if rect.width > 0 and rect.height > 0:
            self.transforms.extend(transform.tuple)
            return super().add(rect.tuple, 1)

    def draw(self):
        self.renderer.renderSolidRects(self.vertices, self.transforms, self.count, self.color, self.blend)
        self.transforms = array.array('f')
        return super().draw()


class RenderSolidQuads(_RenderContext):
    def add(self, lb, lt, rt, rb):
        return super().add(lb.tuple + lt.tuple + rt.tuple + rb.tuple, 4)

    def draw(self):
        self.renderer.renderSolidQuads(self.vertices, self.count, self.color, self.blend)
        return super().draw()


class RenderSolidTriangles(_RenderContext):
    def add(self, p1, p2, p3):
        return super().add(p1.tuple + p2.tuple + p3.tuple, 3)

    def draw(self):
        self.renderer.renderSolidTriangles(self.vertices, self.count, self.color, self.blend)
        return super().draw()


class RenderSolidTriangleStrip(_RenderContext):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.last_point = (0.0, 0.0)

    def addTriangle(self, p1, p2, p3):
        if len(self.vertices):
            super().add(self.last_point + p1.tuple, 2)
        self.last_point = p3.tuple
        return super().add(p1.tuple + p2.tuple + p3.tuple, 3)

    def add(self, p):
        self.last_point = p.tuple
        return super().add(self.last_point, 1)

    def draw(self):
        self.renderer.renderSolidTriangleStrip(self.vertices, self.count, self.color, self.blend)
        self.last_point = (0.0, 0.0)
        return super().draw()


class RenderSolidTriangleFan(_RenderContext):
    def add(self, p):
        return super().add(p.tuple, 1)

    def draw(self):
        self.renderer.renderSolidTriangleFan(self.vertices, self.count, self.color, self.blend)
        return super().draw()


class RenderSolidLines(_RenderContext):
    def add(self, p1, p2):
        return super().add(p1.tuple + p2.tuple, 2)

    def draw(self):
        self.renderer.renderSolidLines(self.vertices, self.count, self.color, self.blend)
        return super().draw()


class RenderSolidLineStrip(_RenderContext):
    def addLine(self, p1, p2):
        if len(self.vertices):
            self.draw()
        return super().add(p1.tuple + p2.tuple, 2)

    def add(self, p):
        return super().add(p.tuple, 1)

    def draw(self):
        self.renderer.renderSolidLineStrip(self.vertices, self.count, self.color, self.blend)
        return super().draw()


class RenderSolidPoints(_RenderContext):
    def add(self, p):
        return super().add(p.tuple, 1)

    def draw(self):
        self.renderer.renderSolidPoints(self.vertices, self.count, self.color, self.blend)
        return super().draw()


class RenderSolidEllipses(_RenderContext):
    def add(self, rect):
        return super().add(rect.tuple, 1)

    def draw(self):
        self.renderer.renderSolidEllipses(self.vertices, self.count, self.color, self.blend)
        return super().draw()
