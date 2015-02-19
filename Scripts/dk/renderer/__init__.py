import _dk_core as core

from .. import blendstate
from .. import color
from . import _solid
from . import _textured
from . import _text
from . import _solid3d
from . import _colored3d
from . import _textured3d
from . import _shape


class Renderer(core.Renderer):

    SHAPE_FACES = _shape.SHAPE_FACES
    SHAPE_EDGES = _shape.SHAPE_EDGES

    TEXT_ALIGN_LEFT_UPWARD = _text.TEXT_ALIGN_LEFT_UPWARD
    TEXT_ALIGN_LEFT_DOWNWARD = _text.TEXT_ALIGN_LEFT_DOWNWARD
    TEXT_ALIGN_RIGHT_UPWARD = _text.TEXT_ALIGN_RIGHT_UPWARD
    TEXT_ALIGN_RIGHT_DOWNWARD = _text.TEXT_ALIGN_RIGHT_DOWNWARD

    def contextForSolidRects(self, color, blend=blendstate.defaultOpaque):
        return _solid.RenderSolidRects(self, color, blend)

    def contextForSolidQuads(self, color, blend=blendstate.defaultOpaque):
        return _solid.RenderSolidQuads(self, color, blend)

    def contextForSolidTriangles(self, color, blend=blendstate.defaultOpaque):
        return _solid.RenderSolidTriangles(self, color, blend)

    def contextForSolidTriangleStrip(self, color, blend=blendstate.defaultOpaque):
        return _solid.RenderSolidTriangleStrip(self, color, blend)

    def contextForSolidTriangleFan(self, color, blend=blendstate.defaultOpaque):
        return _solid.RenderSolidTriangleFan(self, color, blend)

    def contextForSolidLines(self, color, blend=blendstate.defaultOpaque):
        return _solid.RenderSolidLines(self, color, blend)

    def contextForSolidLineStrip(self, color, blend=blendstate.defaultOpaque):
        return _solid.RenderSolidLineStrip(self, color, blend)

    def contextForSolidPoints(self, color, blend=blendstate.defaultOpaque):
        return _solid.RenderSolidPoints(self, color, blend)

    def contextForSolidEllipses(self, color, blend=blendstate.defaultOpaque):
        return _solid.RenderSolidEllipses(self, color, blend)

    def contextForTexturedRects(self, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured.RenderTexturedRects(self, texture, sampler, color, blend)

    def contextForTexturedQuads(self, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured.RenderTexturedQuads(self, texture, sampler, color, blend)

    def contextForTexturedTriangles(self, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured.RenderTexturedTriangles(self, texture, sampler, color, blend)

    def contextForTexturedTriangleStrip(self, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured.RenderTexturedTriangleStrip(self, texture, sampler, color, blend)

    def contextForTexturedTriangleFan(self, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured.RenderTexturedTriangleFan(self, texture, sampler, color, blend)

    def contextForTexturedLines(self, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured.RenderTexturedLines(self, texture, sampler, color, blend)

    def contextForTexturedLineStrip(self, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured.RenderTexturedLineStrip(self, texture, sampler, color, blend)

    def contextForTexturedPoints(self, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured.RenderTexturedPoints(self, texture, sampler, color, blend)

    def contextForTexturedEllipses(self, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured.RenderTexturedEllipses(self, texture, sampler, color, blend)

    def contextForTextBaseline(self, font, begin, align, color=color.black, blend=blendstate.defaultAlpha):
        return _text.RenderTextBaseline(self, font, begin, align, color, blend)

    def contextForSolid3DTriangles(self, transform, color, blend=blendstate.defaultOpaque):
        return _solid3d.RenderSolid3DTriangles(self, transform, color, blend)

    def contextForSolid3DTriangleStrip(self, transform, color, blend=blendstate.defaultOpaque):
        return _solid3d.RenderSolid3DTriangleStrip(self, transform, color, blend)

    def contextForSolid3DTriangleFan(self, transform, color, blend=blendstate.defaultOpaque):
        return _solid3d.RenderSolid3DTriangleFan(self, transform, color, blend)

    def contextForSolid3DLines(self, transform, color, blend=blendstate.defaultOpaque):
        return _solid3d.RenderSolid3DLines(self, transform, color, blend)

    def contextForSolid3DLineStrip(self, transform, color, blend=blendstate.defaultOpaque):
        return _solid3d.RenderSolid3DLineStrip(self, transform, color, blend)

    def contextForSolid3DPoints(self, transform, color, blend=blendstate.defaultOpaque):
        return _solid3d.RenderSolid3DPoints(self, transform, color, blend)

    def contextForColored3DTriangles(self, transform, blend=blendstate.defaultOpaque):
        return _colored3d.RenderColored3DTriangles(self, transform, blend)

    def contextForColored3DTriangleStrip(self, transform, blend=blendstate.defaultOpaque):
        return _colored3d.RenderColored3DTriangleStrip(self, transform, blend)

    def contextForColored3DTriangleFan(self, transform, blend=blendstate.defaultOpaque):
        return _colored3d.RenderColored3DTriangleFan(self, transform, blend)

    def contextForColored3DLines(self, transform, blend=blendstate.defaultOpaque):
        return _colored3d.RenderColored3DLines(self, transform, blend)

    def contextForColored3DLineStrip(self, transform, blend=blendstate.defaultOpaque):
        return _colored3d.RenderColored3DLineStrip(self, transform, blend)

    def contextForColored3DPoints(self, transform, blend=blendstate.defaultOpaque):
        return _colored3d.RenderColored3DPoints(self, transform, blend)

    def contextForTextured3DTriangles(self, transform, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured3d.RenderTextured3DTriangles(self, transform, texture, sampler, color, blend)

    def contextForTextured3DTriangleStrip(self, transform, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured3d.RenderTextured3DTriangleStrip(self, transform, texture, sampler, color, blend)

    def contextForTextured3DTriangleFan(self, transform, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured3d.RenderTextured3DTriangleFan(self, transform, texture, sampler, color, blend)

    def contextForTextured3DLines(self, transform, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured3d.RenderTextured3DLines(self, transform, texture, sampler, color, blend)

    def contextForTextured3DLineStrip(self, transform, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured3d.RenderTextured3DLineStrip(self, transform, texture, sampler, color, blend)

    def contextForTextured3DPoints(self, transform, texture, sampler=None, color=color.white, blend=blendstate.defaultOpaque):
        return _textured3d.RenderTextured3DPoints(self, transform, texture, sampler, color, blend)

    def contextForCollisionShape(self, camera, mode=_shape.SHAPE_FACES):
        return _shape.RenderCollisionShape(self, camera, mode)


core.setDefaultClass(core.Renderer, Renderer)
