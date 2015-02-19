import _dk_core as core
from .. import blendstate
from . import view
from . import font

# text alignment
ALIGN_TOP_LEFT = font.ALIGN_TOP_LEFT
ALIGN_TOP = font.ALIGN_TOP
ALIGN_TOP_RIGHT = font.ALIGN_TOP_RIGHT
ALIGN_LEFT = font.ALIGN_LEFT
ALIGN_CENTER = font.ALIGN_CENTER
ALIGN_RIGHT = font.ALIGN_RIGHT
ALIGN_BOTTOM_LEFT = font.ALIGN_BOTTOM_LEFT
ALIGN_BOTTOM = font.ALIGN_BOTTOM
ALIGN_BOTTOM_RIGHT = font.ALIGN_BOTTOM_RIGHT

# text line-break mode
LINE_BREAK_CLIPPING = font.LINE_BREAK_CLIPPING
LINE_BREAK_TRUNCATING_HEAD = font.LINE_BREAK_TRUNCATING_HEAD
LINE_BREAK_TRUNCATING_MIDDLE = font.LINE_BREAK_TRUNCATING_MIDDLE
LINE_BREAK_TRUNCATING_TAIL = font.LINE_BREAK_TRUNCATING_TAIL


class Label(view.View):

    fontAttributes = font.attributes(12)

    textColor = core.Color(0.0, 0.0, 0.0)
    outlineColor = None
    textTransform = core.Matrix3()
    scaleToFit = False

    leftMargin = 0
    topMargin = 0
    rightMargin = 0
    bottomMargin = 0

    def __init__(self, text='', *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.text = str(text)
        self.align = ALIGN_CENTER
        self.linebreak = LINE_BREAK_TRUNCATING_TAIL

    def onLoaded(self):
        super().onLoaded()
        if not self.font:
            self.font = font.loadUIFont(self.fontAttributes, self.scaleFactor)

    def onUnload(self):
        self.font = None
        super().onUnload()

    def onRender(self, renderer):
        super().onRender(renderer)

        if self.font:
            bounds = self.contentBounds()
            bounds.x += self.leftMargin
            bounds.y += self.bottomMargin
            bounds.width -= self.leftMargin + self.rightMargin
            bounds.height -= self.topMargin + self.bottomMargin

            font.drawText(renderer,
                          bounds,
                          self.text,
                          self.font,
                          self.textColor,
                          self.outlineColor,
                          scaleToFit=self.scaleToFit,
                          align=self.align,
                          linebreak=self.linebreak,
                          blend=blendstate.defaultAlpha)
