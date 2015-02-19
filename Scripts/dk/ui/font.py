import _dk_core as core
from collections import namedtuple
from . import resource
from .. import blendstate
from math import floor

DEFAULT_FILENAME = 'BitstreamVeraSans.ttf'
DEFAULT_OUTLINE = 1.0
DEFAULT_DPI = (72, 72)


UniformScaleAttr = namedtuple('UniformScaleAttr', 'file, point, embolden, outline, dpi, kerning, bitmap')
FontAttr = namedtuple('FontAttr', 'attr, scale')


class UIFont:
    def __init__(self, textFont, outlineFont):
        self.textFont = textFont
        self.outlineFont = outlineFont

    def lineWidth(self, text):
        return self.textFont.lineWidth(text)

    def lineHeight(self):
        return self.textFont.lineHeight()

    @property
    def baseline(self):
        return self.textFont.baseline

    @property
    def width(self):
        return self.textFont.width

    @property
    def height(self):
        return self.textFont.height



def attributes(point, embolden=0, outline=1, dpi=DEFAULT_DPI, kerning=True, bitmap=False, file=''):
    return UniformScaleAttr(file, point, embolden, outline, dpi, kerning, bitmap)


def textFont(fontAttr, scale):
    assert isinstance(fontAttr, UniformScaleAttr)
    attr = FontAttr(fontAttr, scale)
    try:
        return resource.textFonts[attr]
    except KeyError:
        file = fontAttr.file
        if file is '':
            file = DEFAULT_FILENAME
        fontData = resource.pool.loadResourceData(file)
        if fontData:
            fontClass = core.defaultClass(core.Font)
            font = fontClass(fontData,
                             point=fontAttr.point,
                             embolden=fontAttr.embolden * attr.scale,
                             outline=0,
                             dpi=tuple(int(x * attr.scale) for x in fontAttr.dpi),
                             enableKerning=fontAttr.kerning,
                             forceBitmap=fontAttr.bitmap)
            resource.textFonts[attr] = font
            return resource.textFonts[attr]
        else:
            raise FileNotFoundError


def outlineFont(fontAttr, scale):
    assert isinstance(fontAttr, UniformScaleAttr)
    attr = FontAttr(fontAttr, scale)
    try:
        return resource.outlineFonts[attr]
    except KeyError:
        file = fontAttr.file
        if file is '':
            file = DEFAULT_FILENAME
        fontData = resource.pool.loadResourceData(file)
        if fontData:
            fontClass = core.defaultClass(core.Font)
            font = fontClass(fontData,
                             point=fontAttr.point,
                             embolden=fontAttr.embolden * attr.scale,
                             outline=fontAttr.outline * attr.scale,
                             dpi=tuple(int(x * attr.scale) for x in fontAttr.dpi),
                             enableKerning=fontAttr.kerning,
                             forceBitmap=fontAttr.bitmap)
            resource.outlineFonts[attr] = font
            return resource.outlineFonts[attr]
        else:
            raise FileNotFoundError


def loadUIFont(fontAttr, scale):
    assert isinstance(fontAttr, UniformScaleAttr)
    if fontAttr.point > 0:
        tf = textFont(fontAttr, scale)
        of = None
        if fontAttr.outline > 0:
            of = outlineFont(fontAttr, scale)
        return UIFont(tf, of)


#text-align functions
def ALIGN_TOP_LEFT(frame, width, height, baseline):
    x = 0
    y = frame.height - height + baseline
    return x + frame.x, y + frame.y

def ALIGN_TOP(frame, width, height, baseline):
    x = (frame.width - width) * 0.5
    y = frame.height - height + baseline
    return x + frame.x, y + frame.y

def ALIGN_TOP_RIGHT(frame, width, height, baseline):
    x = frame.width - width
    y = frame.height - height + baseline
    return x + frame.x, y + frame.y

def ALIGN_LEFT(frame, width, height, baseline):
    x = 0
    y = (frame.height - height + baseline) * 0.5
    return x + frame.x, y + frame.y

def ALIGN_CENTER(frame, width, height, baseline):
    x = (frame.width - width) * 0.5
    y = (frame.height - height + baseline) * 0.5
    return x + frame.x, y + frame.y

def ALIGN_RIGHT(frame, width, height, baseline):
    x = frame.width - width
    y = (frame.height - height + baseline) * 0.5
    return x + frame.x, y + frame.y

def ALIGN_BOTTOM_LEFT(frame, width, height, baseline):
    x = 0
    y = baseline
    return x + frame.x, y + frame.y

