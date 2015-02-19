import _dk_core as core
from .. import blendstate
from . import view
from . import font

from collections import namedtuple

_MouseTrackInfo = namedtuple('_MouseTrackInfo', 'deviceId, pos')

class TitledView(view.View):

    caption = ''
    captionLeftMargin = 2
    captionRightMargin = 2
    captionTopMargin = 4
    captionBottomMargin = 1
    captionHeight = 28

    contentBorder = 1
    contentMargin = 4
    borderWidth = 1

    movable = True
    pixelBasedMove = True
    clampToParentEdge = True

    fontAttributes = font.attributes(16)

    titleTextColor = core.Color(0, 0, 0)
    titleTextColorHighlighted = core.Color(0, 0, 0)
    titleTextColorActivated = core.Color(1.0, 1.0, 1.0)
    titleTextColorDisabled = core.Color(0.3, 0.3, 0.3)

    titleOutlineColor = None
    titleOutlineColorHighlighted = core.Color(1.0, 1.0, 1.0, 0.75)
    titleOutlineColorActivated = core.Color(0.2, 0.2, 0.2, 0.84)
    titleOutlineColorDisabled = None

    outerBoundColor = core.Color(0.9, 0.9, 0.9)
    outerBoundColorHighlighted = core.Color(0.8, 0.8, 1.0)
    outerBoundColorActivated = core.Color(0.6, 0.6, 1.0)
    outerBoundColorDisabled = core.Color(0.8, 0.8, 0.8)

    minimumViewWidth = borderWidth * 2 + contentBorder * 2 + contentMargin * 2 + 1
    minimumViewHeight = borderWidth * 2 + contentBorder * 2 + contentMargin * 2 + captionHeight + 1

    def __init__(self, caption=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if caption is not None:
            self.caption = str(caption)
        self.__mouseTrackInfo = None
        self.__mouseHover = False

    def orderFront(self):
        parent = self.parent()
        if parent:
            parent.bringChildToFront(self)

    def titleFrame(self):
        border = round(self.contentBorder)
        margin = round(self.contentMargin)
        bounds = super().contentBounds()

        y2 = bounds.y + bounds.height
        y1 = bounds.y + bounds.height - self.captionHeight
        bounds.y = y1
        bounds.height = y2 - y1
        return bounds

    def contentBounds(self):
        border = round(self.contentBorder)
        margin = round(self.contentMargin)
        bounds = super().contentBounds()

        y1 = bounds.y + margin + border
        y2 = bounds.y + bounds.height - self.captionHeight - margin - border
        x1 = bounds.x + margin + border
        x2 = bounds.x + bounds.width - margin - border

        return core.Rect(x1, y1, x2 - x1, y2 - y1)


    def onLoaded(self):
        super().onLoaded()
        b2 = round(self.borderWidth) * 2
        cb2 = round(self.contentBorder) * 2
        m2 = round(self.contentMargin) * 2

        self.minimumViewWidth = b2 + cb2 + m2 + 1
        self.minimumViewHeight = b2 + cb2 + m2 + self.captionHeight + 1

        self.surfaceVisibilityTest = False

    def onUnload(self):
        super().onUnload()

    def __frameClippedByParentBounds(self, frame):
        parent = self.parent()
        if parent:
            try:
                bounds = parent.contentBounds()
            except AttributeError:
                bounds = parent.bounds()

            # make a copy of frame
            frame = core.Rect(frame)
            if frame.width > bounds.width:
                frame.x = bounds.x
            elif frame.x + frame.width > bounds.x + bounds.width:
                frame.x = bounds.x + bounds.width - frame.width
            elif frame.x < bounds.x:
                frame.x = bounds.x
            if frame.height > bounds.height:
                frame.y = bounds.y + bounds.height - frame.height
            elif frame.y + frame.height > bounds.y + bounds.height:
                frame.y = bounds.y + bounds.height - frame.height
            elif frame.y < bounds.y:
                frame.y = bounds.y
        return frame

    def moveToParentInside(self):
        frame = self.__frameClippedByParentBounds(self.frame)
        if frame != self.frame:
            self.frame = frame

    def onUpdate(self, delta, tick, date):
        super().onUpdate(delta, tick, date)
        if self.clampToParentEdge:
            frame = self.__frameClippedByParentBounds(self.frame)
            if frame != self.frame:
                self.frame = frame


    def onRender(self, renderer):
        c = self.backgroundColor
        if self.enabled:
            if self.__mouseTrackInfo:
                self.backgroundColor = self.outerBoundColorActivated
                textColor = self.titleTextColorActivated
                outlineColor = self.titleOutlineColorActivated
            elif self.__mouseHover:
                self.backgroundColor = self.outerBoundColorHighlighted
                textColor = self.titleTextColorHighlighted
                outlineColor = self.titleOutlineColorHighlighted
            else:
                self.backgroundColor = self.outerBoundColor
                textColor = self.titleTextColor
                outlineColor = self.titleOutlineColor
        else:
            self.backgroundColor = self.outerBoundColorDisabled
            textColor = self.titleTextColorDisabled
            outlineColor = self.titleOutlineColorDisabled

        super().onRender(renderer)
        self.backgroundColor = c

        _tm = renderer.transform
        renderer.transform = core.Matrix3()

        titleRect = self.titleFrame()
        titleRect.x += self.captionLeftMargin
        titleRect.width -= self.captionLeftMargin + self.captionRightMargin
        titleRect.y += self.captionBottomMargin
        titleRect.height -= self.captionTopMargin + self.captionBottomMargin
        font.drawText(renderer, titleRect, self.caption, self.font,
                      textColor, outlineColor,
                      align=font.ALIGN_BOTTOM)

        bounds = TitledView.contentBounds(self)
        border = round(self.contentBorder)
        if border > 0:
            rc = core.Rect(bounds)
            rc.x -= border
            rc.width += border * 2
            rc.y -= border
            rc.height += border * 2
            with renderer.contextForSolidRects(self.borderColor, blend=blendstate.defaultOpaque) as r:
                r.add(rc)
        with renderer.contextForSolidRects(self.backgroundColor, blend=blendstate.defaultOpaque) as r:
            r.add(bounds)

        pixelBounds = self.convertLocalToPixel(self.unprojectLocalRect(bounds))
        renderer.viewport = pixelBounds.tuple
        renderer.bounds = bounds.tuple
        renderer.transform = _tm


    def onMouseDown(self, deviceId, buttonId, pos):
        super().onMouseDown(deviceId, buttonId, pos)
        if buttonId == 0 and not self.__mouseTrackInfo:
            self.screen().postOperation(self.orderFront, ())
            bounds = self.bounds()
            contentBounds = TitledView.contentBounds(self)
            pos = self.projectLocalPoint(pos)
            if bounds.isInside(pos) and not contentBounds.isInside(pos):
                self.__mouseTrackInfo = _MouseTrackInfo(deviceId, pos)
                self.captureMouse(deviceId)
                self.redraw()

    def onMouseUp(self, deviceId, buttonId, pos):
        super().onMouseUp(deviceId, buttonId, pos)
        if buttonId == 0 and self.__mouseTrackInfo and self.__mouseTrackInfo.deviceId == deviceId:
            self.__mouseTrackInfo = None
            self.releaseMouse(deviceId)
            self.redraw()
            if deviceId == 0 and not self.__mouseHover:
                bounds = self.bounds()
                contentBounds = self.contentBounds()
                pos = self.projectLocalPoint(pos)
                if bounds.isInside(pos) and not contentBounds.isInside(pos):
                    self.__mouseHover = True

    def onMouseMove(self, deviceId, pos, delta):
        super().onMouseMove(deviceId, pos, delta)
        if self.__mouseTrackInfo and self.__mouseTrackInfo.deviceId == deviceId:
            if self.isMouseCapturedBySelf(deviceId):
                if self.movable:
                    parent = self.parent()
                    if parent:
                        bounds = parent.contentDisplayBounds()
                        v1 = self.unprojectLocalPoint(self.__mouseTrackInfo.pos).vector()
                        v2 = pos.vector()
                        tm = self.localToParentTransformMatrix()
                        v1.transform(tm)
                        v2.transform(tm)

                        x1, x2 = bounds.x, bounds.x + bounds.width
                        y1, y2 = bounds.y, bounds.y + bounds.height

                        v2.x = x1 if v2.x < x1 else x2 if v2.x > x2 else v2.x
                        v2.y = y1 if v2.y < y1 else y2 if v2.y > y2 else v2.y

                        offset = v2 - v1
                        frame = self.frame
                        frame.origin = frame.x + offset.x, frame.y + offset.y

                        if self.clampToParentEdge:
                            frame = self.__frameClippedByParentBounds(frame)

                        if self.pixelBasedMove:
                            p = parent.convertLocalToPixel(core.Point(frame.origin))
                            p.tuple = round(p.x), round(p.y)
                            frame.origin = parent.convertPixelToLocal(p)

                        if frame != self.frame:
                            self.screen().postOperation(self.setFrame, (frame,))

            else:       # we lost mouse
                self.__mouseTrackInfo = None
                self.redraw()

        elif deviceId == 0:     # process hover
            if not self.__mouseHover:
                self.__mouseHover = True
                if not self.__mouseTrackInfo:
                    self.redraw()

    def onMouseLeave(self, deviceId):
        super().onMouseLeave(deviceId)
        if deviceId == 0 and self.__mouseHover:
            self.__mouseHover = False
            if not self.__mouseTrackInfo:
                self.redraw()

    def frameForContentFrame(self, frame):
        frameBorder = round(self.borderWidth)
        contentBorder = round(self.contentBorder)
        margin = round(self.contentMargin)

        x1 = frame.x - (margin + contentBorder + frameBorder)
        x2 = frame.x + frame.width + (margin + contentBorder + frameBorder)
        y1 = frame.y - (margin + contentBorder + frameBorder)
        y2 = frame.y + frame.height + (margin + contentBorder + frameBorder + self.captionHeight)

        return core.Rect(x1, y1, x2-x1, y2-y1)
