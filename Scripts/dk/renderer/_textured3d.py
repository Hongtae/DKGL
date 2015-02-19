import _dk_core as core
import array
from .. import blendstate

class _RenderContext:
    def __init__(self, renderer, transform, texture, sampler=None, color=core.Color(1,1,1,1), blend=blendstate.defaultOpaque):
        self.vertices = array.array('f')
        self.renderer = renderer
        self.transform = transform
        self.color = color
        self.blend = blend
        self.texture = texture
        self.sampler = sampler
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


class RenderTextured3DTriangles(_RenderContext):
    def add(self, p1, t1, p2, t2, p3, t3):
        return super().add(p1.tuple + t1.tuple + p2.tuple + t2.tuple + p3.tuple + t3.tuple, 3)

    def draw(self):
        self.renderer.renderTextured3DTriangles(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTextured3DTriangleStrip(_RenderContext):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.last_point = (0.0,) * 5    # vec3(pos) + point(uv)

    def addTriangle(self, p1, t1, p2, t2, p3, t3):
        if len(self.vertices):
            super().add(self.last_point + p1.tuple + t1.tuple, 2)
        self.last_point = p3.tuple + t3.tuple
        return super().add(p1.tuple + t1.tuple + p2.tuple + t2.tuple + p3.tuple + t3.tuple, 3)

    def add(self, p, t):
        self.last_point = p.tuple + t.tuple
        return super().add(self.last_point, 1)

    def draw(self):
        self.renderer.renderTextured3DTriangleStrip(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        self.last_point = (0.0,) * 5    # vec3(pos) + point(uv)
        return super().draw()


class RenderTextured3DTriangleFan(_RenderContext):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def add(self, p, t):
        return super().add(p.tuple + t.tuple, 1)

    def draw(self):
        self.renderer.renderTextured3DTriangleFan(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTextured3DLines(_RenderContext):
    def add(self, p1, p2, t1, t2):
        return super().add(p1.tuple + t1.tuple + p2.tuple + t2.tuple, 2)

    def draw(self):
        self.renderer.renderTextured3DLines(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTextured3DLineStrip(_RenderContext):
    def addLine(self, p1, p2, t1, t2):
        if len(self.vertices):
            self.draw()
        return super().add(p1.tuple + t1.tuple + p2.tuple + t2.tuple, 2)

    def add(self, p, t):
        return super().add(p.tuple + t.tuple, 1)

    def draw(self):
        self.renderer.renderTextured3DLineStrip(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTextured3DPoints(_RenderContext):
    def add(self, p, t):
        return super().add(p.tuple + t.tuple, 1)

    def draw(self):
        self.renderer.renderTextured3DPoints(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()