def ALIGN_BOTTOM(frame, width, height, baseline):
    x = (frame.width - width) * 0.5
    y = baseline
    return x + frame.x, y + frame.y

def ALIGN_BOTTOM_RIGHT(frame, width, height, baseline):
    x = frame.width - width
    y = baseline
    return x + frame.x, y + frame.y


#text line-break functions
def LINE_BREAK_CLIPPING(frame, font, text):
    return text

def LINE_BREAK_TRUNCATING_HEAD(frame, font, text):
    width = font.lineWidth(text)
    if width > frame.width:
        ellipsis = '...'
        if font.lineWidth(ellipsis) > frame.width:
            return ''
        for i in range(1, len(text)+1):
            text2 = ellipsis + text[i:]
            width = font.lineWidth(text2)
            if width <= frame.width:
                return text2
        return ''
    return text

def LINE_BREAK_TRUNCATING_TAIL(frame, font, text):
    width = font.lineWidth(text)
    if width > frame.width:
        ellipsis = '...'
        if font.lineWidth(ellipsis) > frame.width:
            return ''
        for i in range(len(text)-1, -1, -1):
            text2 = text[:i] + ellipsis
            width = font.lineWidth(text2)
            if width <= frame.width:
                return text2
        return ''
    return text

def LINE_BREAK_TRUNCATING_MIDDLE(frame, font, text):
    width = font.lineWidth(text)
    if width > frame.width:
        ellipsis = '...'
        if font.lineWidth(ellipsis) > frame.width:
            return ''

        half = len(text)//2
        head = text[:half]
        tail = text[half:]

        while len(head) > 0 or len(tail) > 0:
            tail = tail[1:]
            text = head + ellipsis + tail
            width = font.lineWidth(text)
            if width <= frame.width:
                return text

            head = head[:-1]
            text = head + ellipsis + tail
            width = font.lineWidth(text)
            if width <= frame.width:
                return text

        return ''
    return text

def drawText(renderer,
             frame,
             text,
             font,
             textColor = core.Color(1.0, 1.0, 1.0, 1.0),
             outlineColor = core.Color(0, 0, 0, 0.5),
             scaleToFit=False,
             align=ALIGN_CENTER,
             alignToPixel=True,
             linebreak=LINE_BREAK_TRUNCATING_TAIL,
             blend=blendstate.defaultAlpha):

    textFont = None
    outlineFont = None
    if font:
        if isinstance(font, UIFont):
            textFont = font.textFont
            outlineFont = font.outlineFont
        elif isinstance(font, core.Font):
            textFont = font
        elif isinstance(font, (tuple, list)):
            c = len(font)
            if c > 0:
                textFont = font[0]
            if c > 1:
                outlineFont = font[1]
        else:
            raise TypeError('font argument must be Font or two Font objects tuple.')

    layoutFont = textFont if textFont else outlineFont

    if len(text) > 0 and frame.width > 0 and frame.height > 0 and layoutFont:

        viewport = renderer.viewport[2:]
        scale = renderer.bounds[2:]
        scaleFactor = (viewport[0] / scale[0], viewport[1] / scale[1])

        localToPixel = lambda x, y: (x * scaleFactor[0], y * scaleFactor[1])
        pixelToLocal = lambda x, y: (x / scaleFactor[0], y / scaleFactor[1])

        pixelFrame = core.Rect(frame)
        pixelFrame.origin = localToPixel(*frame.origin)
        pixelFrame.size = localToPixel(*frame.size)

        text = linebreak(pixelFrame, layoutFont, text)

        width = layoutFont.lineWidth(text)
        height = layoutFont.lineHeight()
        baseline = layoutFont.baseline

        if scaleToFit:
            scaleX = pixelFrame.width / width
            scaleY = pixelFrame.height / height
            scale = min(scaleX, scaleY)
            width = width * scale
            height = height * scale
            baseline = baseline * scale

        begin = align(pixelFrame, width, height, baseline)
        if alignToPixel:
            x = floor(begin[0] + 0.5)
            y = floor(begin[1] + 0.5)
            begin = (x, y)

        end = (begin[0] + width, begin[1])

        begin = core.Point(pixelToLocal(*begin))
        end = core.Point(pixelToLocal(*end))


        if outlineFont and outlineColor:
            renderer.renderTextBaseline(begin, end, text, outlineFont, outlineColor, blend)
        if textFont and textColor:
            renderer.renderTextBaseline(begin, end, text, textFont, textColor, blend)
