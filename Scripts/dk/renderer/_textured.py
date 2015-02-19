import _dk_core as core
import array
from .. import blendstate

class _RenderContext:
    def __init__(self, renderer, texture, sampler=None, color=core.Color(1,1,1,1), blend=blendstate.defaultOpaque):
        self.vertices = array.array('f')
        self.renderer = renderer
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


class RenderTexturedRects(_RenderContext):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.transforms = array.array('f')
        self.tex_rects = array.array('f')
        self.tex_trans = array.array('f')

    def add(self, rect, transform, tex_rect=core.Rect(0,0,1,1), tex_trans=core.Matrix3()):
        if rect.width > 0 and rect.height > 0:
            self.transforms.extend(transform.tuple)
            self.tex_rects.extend(tex_rect.tuple)
            self.tex_trans.extend(tex_trans.tuple)
            return super().add(rect.tuple, 1)

    def draw(self):
        self.renderer.renderTexturedRects(self.vertices, self.transforms, self.tex_rects, self.tex_trans, self.count, self.texture, self.sampler, self.color, self.blend)
        self.transforms = array.array('f')
        self.tex_rects = array.array('f')
        self.tex_trans = array.array('f')
        return super().draw()


class RenderTexturedQuads(_RenderContext):
    def add(self, lb, lt, rt, rb, lb_tex=core.Point(0,0), lt_tex=core.Point(0,1), rt_tex=core.Point(1,1), rb_tex=core.Point(1,0)):
        return super().add(lb.tuple + lb_tex.tuple + lt.tuple + lt_tex.tuple + rt.tuple + rt_tex.tuple + rb.tuple + rb_tex.tuple, 4)

    def draw(self):
        self.renderer.renderTexturedQuads(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTexturedTriangles(_RenderContext):
    def add(self, p1, t1, p2, t2, p3, t3):
        return super().add(p1.tuple + t1.tuple + p2.tuple + t2.tuple + p3.tuple + t3.tuple, 3)

    def draw(self):
        self.renderer.renderTexturedTriangles(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTexturedTriangleStrip(_RenderContext):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.last_point = (0.0,) * 4    # point(pos) + point(uv) = 4

    def addTriangle(self, p1, t1, p2, t2, p3, t3):
        if len(self.vertices):
            super().add(self.last_point + p1.tuple + t1.tuple, 2)
        self.last_point = p3.tuple + t3.tuple
        return super().add(p1.tuple + t1.tuple + p2.tuple + t2.tuple + p3.tuple + t3.tuple, 3)

    def add(self, p, t):
        self.last_point = p.tuple + t.tuple
        return super().add(self.last_point, 1)

    def draw(self):
        self.renderer.renderTexturedTriangleStrip(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        self.last_point = (0.0,) * 4    # point(pos) + point(uv) = 4
        return super().draw()


class RenderTexturedTriangleFan(_RenderContext):
    def add(self, p, t):
        return super().add(p.tuple + t.tuple, 1)

    def draw(self):
        self.renderer.renderTexturedTriangleFan(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTexturedLines(_RenderContext):
    def add(self, p1, t1, p2, t2):
        return super().add(p1.tuple + t1.tuple + p2.tuple + t2.tuple, 2)

    def draw(self):
        self.renderer.renderTexturedLines(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTexturedLineStrip(_RenderContext):
    def addLine(self, p1, t1, p2, t2):
        if len(self.vertices):
            self.draw()
        return super().add(p1.tuple + t1.tuple + p2.tuple + t2.tuple, 2)

    def add(self, p, t):
        return super().add(p.tuple + t.tuple, 1)

    def draw(self):
        self.renderer.renderTexturedLineStrip(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTexturedPoints(_RenderContext):
    def add(self, p, t):
        return super().add(p.tuple + t.tuple, 1)

    def draw(self):
        self.renderer.renderTexturedPoints(self.vertices, self.count, self.texture, self.sampler, self.color, self.blend)
        return super().draw()


class RenderTexturedEllipses(_RenderContext):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.tex_bounds = array.array('f')

    def add(self, rect, tex_rect=core.Rect(0.0, 0.0, 1.0, 1.0)):
        self.tex_bounds.extend(tex_rect.tuple)
        return super().add(rect.tuple, 1)

    def draw(self):
        self.renderer.renderTexturedEllipses(self.vertices, self.tex_bounds, self.count, self.texture, self.sampler, self.color, self.blend)
        self.tex_bounds = array.array('f')
        return super().draw()
