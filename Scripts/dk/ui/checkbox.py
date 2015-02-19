import _dk_core as core
from .. import blendstate
from . import view
from . import font
from . import control


class Checkbox(control.Control, view.View):

    boxSize = 14
    boxBorder = 1
    boxBorderColor = core.Color(0, 0, 0)

    leftMargin = 4
    rightMargin = 0
    padding = 8

    boxColor = core.Color(1.0, 1.0, 1.0)
    boxColorHighlighted = core.Color(0.8, 0.8, 1.0)
    boxColorActivated = core.Color(0.3, 0.3, 1.0)
    boxColorDisabled = core.Color(0.6, 0.6, 0.6)

    checkColor = core.Color(0.0, 0.0, 0.0)
    checkColorHighlighted = core.Color(0.0, 0.0, 1.0)
    checkColorActivated = core.Color(0.0, 1.0, 1.0)
    checkColorDisabled = core.Color(0.2, 0.2, 0.2)

    textColor = core.Color(0.0, 0.0, 0.0)
    textColorHighlighted = core.Color(0.0, 0.0, 0.0)
    textColorActivated = core.Color(0.0, 0.0, 0.7)
    textColorDisabled = core.Color(0.3, 0.3, 0.3)

    outlineColor = None
    outlineColorHighlighted = None
    outlineColorActivated = None
    outlineColorDisabled = None

    backgroundColor = core.Color(1, 1, 1)

    fontAttributes = font.attributes(14)

    interactOnlyInsideVisibleContentRect = True

    minimumViewHeight = boxSize + boxBorder * 2
    minimumViewWidth = boxSize + leftMargin + boxBorder * 2

    def __init__(self, text='', value=False, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.text = str(text)
        self.value = value
        self.__mouseHover = False
        self.__activated = False
        self.__capturedMouseId = None
        self.__interactFrame = None

    def onLoaded(self):
        super().onLoaded()

    def onUnload(self):
        super().onUnload()

    def onRender(self, renderer):
        super().onRender(renderer)

        invScale = 1.0 / self.scaleFactor

        state = self.STATE_DISABLED if not self.enabled else \
                self.STATE_ACTIVATED if self.__activated else \
                self.STATE_HIGHLIGHTED if self.__mouseHover else \
                self.STATE_NORMAL

        bounds = self.contentBounds()

        # draw box
        boxRect = core.Rect(bounds.x + self.leftMargin,
                            bounds.y + (bounds.height - self.boxSize) * 0.5,
                            self.boxSize,
                            self.boxSize)

        if self.boxBorder > 0:
            border2 = self.boxBorder * 2
            boxRect.x += self.boxBorder
            rc = core.Rect(boxRect.x - self.boxBorder, boxRect.y - self.boxBorder,
                           boxRect.width + border2, boxRect.height + border2)
            with renderer.contextForSolidRects(self.boxBorderColor, blend=blendstate.defaultOpaque) as r:
                r.add(rc)

        boxColor = (self.boxColor,
                    self.boxColorHighlighted,
                    self.boxColorActivated,
                    self.boxColorDisabled)[state]

        with renderer.contextForSolidRects(boxColor, blend=blendstate.defaultOpaque) as r:
            r.add(boxRect)

        # draw check-mark
        if self.value:
            p0 = core.Point(0.15, 0.6)
            p1 = core.Point(0.05, 0.5)
            p2 = core.Point(0.4, 0.35)
            p3 = core.Point(0.4, 0.15)
            p4 = core.Point(0.85, 0.8)
            p5 = core.Point(0.95, 0.7)
            for p in (p0, p1, p2, p3, p4, p5):
                p.x = p.x * boxRect.width + boxRect.x
                p.y = p.y * boxRect.height + boxRect.y

            checkColor = (self.checkColor,
                          self.checkColorHighlighted,
                          self.checkColorActivated,
                          self.checkColorDisabled)[state]

            with renderer.contextForSolidTriangleStrip(checkColor, blend=blendstate.defaultOpaque) as r:
                r.addTriangle(p0, p1, p2)
                r.add(p3)
                r.add(p4)
                r.add(p5)

        # draw text
        textRect = core.Rect(boxRect.x + boxRect.width + self.boxBorder + self.padding,
                             bounds.y,
                             0,
                             bounds.height)
        lineWidth = self.font.lineWidth(self.text) * invScale
        lineHeight = self.font.lineHeight() * invScale
        textRect.width = min(bounds.x + bounds.width - textRect.x - self.rightMargin, lineWidth)

        # with renderer.contextForSolidRects(core.Color(1,0,0), blend=blendstate.defaultOpaque) as r:
        #     r.add(textRect)

        textColor = (self.textColor,
                     self.textColorHighlighted,
                     self.textColorActivated,
                     self.textColorDisabled)[state]
        outlineColor = (self.outlineColor,
                        self.outlineColorHighlighted,
                        self.outlineColorActivated,
                        self.outlineColorDisabled)[state]

        font.drawText(renderer, textRect, self.text, self.font, textColor, outlineColor,
                      align=font.ALIGN_LEFT,
                      linebreak=font.LINE_BREAK_TRUNCATING_TAIL,
                      blend=blendstate.defaultAlpha)

        if self.interactOnlyInsideVisibleContentRect:
            x1 = boxRect.x - self.boxBorder - self.padding
            x2 = textRect.x + textRect.width + self.padding

            textOriginY = bounds.y + (bounds.height - lineHeight) * 0.5
            y1 = min(boxRect.y - self.boxBorder, textOriginY) - self.padding
            y2 = max(boxRect.y + boxRect.height, textOriginY + lineHeight) + self.padding

            self.__interactFrame = core.Rect(x1, y1, x2 - x1, y2 - y1)
        else:
            self.__interactFrame = bounds


    def onMouseDown(self, deviceId, buttonId, pos):
        super().onMouseDown(deviceId, buttonId, pos)
        if self.__capturedMouseId:
            if not self.isMouseCapturedBySelf(self.__capturedMouseId[0]):
                self.__capturedMouseId = None

        if self.__capturedMouseId is None:
            if self.__interactFrame and self.__interactFrame.isInside(pos):
                self.captureMouse(deviceId)
                self.__capturedMouseId = (deviceId, buttonId)
                self.__activated = True
                self.redraw()

    def onMouseUp(self, deviceId, buttonId, pos):
        super().onMouseUp(deviceId, buttonId, pos)
        if self.__capturedMouseId and self.__capturedMouseId == (deviceId, buttonId):
            self.releaseMouse(deviceId)
            if self.__interactFrame and self.__interactFrame.isInside(pos):
                self.value = not self.value
                self.screen().postOperation(self.postEvent, ())

            self.__capturedMouseId = None
            self.__activated = False
            self.redraw()


    def onMouseMove(self, deviceId, pos, delta):
        super().onMouseMove(deviceId, pos, delta)
        if self.__capturedMouseId and self.__capturedMouseId[0] == deviceId:
            act = self.__activated
            if self.isMouseCapturedBySelf(deviceId):
                if self.__interactFrame:
                    self.__activated = self.__interactFrame.isInside(pos)
                else:
                    self.__activated = False
            else:
                self.__capturedMouseId = None
                self.__activated = False
                
            if act != self.__activated:
                self.redraw()

        elif deviceId == 0:
            h = self.__mouseHover
            self.__mouseHover = self.__interactFrame.isInside(pos) if self.__interactFrame else False
            if self.__mouseHover != h:
                self.redraw()

    def onMouseLeave(self, deviceId):
        super().onMouseLeave(deviceId)
        if deviceId == 0 and self.__mouseHover:
            self.__mouseHover = False
            self.redraw()

    def postEvent(self):
        super().invokeAllTargets(self)