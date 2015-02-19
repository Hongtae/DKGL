import _dk_core as core
import math
from .. import blendstate
from . import view
from . import font
from . import control


class RadioButton(control.Control, view.View):

    radius = 8
    circleBorder = 1
    innerCircleRadius = 5

    leftMargin = 4
    rightMargin = 0
    padding = 8

    circleBorderColor = core.Color(0.4, 0.4, 0.4)

    circleColor = core.Color(0.9, 0.9, 1.0)
    circleColorHighlighted = core.Color(0.8, 0.8, 1.0)
    circleColorActivated = core.Color(0.6, 0.6, 1.0)
    circleColorDisabled = core.Color(0.6, 0.6, 0.6)

    innerCircleColor = core.Color(0.25, 0.25, 0.25)
    innerCircleColorHighlighted = core.Color(0.25, 0.25, 1.0)
    innerCircleColorActivated = core.Color(0.0, 0.0, 0.6)
    innerCircleColorDisabled = core.Color(0.2, 0.2, 0.2)

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

    minimumViewWidth = (radius + circleBorder) * 2
    minimumViewHeight = (radius + circleBorder) * 2

    def __init__(self, text, group, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.text = str(text)
        self.group = group
        self.__selected = False
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

        state = self.STATE_DISABLED if not self.enabled else \
                self.STATE_ACTIVATED if self.__activated else \
                self.STATE_HIGHLIGHTED if self.__mouseHover else \
                self.STATE_NORMAL

        bounds = self.contentBounds()

        # draw circle
        circleRect = core.Rect(bounds.x + self.circleBorder + self.leftMargin,
                               bounds.y + bounds.height * 0.5 - self.radius,
                               self.radius * 2, self.radius * 2)
        circleRect.x = math.floor(circleRect.x * self.scaleFactor) / self.scaleFactor
        circleRect.y = math.floor(circleRect.y * self.scaleFactor) / self.scaleFactor

        if self.circleBorder > 0:
            rc = core.Rect(circleRect)
            rc.origin = rc.x - self.circleBorder, rc.y - self.circleBorder
            rc.size = rc.width + self.circleBorder*2, rc.height + self.circleBorder*2
            with renderer.contextForSolidEllipses(self.circleBorderColor) as r:
                r.add(rc)

        circleColor = (self.circleColor,
                       self.circleColorHighlighted,
                       self.circleColorActivated,
                       self.circleColorDisabled)[state]

        with renderer.contextForSolidEllipses(circleColor) as r:
            r.add(circleRect)

        if self.__selected:
            r = self.radius - self.innerCircleRadius
            innerCircleRect = core.Rect(circleRect.x + r, circleRect.y + r,
                                        self.innerCircleRadius * 2, self.innerCircleRadius * 2)

            innerCircleColor = (self.innerCircleColor,
                                self.innerCircleColorHighlighted,
                                self.innerCircleColorActivated,
                                self.innerCircleColorDisabled)[state]

            with renderer.contextForSolidEllipses(innerCircleColor, blend=blendstate.defaultOpaque) as r:
                r.add(innerCircleRect)

        # draw text
        textRect = core.Rect(circleRect.x + circleRect.width + self.circleBorder + self.padding,
                             bounds.y,
                             0,
                             bounds.height)

        lineWidth = self.font.lineWidth(self.text)
        lineHeight = self.font.lineHeight()
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

        font.drawText(renderer, textRect, self.text, self.font,
                      textColor, outlineColor,
                      align=font.ALIGN_LEFT,
                      linebreak=font.LINE_BREAK_TRUNCATING_TAIL,
                      blend=blendstate.defaultAlpha)

        if self.interactOnlyInsideVisibleContentRect:
            x1 = circleRect.x - self.circleBorder - self.padding
            x2 = textRect.x + textRect.width + self.padding

            textOriginY = bounds.y + (bounds.height - lineHeight) * 0.5
            y1 = min(circleRect.y - self.circleBorder, textOriginY) - self.padding
            y2 = max(circleRect.y + circleRect.height, textOriginY + lineHeight) + self.padding

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
                self.setSelected()

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

    def siblings(self):
        parent = self.parent()
        s = []
        if parent and self.group is not None:
            for c in parent.children():
                if c is not self and isinstance(c, RadioButton):
                    if c.group == self.group:
                        s.append(c)
        return s

    @property
    def selected(self):
        return self.__selected

    def setSelected(self):
        if not self.__selected:
            self.__selected = True
            parent = self.parent()
            if parent and self.group is not None:
                for c in parent.children():
                    if c is not self and isinstance(c, RadioButton):
                        if c.group == self.group:
                            if c.__selected:
                                c.__selected = False
                                c.redraw()
            self.redraw()
            # post event
            screen = self.screen()
            if screen:
                screen.postOperation(self.postEvent, ())

    def postEvent(self):
        super().invokeAllTargets(self)


def addItems(view, items, rect, columns=1, selectedItemIndex=0, group=None):

    if group is None:
        group = object()

    buttons = []

    numItems = len(items)
    columns = max(columns, 1)
    rows = math.ceil(numItems / columns)
    width = rect.width / columns
    height = rect.height / rows

    count = 0
    for v in items:
        col = count % columns
        row = int(count / columns)
        rc = core.Rect(rect.x + width * col, rect.y + height * row, width, height)
        item = RadioButton(v, group, frame=rc)
        buttons.append(item)
        count += 1

    try:
        button = buttons[selectedItemIndex]
        button.setSelected()
    except IndexError:
        pass

    if view is not None:
        for item in buttons:
            view.addChild(item)

    return buttons
