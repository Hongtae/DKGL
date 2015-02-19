import _dk_core as core
from .. import blendstate


def _pixelScaleFactor(renderer):
    w1,h1 = renderer.bounds[2:]
    w2,h2 = renderer.viewport[2:]
    return (w1 / w2, h1 / h2)

class _RTBaseline:
    def __init__(self, renderer, font, begin, color=core.Color(0, 0, 0), blend=blendstate.defaultAlpha):
        self.renderer = renderer
        self.font = font
        self.color = color
        self.blend = blend
        self.lines = []
        self.line_begin = begin.x
        
        self.scaleFactor = _pixelScaleFactor(renderer)
        self.line_height = font.lineHeight() * self.scaleFactor[1]

        self.position = core.Point(begin)

    def lineOffset(self):
        return self.line_height

    def getTextBaseline(self, text, pos):
        width = self.font.lineWidth(text) * self.scaleFactor[0]
        pos_begin = core.Point(pos.x, pos.y)
        pos_end = core.Point(pos_begin.x + width, pos_begin.y)
        pos.tuple = pos_end.tuple
        return pos_begin, pos_end

    def drawSingleLine(self, str):
        if str.endswith(('\r','\n','\r\n')):
            str = str.rstrip('\r\n')
            if str:
                begin, end = self.getTextBaseline(str, self.position)
                self.renderer.renderTextBaseline(begin, end, str, self.font, self.color, self.blend)
            self.position.x = self.line_begin
            self.position.y += self.lineOffset()
        elif str:
            begin, end = self.getTextBaseline(str, self.position)
            self.renderer.renderTextBaseline(begin, end, str, self.font, self.color, self.blend)

    def endline(self):
        self.lines.append('\n')
        for str in self.lines:
            self.drawSingleLine(str)
        self.lines = []

    def add(self, text, color):
        if self.color is not color:
            for str in self.lines:
                self.drawSingleLine(str)
            self.lines = []

        self.lines = text.splitlines(True)
        self.color = color
        if len(self.lines) > 1:
            for str in self.lines[:-1]:
                self.drawSingleLine(str)
            self.lines = self.lines[-1:]

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        for str in self.lines:
            self.drawSingleLine(str)
        self.lines = []


class _RTDownward:
    def lineOffset(self):
        return -super().lineOffset()

class _RTAlignRight:
    def getTextBaseline(self, text, pos):
        begin, end = super().getTextBaseline(text, pos)
        pos.tuple = begin.tuple
        return end, begin

class RenderTextBaselineRightUpward(_RTBaseline):
    pass

class RenderTextBaselineRightDownward(_RTDownward, _RTBaseline):
    pass

class RenderTextBaselineLeftUpward(_RTAlignRight, _RTBaseline):
    pass

class RenderTextBaselineLeftDownward(_RTAlignRight, _RTDownward, _RTBaseline):
    pass


TEXT_ALIGN_LEFT_UPWARD = 0
TEXT_ALIGN_LEFT_DOWNWARD = 1
TEXT_ALIGN_RIGHT_UPWARD = 2
TEXT_ALIGN_RIGHT_DOWNWARD = 3

def RenderTextClass(align):
    if align is TEXT_ALIGN_LEFT_UPWARD:
        return RenderTextBaselineRightUpward
    if align is TEXT_ALIGN_LEFT_DOWNWARD:
        return RenderTextBaselineRightDownward
    if align is TEXT_ALIGN_RIGHT_UPWARD:
        return RenderTextBaselineLeftUpward
    if align is TEXT_ALIGN_RIGHT_DOWNWARD:
        return RenderTextBaselineLeftDownward
    raise ValueError("invalid value")


def RenderTextBaseline(renderer, font, begin, align, color=core.Color(0,0,0), blend=blendstate.defaultAlpha):
    cls = RenderTextClass(align)
    return cls(renderer, font, begin, color, blend)
